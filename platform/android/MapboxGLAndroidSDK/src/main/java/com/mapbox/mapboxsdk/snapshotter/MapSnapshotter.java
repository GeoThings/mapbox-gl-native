package com.mapbox.mapboxsdk.snapshotter;

import android.content.Context;
import android.graphics.Bitmap;

import com.mapbox.mapboxsdk.camera.CameraPosition;
import com.mapbox.mapboxsdk.constants.Style;
import com.mapbox.mapboxsdk.geometry.LatLngBounds;
import com.mapbox.mapboxsdk.maps.MapboxMap;
import com.mapbox.mapboxsdk.storage.FileSource;

/**
 * The map snapshotter creates a bitmap of the map, rendered
 * off the main thread.
 */
public class MapSnapshotter {

  // Holds the pointer to JNI NativeMapView
  private long nativePtr = 0;
  private MapboxMap.SnapshotReadyCallback callback;

  public static class Options {
    private int pixelRatio = 1;
    private int width;
    private int height;
    private String styleUrl = Style.MAPBOX_STREETS;
    private LatLngBounds region;
    private CameraPosition cameraPosition;

    public Options(int width, int height) {
      this.width = width;
      this.height = height;
    }

    public Options withStyle(String url) {
      this.styleUrl = url;
      return this;
    }

    public Options withRegion(LatLngBounds region) {
      this.region = region;
      return this;
    }

    public Options withPixelRatio(int pixelRatio) {
      this.pixelRatio = pixelRatio;
      return this;
    }

    public Options withCameraPosition(CameraPosition cameraPosition) {
      this.cameraPosition = cameraPosition;
      return this;
    }

    public int getWidth() {
      return width;
    }

    public int getHeight() {
      return height;
    }

    public int getPixelRatio() {
      return pixelRatio;
    }

    public LatLngBounds getRegion() {
      return region;
    }

    public String getStyleUrl() {
      return styleUrl;
    }

    public CameraPosition getCameraPosition() {
      return cameraPosition;
    }
  }

  public MapSnapshotter(Context context, Options options) {
    FileSource fileSource = FileSource.getInstance(context);
    String programCacheDir = context.getCacheDir().getAbsolutePath();

    nativeInitialize(this, fileSource, options.pixelRatio, options.width,
      options.height, options.styleUrl, options.region, options.cameraPosition,
      programCacheDir);
  }

  public void start(MapboxMap.SnapshotReadyCallback callback) {
    if (this.callback != null) {
      throw new IllegalStateException("Snapshotter was already started");
    }

    this.callback = callback;
    nativeStart();
  }

  /**
   * Must be called in onPause or when discarding the object
   */
  public void cancel() {
    callback = null;
    nativeDestroy();
  }

  /**
   * Called by JNI peer when snapshot is ready.
   * Always called on the origin (main) thread.
   *
   * @param bitmap the generated snapshot
   */
  protected void onSnapshotReady(Bitmap bitmap) {
    if (callback != null) {
      callback.onSnapshotReady(bitmap);
    }
  }

  protected native void nativeInitialize(MapSnapshotter mapSnapshotter,
                                         FileSource fileSource, float pixelRatio,
                                         int width, int height, String styleUrl,
                                         LatLngBounds region, CameraPosition position,
                                         String programCacheDir);

  protected native void nativeStart();

  protected native void nativeDestroy();

}
