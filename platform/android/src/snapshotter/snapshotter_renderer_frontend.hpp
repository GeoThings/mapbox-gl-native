
#pragma once

#include <mbgl/renderer/renderer_backend.hpp>
#include <mbgl/renderer/renderer_frontend.hpp>
#include <mbgl/util/async_task.hpp>

#include <memory>
#include <vector>

namespace mbgl {

class Renderer;
class RendererBackend;
class RendererBridge;
class RendererObserver;
class FileSource;
class Scheduler;
class UpdateParameters;

namespace util {
template<class> class Thread;
} // namspace util

namespace android {

class SnapshotterRendererFrontend : public RendererFrontend {
public:
    SnapshotterRendererFrontend(float pixelRatio,
                                Size,
                                mbgl::FileSource&,
                                Scheduler&,
                                std::string programCacheDir);
    ~SnapshotterRendererFrontend() override;

    void reset() override;
    void setObserver(RendererObserver&) override;

    void update(std::shared_ptr<UpdateParameters>) override;

    PremultipliedImage getImage();

    // Memory
    void onLowMemory();

private:
    void render();

    std::unique_ptr<util::Thread<RendererBridge>> renderer;
    std::shared_ptr<UpdateParameters> updateParameters;
    util::AsyncTask asyncInvalidate;
};

} // namespace android
} // namespace mbgl
