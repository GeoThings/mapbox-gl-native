#pragma once

#include <mbgl/map/map.hpp>
#include <mbgl/map/map_observer.hpp>
#include <mbgl/util/default_thread_pool.hpp>

#include "../file_source.hpp"
#include "../geometry/lat_lng_bounds.hpp"
#include "../map/camera_position.hpp"

#include <jni/jni.hpp>

#include <memory>

namespace mbgl {
namespace android {

class SnapshotterRendererFrontend;

class MapSnapshotter : public MapObserver {
public:

    static constexpr auto Name() { return "com/mapbox/mapboxsdk/snapshotter/MapSnapshotter"; };

    static jni::Class<MapSnapshotter> javaClass;

    static void registerNative(jni::JNIEnv&);

    MapSnapshotter(jni::JNIEnv&,
                   jni::Object<MapSnapshotter>,
                   jni::Object<FileSource>,
                   jni::jfloat pixelRatio,
                   jni::jint width,
                   jni::jint height,
                   jni::String styleURL,
                   jni::Object<LatLngBounds> region,
                   jni::Object<CameraPosition> position,
                   jni::String programCacheDir);

    void start(JNIEnv&);

private:
    JavaVM *vm = nullptr;
    jni::UniqueWeakObject<MapSnapshotter> javaPeer;

    float pixelRatio;

    std::unique_ptr<SnapshotterRendererFrontend> rendererFrontend;
    std::shared_ptr<mbgl::ThreadPool> threadPool;
    std::unique_ptr<mbgl::Map> map;
};

} // namespace android
} // namespace mbgl