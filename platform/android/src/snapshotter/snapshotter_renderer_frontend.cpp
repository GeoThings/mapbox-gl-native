#include "snapshotter_renderer_frontend.hpp"

#include <mbgl/actor/actor_ref.hpp>
#include <mbgl/actor/mailbox.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/gl/headless_backend.hpp>
#include <mbgl/renderer/renderer.hpp>
#include <mbgl/renderer/renderer_observer.hpp>
#include <mbgl/storage/file_source.hpp>
#include <mbgl/util/thread.hpp>

#include <cassert>
#include <mbgl/util/event.hpp>

namespace {

using namespace mbgl;

// Forwards RendererObserver signals to the given
// Delegate RendererObserver on the given RunLoop
class RendererObserverBridge : public RendererObserver {
public:
    RendererObserverBridge(RendererObserver& delegate_)
            : mailbox(std::make_shared<Mailbox>(*Scheduler::GetCurrent()))
            , delegate(ActorRef<RendererObserver>(delegate_, mailbox)) {
    }

    void onInvalidate() override {
        delegate.invoke(&RendererObserver::onInvalidate);
    }

    void onResourceError(std::exception_ptr err) override {
        delegate.invoke(&RendererObserver::onResourceError, err);
    }

    void onWillStartRenderingMap() override {
        delegate.invoke(&RendererObserver::onWillStartRenderingMap);
    }

    void onWillStartRenderingFrame() override {
        delegate.invoke(&RendererObserver::onWillStartRenderingFrame);
    }

    void onDidFinishRenderingFrame(RenderMode mode, bool repaintNeeded) override {
        delegate.invoke(&RendererObserver::onDidFinishRenderingFrame, mode, repaintNeeded);
    }

    void onDidFinishRenderingMap() override {
        delegate.invoke(&RendererObserver::onDidFinishRenderingMap);
    }

private:
    std::shared_ptr<Mailbox> mailbox;
    ActorRef<RendererObserver> delegate;
};

} // namespace

namespace mbgl {


// A bridge that takes care of data ownership and Backend handling
// when called asynchronously
class RendererBridge {
public:
    RendererBridge(float pixelRatio,
                   Size size,
                   mbgl::FileSource& fileSource,
                   Scheduler& scheduler,
                   std::string programCacheDir)
            : backend(std::make_unique<HeadlessBackend>(size))
            , renderer(std::make_unique<Renderer>(*backend,
                                                  pixelRatio,
                                                  fileSource,
                                                  scheduler,
                                                  GLContextMode::Unique,
                                                  programCacheDir)) {

    }

    void render(std::shared_ptr<UpdateParameters> updateParameters) {
        BackendScope guard { *backend };
        renderer->render(*updateParameters);
    }

    PremultipliedImage getImage() {
        BackendScope guard { *backend };
        return backend->readStillImage();
    }

    void setObserver(std::unique_ptr<RendererObserver> observer) {
        assert(!rendererObserver);
        rendererObserver = std::move(observer);
        renderer->setObserver(rendererObserver.get());
    }

    void onLowMemory() {
        renderer->onLowMemory();
    }
private:
    std::unique_ptr<HeadlessBackend> backend;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<RendererObserver> rendererObserver;
};

namespace android {

SnapshotterRendererFrontend::SnapshotterRendererFrontend(float pixelRatio,
                                                         Size size,
                                                         mbgl::FileSource& fileSource,
                                                         Scheduler& scheduler,
                                                         std::string programCacheDir)
        : renderer(std::make_unique<util::Thread<RendererBridge>>(
                "Map Snapshotter Thread",
                pixelRatio,
                size,
                fileSource,
                scheduler,
                programCacheDir
        ))
        , asyncInvalidate([this]() {
            render();
        }) {
}

SnapshotterRendererFrontend::~SnapshotterRendererFrontend() = default;

void SnapshotterRendererFrontend::reset() {
    assert (renderer);
    renderer.reset();
}

void SnapshotterRendererFrontend::setObserver(RendererObserver& observer) {
    assert (renderer);
    renderer->actor().invoke(&RendererBridge::setObserver, std::make_unique<RendererObserverBridge>(observer));
}

void SnapshotterRendererFrontend::update(std::shared_ptr<UpdateParameters> params) {
    updateParameters = std::move(params);
    asyncInvalidate.send();
}

void SnapshotterRendererFrontend::render() {
    assert (renderer);
    if (!updateParameters) return;

    renderer->actor().invoke(&RendererBridge::render, std::move(updateParameters));
}

void SnapshotterRendererFrontend::onLowMemory() {
    assert (renderer);
    renderer->actor().invoke(&RendererBridge::onLowMemory);
}

PremultipliedImage SnapshotterRendererFrontend::getImage() {
    assert(renderer);

    // Block the renderer thread and get the still image
    util::BlockingThreadGuard<RendererBridge> guard { *renderer };
    return guard.object().getImage();
}

} // namespace android
} // namespace mbgl

