#import "MGLTileIDUtils.h"
#import "MGLGeometry_Private.h"

@implementation MGLTileIDUtils

+ (MGLCoordinateBounds)coordinateBoundsForTileID:(MGLTileID)tileID {
    return MGLCoordinateBondsForTileId(tileID);
}

+ (MGLTileID)tileIDContainingCoordinate:(CLLocationCoordinate2D)coordinate atZoom:(double)zoomLevel {
    return MGLTileIDContainingCoordinate2D(coordinate, zoomLevel);
}

@end

