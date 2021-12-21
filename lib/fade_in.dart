import 'package:flutter/material.dart';

class FadeIn extends StatefulWidget {
  const FadeIn({
    Key? key,
    required this.duration,
    required this.child,
    this.skipAnimation = false,
  }) : super(key: key);

  final Duration duration;
  final Widget child;
  final bool skipAnimation;

  @override
  State<FadeIn> createState() => _FadeInState();
}

class _FadeInState extends State<FadeIn> {
  bool _faded = false;

  @override
  void initState() {
    super.initState();

    WidgetsBinding.instance?.addPostFrameCallback((_) {
      setState(() {
        _faded = true;
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedOpacity(
      opacity: _faded || widget.skipAnimation ? 1.0 : 0.0,
      duration: widget.duration,
      child: widget.child,
    );
  }
}
