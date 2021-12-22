import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:game_man/download_manager.dart';

enum DownloadWidgetStatus {
  uninitialized,
  waiting,
  loaded,
  error,
}

typedef BuildCallback = Widget Function();
typedef BuildDataCallback = Widget Function(Uint8List, bool);

class DownloadWidget extends StatefulWidget {
  const DownloadWidget({
    Key? key,
    required this.url,
    required this.buildWaiting,
    required this.buildError,
    required this.buildLoaded,
  }) : super(key: key);

  final String url;
  final BuildCallback buildWaiting;
  final BuildCallback buildError;
  final BuildDataCallback buildLoaded;

  @override
  State<DownloadWidget> createState() => _DownloadWidgetState();
}

class _DownloadWidgetState extends State<DownloadWidget> {
  DownloadWidgetStatus _status = DownloadWidgetStatus.uninitialized;
  Uint8List? _data;

  void _markDone(Uint8List data) {
    if (!mounted) {
      return;
    }

    setState(() {
      _status = DownloadWidgetStatus.loaded;
      _data = data;
    });
  }

  void _startDownload() {
    final result = gDownloadManager.download(widget.url);
    if (result.data != null) {
      _markDone(result.data!);
      return;
    }

    result.future //
        .then(_markDone) //
        .catchError((e) {
      if (!mounted) {
        return;
      }

      setState(() {
        _status = DownloadWidgetStatus.error;
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    bool wasUninitialized = _status == DownloadWidgetStatus.uninitialized;
    if (wasUninitialized) {
      _status = DownloadWidgetStatus.waiting;
      _startDownload();
    }

    if (_status == DownloadWidgetStatus.loaded) {
      return widget.buildLoaded(_data!, wasUninitialized);
    }

    if (_status == DownloadWidgetStatus.error) {
      return widget.buildError();
    }

    return widget.buildWaiting();
  }
}
