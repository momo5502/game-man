#import "NativeAddPlugin.h"
#if __has_include(<native_add/native_add-Swift.h>)
#import <native_add/native_add-Swift.h>
#else
// Support project import fallback if the generated compatibility header
// is not copied when this plugin is created as a library.
// https://forums.swift.org/t/swift-static-libraries-dont-copy-generated-objective-c-header/19816
#import "native_add-Swift.h"
#endif

@implementation NativeAddPlugin
+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
  [SwiftNativeAddPlugin registerWithRegistrar:registrar];
}
@end
