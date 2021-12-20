import 'dart:async';
import 'dart:collection';
import 'dart:typed_data';
import 'package:http/http.dart' as http;

class DownloadObject {
  bool started = false;
  Completer<Uint8List> completer = Completer<Uint8List>();
  DateTime insertionTime = DateTime.now();
}

class DownloadManager {
  final int _parallelLimit = 5;
  final int _objectLimit = 100;
  final HashMap<String, DownloadObject> _objects =
      HashMap<String, DownloadObject>();

  bool _removeElement() {
    String? keyToRemove;
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

  void _startDownload(String url, Completer<Uint8List> completer) {
    final uri = Uri.parse(url);
    http.get(uri).then((response) {
      if (response.statusCode != 200) {
        completer.completeError(response.statusCode);
      } else {
        completer.complete(response.bodyBytes);
      }
      _runTasks();
    }).catchError((e) {
      completer.completeError(e);
      _runTasks();
    });
  }

  bool _triggerDownload() {
    String? keyToDownload;
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

    _startDownload(keyToDownload, object.completer);
    return true;
  }

  void _triggerDownloads() {
    while (_countRunningDownloads() < _parallelLimit) {
      if (!_triggerDownload()) {
        break;
      }
    }
  }

  DownloadObject _getDownloadObject(String url) {
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

  Future<Uint8List> download(String url) {
    final object = _getDownloadObject(url);
    _runTasks();
    return object.completer.future;
  }
}

final gDownloadManager = DownloadManager();
