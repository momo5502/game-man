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
typedef BuildDataCallback = Widget Function(Uint8List);

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

  void _startDownload() {
    gDownloadManager.download(widget.url).then((value) {
      if (!mounted) {
        return;
      }

      setState(() {
        _status = DownloadWidgetStatus.loaded;
        _data = value;
      });
    }).catchError((e) {
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
    if (_status == DownloadWidgetStatus.loaded) {
      return widget.buildLoaded(_data!);
    }

    if (_status == DownloadWidgetStatus.error) {
      return widget.buildError();
    }

    if (_status == DownloadWidgetStatus.uninitialized) {
      _status = DownloadWidgetStatus.waiting;
      _startDownload();
    }

    return widget.buildWaiting();
  }
}
