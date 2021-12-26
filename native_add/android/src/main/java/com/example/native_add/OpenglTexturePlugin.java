package com.example.native_add;

import android.graphics.SurfaceTexture;
import android.util.Log;
import android.util.LongSparseArray;
import android.view.Surface;

import java.util.Map;

import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.MethodChannel.MethodCallHandler;
import io.flutter.plugin.common.MethodChannel.Result;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.PluginRegistry.Registrar;
import io.flutter.view.TextureRegistry;

public class OpenglTexturePlugin implements MethodCallHandler {
    static {
        System.loadLibrary("native_add");
    }

    private final TextureRegistry textures;

    public OpenglTexturePlugin(TextureRegistry textures) {
        this.textures = textures;
    }

    public static void registerWith(Registrar registrar) {
        Log.i("OpenglTexturePlugin", "INITTT");
        final MethodChannel channel = new MethodChannel(registrar.messenger(), "opengl_texture");
        channel.setMethodCallHandler(new OpenglTexturePlugin(registrar.textures()));
    }

    @Override
    public void onMethodCall(MethodCall call, Result result) {
        Map<String, Number> arguments = (Map<String, Number>) call.arguments;
        Log.d("OpenglTexturePlugin", call.method + " " + call.arguments.toString());
        if (call.method.equals("create")) {
            TextureRegistry.SurfaceTextureEntry entry = textures.createSurfaceTexture();
            SurfaceTexture surfaceTexture = entry.surfaceTexture();

            surfaceTexture.setDefaultBufferSize(160 * 2, 144 * 2);

            nativeSetSurface(new Surface(surfaceTexture));
            result.success(entry.id());
        } else if (call.method.equals("dispose")) {

        } else {
            result.notImplemented();
        }
    }

    public static native void nativeSetSurface(Surface surface);
}
