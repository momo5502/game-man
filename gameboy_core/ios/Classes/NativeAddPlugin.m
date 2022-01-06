#import "NativeAddPlugin.h"
#if __has_include(<gameboy_core/gameboy_core-Swift.h>)
#import <gameboy_core/gameboy_core-Swift.h>
#else
// Support project import fallback if the generated compatibility header
// is not copied when this plugin is created as a library.
// https://forums.swift.org/t/swift-static-libraries-dont-copy-generated-objective-c-header/19816
#import "gameboy_core-Swift.h"
#endif

@implementation NativeAddPlugin
+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
  [SwiftNativeAddPlugin registerWithRegistrar:registrar];
}
@end
