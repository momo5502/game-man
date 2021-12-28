import 'dart:ffi'; // For FFI
import 'dart:io';

import 'dart:typed_data';

import 'package:ffi/ffi.dart'; // For Platform.isX

typedef GetStateCallback = Void Function(Pointer<Uint8>, Uint64);
typedef GetStateCallbackPointer = Pointer<NativeFunction<GetStateCallback>>;

typedef GetStateFFI = NativeFunction<Void Function(GetStateCallbackPointer)>;
typedef GetStateNative = void Function(GetStateCallbackPointer);

DynamicLibrary? nativeAddLib;
void Function(int, bool)? pressButton;
void Function(bool)? setPaused;
GetStateNative? getState;
void Function(Pointer<Uint8>, int, bool)? loadRom;

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

void Function(bool) getSetPausedFunc() {
  setPaused ??= getDynamicLib()
      .lookup<NativeFunction<Void Function(Bool)>>('set_paused')
      .asFunction();

  return setPaused!;
}

GetStateNative getGetStateFunc() {
  getState ??= getDynamicLib().lookup<GetStateFFI>('get_state').asFunction();

  return getState!;
}

void Function(Pointer<Uint8>, int, bool) getLoadRomFunc() {
  loadRom ??= getDynamicLib()
      .lookup<NativeFunction<Void Function(Pointer<Uint8>, Uint64, Bool)>>(
          'load_rom')
      .asFunction();

  return loadRom!;
}

void pressGbButton(int id, bool value) {
  getPressButtonFunc()(id, value);
}

void loadGbRom(Uint8List list, bool isSave) {
  final pointer = calloc.allocate<Uint8>(list.length);
  pointer.asTypedList(list.length).setAll(0, list);
  getLoadRomFunc()(pointer, list.length, isSave);

  calloc.free(pointer);
}

void setGbPaused(bool paused) {
  getSetPausedFunc()(paused);
}

Uint8List? list;
void callback(Pointer<Uint8> data, int size) {
  list = Uint8List.fromList(data.asTypedList(size).toList());
}

Uint8List? getGbState() {
  getGetStateFunc()(Pointer.fromFunction<GetStateCallback>(callback));
  Uint8List? cList = list;
  list = null;
  return cList;
}
