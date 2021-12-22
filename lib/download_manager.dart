import 'dart:async';
import 'dart:collection';
import 'dart:typed_data';
import 'package:http/http.dart' as http;

class DownloadObject {
  bool started = false;
  Uint8List? data;
  Completer<Uint8List> completer = Completer<Uint8List>();
  DateTime insertionTime = DateTime.now();
}

class DownloadResult {
  DownloadResult(this.future);

  Uint8List? data;
  Future<Uint8List> future;
}

class DownloadManager {
  final _parallelLimit = 5;
  final _objectLimit = 100;
  final _objects = HashMap<Uri, DownloadObject>();

  bool _removeElement() {
    Uri? keyToRemove;
    DateTime? insertionTime;

    for (final entry in _objects.entries) {
      if (!entry.value.completer.isCompleted) {
        continue;
      }

      if (insertionTime != null &&
          insertionTime.isBefore(entry.value.insertionTime)) {
        continue;
      }

      insertionTime = entry.value.insertionTime;
      keyToRemove = entry.key;
    }

    if (keyToRemove == null) {
      return false;
    }

    _objects.remove(keyToRemove);
    return true;
  }

  void _reduceMap() {
    while (_objects.length > _objectLimit) {
      if (!_removeElement()) {
        break;
      }
    }
  }

  int _countRunningDownloads() {
    int result = 0;

    for (final entry in _objects.entries) {
      if (entry.value.started && !entry.value.completer.isCompleted) {
        ++result;
      }
    }

    return result;
  }

  void _startDownload(Uri uri, DownloadObject object) {
    http.get(uri).then((response) {
      if (response.statusCode != 200) {
        object.completer.completeError(response.statusCode);
      } else {
        object.data = response.bodyBytes;
        object.completer.complete(response.bodyBytes);
      }
      _runTasks();
    }).catchError((e) {
      object.completer.completeError(e);
      _runTasks();
    });
  }

  bool _triggerDownload() {
    Uri? keyToDownload;
    DateTime? insertionTime;

    for (final entry in _objects.entries) {
      if (entry.value.started) {
        continue;
      }

      if (insertionTime != null &&
          insertionTime.isBefore(entry.value.insertionTime)) {
        continue;
      }

      keyToDownload = entry.key;
      insertionTime = entry.value.insertionTime;
    }

    if (keyToDownload == null) {
      return false;
    }

    final object = _objects[keyToDownload];
    object!.started = true;

    _startDownload(keyToDownload, object);
    return true;
  }

  void _triggerDownloads() {
    while (_countRunningDownloads() < _parallelLimit) {
      if (!_triggerDownload()) {
        break;
      }
    }
  }

  DownloadObject _getDownloadObject(Uri url) {
    var object = _objects[url];
    if (object != null) {
      return object;
    }

    object = DownloadObject();
    _objects[url] = object;
    return object;
  }

  void _runTasks() {
    _reduceMap();
    _triggerDownloads();
  }

  DownloadResult _buildResult(DownloadObject object) {
    final result = DownloadResult(object.completer.future);
    if (object.completer.isCompleted && object.data != null) {
      result.data = object.data;
    }

    return result;
  }

  DownloadResult download(String url) {
    final uri = Uri.parse(url);
    return downloadUri(uri);
  }

  DownloadResult downloadUri(Uri uri) {
    final object = _getDownloadObject(uri);
    _runTasks();
    return _buildResult(object);
  }
}

final gDownloadManager = DownloadManager();
