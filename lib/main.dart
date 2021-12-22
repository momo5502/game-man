import 'package:flutter/material.dart';

import 'no_glow_scroll_behaviour.dart';
import 'pages/game_page.dart';

void main() {
  runApp(const GameManApp());
}

class GameManApp extends StatelessWidget {
  const GameManApp({Key? key}) : super(key: key);

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'GameMan',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: const GamePage(),
      builder: (context, child) {
        return ScrollConfiguration(
          behavior: NoGlowScrollBehavior(),
          child: child!,
        );
      },
    );
  }
}
