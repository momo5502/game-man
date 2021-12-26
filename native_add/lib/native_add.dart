import 'dart:ffi'; // For FFI
import 'dart:io';

import 'dart:typed_data';

import 'package:ffi/ffi.dart'; // For Platform.isX

DynamicLibrary? nativeAddLib;
void Function(int, bool)? pressButton;
Pointer<Uint32> Function()? getBuffer;
void Function(Pointer<Uint8>, int)? loadRom;

DynamicLibrary getDynamicLib() {
  nativeAddLib ??= Platform.isAndroid
      ? DynamicLibrary.open('libnative_add.so')
      : DynamicLibrary.process();

  return nativeAddLib!;
}

void Function(int, bool) getPressButtonFunc() {
  pressButton ??= getDynamicLib()
      .lookup<NativeFunction<Void Function(Int32, Bool)>>('press_button')
      .asFunction();

  return pressButton!;
}

Pointer<Uint32> Function() getGetbufferFunc() {
  getBuffer ??= getDynamicLib()
      .lookup<NativeFunction<Pointer<Uint32> Function()>>('get_buffer')
      .asFunction();

  return getBuffer!;
}

void Function(Pointer<Uint8>, int) getLoadRomFunc() {
  loadRom ??= getDynamicLib()
      .lookup<NativeFunction<Void Function(Pointer<Uint8>, Uint64)>>('load_rom')
      .asFunction();

  return loadRom!;
}

void pressGbButton(int id, bool value) {
  getPressButtonFunc()(id, value);
}

void loadGbRom(Uint8List list) {
  final pointer = calloc.allocate<Uint8>(list.length);
  pointer.asTypedList(list.length).setAll(0, list);
  getLoadRomFunc()(pointer, list.length);

  calloc.free(pointer);
}
