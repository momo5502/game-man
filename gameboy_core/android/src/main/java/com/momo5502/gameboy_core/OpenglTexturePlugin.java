package com.momo5502.gameboy_core;

import android.graphics.SurfaceTexture;
import android.util.Log;
import android.util.LongSparseArray;
import android.view.Surface;

import java.util.Map;

import io.flutter.embedding.engine.plugins.FlutterPlugin;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.MethodChannel.MethodCallHandler;
import io.flutter.plugin.common.MethodChannel.Result;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.PluginRegistry.Registrar;
import io.flutter.view.TextureRegistry;

public class OpenglTexturePlugin implements FlutterPlugin, MethodCallHandler {
    static {
        System.loadLibrary("gameboy_core");
    }

    private MethodChannel channel;
    private TextureRegistry textures;

    void setTextures(TextureRegistry textures)
    {
        this.textures = textures;
    }

    private static void setup(OpenglTexturePlugin plugin, BinaryMessenger binaryMessenger, TextureRegistry textures) {
        plugin.setTextures(textures);

        plugin.channel = new MethodChannel(binaryMessenger, "opengl_texture");
        plugin.channel.setMethodCallHandler(plugin);
    }

    @Override
    public void onAttachedToEngine(FlutterPluginBinding flutterPluginBinding) {
        setup(this, flutterPluginBinding.getBinaryMessenger(), flutterPluginBinding.getTextureRegistry());
    }

    @Override
    public void onDetachedFromEngine(FlutterPluginBinding flutterPluginBinding)
    {
        // TODO
    }

    public static void registerWith(Registrar registrar) {
        setup(new OpenglTexturePlugin(), registrar.messenger(), registrar.textures());
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
