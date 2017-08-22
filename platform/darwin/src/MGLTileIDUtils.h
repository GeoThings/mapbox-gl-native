#import <Foundation/Foundation.h>

#import "MGLFoundation.h"
#import "MGLGeometry.h"

NS_ASSUME_NONNULL_BEGIN

MGL_EXPORT
@interface MGLTileIDUtils : NSObject

+ (MGLCoordinateBounds)coordinateBoundsForTileID:(MGLTileID)tileID;
+ (MGLTileID)tileIDContainingCoordinate:(CLLocationCoordinate2D)coordinate atZoom:(double)zoomLevel;

@end

NS_ASSUME_NONNULL_END
