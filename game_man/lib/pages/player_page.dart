import 'dart:typed_data';
import 'dart:ui';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:game_man/games/game.dart';
import 'package:game_man/games/game_repository.dart';
import 'package:native_add/native_add.dart';
import '../constants.dart';

String activeGame = "";

class OpenGLTextureController {
  static const MethodChannel _channel = MethodChannel('opengl_texture');

  int? textureId;

  Future<int> initialize(double width, double height) async {
    getDynamicLib();

    textureId = await _channel.invokeMethod('create', {
      'width': width,
      'height': height,
    });
    print("Id: $textureId");
    return textureId!;
  }

  Future<Null> dispose() =>
      _channel.invokeMethod('dispose', {'textureId': textureId});

  bool get isInitialized => textureId != null;
}

class PlayerPage extends StatefulWidget {
  PlayerPage({
    Key? key,
    required this.game,
    required this.gameRepository,
  }) : super(key: key);

  final Game game;
  final GameRepository gameRepository;

  final OpenGLTextureController controller = OpenGLTextureController();

  @override
  State<PlayerPage> createState() => _PlayerPageState();
}

class _PlayerPageState extends State<PlayerPage>
    with SingleTickerProviderStateMixin {
  bool run = false;

  @override
  void initState() {
    super.initState();
    widget.controller.initialize(160 * 2, 144 * 2);
    Future.delayed(const Duration(milliseconds: 100)).then((value) {
      widget.gameRepository.getGameRom(widget.game).then((rom) {
        loadGbRom(rom);
        setState(() {
          run = true;
        });
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    return WillPopScope(
      child: Scaffold(
        backgroundColor: kBackgroundColor,
        body: AnnotatedRegion<SystemUiOverlayStyle>(
          value: const SystemUiOverlayStyle(
            statusBarColor: Colors.transparent,
            statusBarIconBrightness: Brightness.light,
            statusBarBrightness: Brightness.dark,
          ),
          child:
              buildPlayer(context, widget.game.image, run, widget.controller),
        ),
      ),
      onWillPop: () {
        setState(() {
          run = false;
        });
        return Future.value(true);
      },
    );
  }
}

Widget buildPlayer(BuildContext context, String heroTag, bool run,
    OpenGLTextureController controller) {
  return Column(
    children: [
      Hero(
          child: Screen(
            run: run,
            controller: controller,
          ),
          tag: heroTag),
      const GamePad(),
    ],
  );
}

class GamePad extends StatelessWidget {
  const GamePad({
    Key? key,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final Size size = MediaQuery.of(context).size;

    final bottomOffset = size.height / 6 + kDefaultPadding;

    return Expanded(
      child: Stack(
        children: [
          Positioned(
            right: kDefaultPadding,
            bottom: bottomOffset + kDefaultPadding * 1.5,
            child: TextGameButton(
              text: "A",
              onDown: () {
                pressGbButton(4, true);
              },
              onUp: () {
                pressGbButton(4, false);
              },
            ),
          ),
          Positioned(
            right: kDefaultPadding * 3.5,
            bottom: bottomOffset - kDefaultPadding * 0.5,
            child: TextGameButton(
              text: "B",
              onDown: () {
                pressGbButton(5, true);
              },
              onUp: () {
                pressGbButton(5, false);
              },
            ),
          ),
          Positioned(
            left: kDefaultPadding * 3,
            bottom: bottomOffset + kDefaultPadding * 2,
            child: IconGameButton(
              icon: Icons.keyboard_arrow_up_rounded,
              onDown: () {
                pressGbButton(0, true);
              },
              onUp: () {
                pressGbButton(0, false);
              },
            ),
          ),
          Positioned(
            left: kDefaultPadding * 3,
            bottom: bottomOffset - kDefaultPadding * 2,
            child: IconGameButton(
              icon: Icons.keyboard_arrow_down_rounded,
              onDown: () {
                pressGbButton(1, true);
              },
              onUp: () {
                pressGbButton(1, false);
              },
            ),
          ),
          Positioned(
            left: kDefaultPadding,
            bottom: bottomOffset,
            child: IconGameButton(
              icon: Icons.keyboard_arrow_left_rounded,
              onDown: () {
                pressGbButton(2, true);
              },
              onUp: () {
                pressGbButton(2, false);
              },
            ),
          ),
          Positioned(
            left: kDefaultPadding * (1 + 3 + 1),
            bottom: bottomOffset,
            child: IconGameButton(
              icon: Icons.keyboard_arrow_right_rounded,
              onDown: () {
                pressGbButton(3, true);
              },
              onUp: () {
                pressGbButton(3, false);
              },
            ),
          ),
          Positioned(
            bottom: kDefaultPadding,
            child: SizedBox(
              width: size.width,
              child: Align(
                child: Row(
                  children: [
                    const Spacer(),
                    FlatGameButton(
                      onDown: () {
                        pressGbButton(7, true);
                      },
                      onUp: () {
                        pressGbButton(7, false);
                      },
                      child: const Text(
                        "SELECT",
                        style: TextStyle(
                          color: Colors.white,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ),
                    const SizedBox(
                      width: kDefaultPadding,
                    ),
                    FlatGameButton(
                      onDown: () {
                        pressGbButton(6, true);
                      },
                      onUp: () {
                        pressGbButton(6, false);
                      },
                      child: const Text(
                        "START",
                        style: TextStyle(
                          color: Colors.white,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ),
                    const Spacer(),
                  ],
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class FlatGameButton extends StatelessWidget {
  const FlatGameButton({
    Key? key,
    this.child,
    required this.onDown,
    required this.onUp,
  }) : super(key: key);

  final Widget? child;
  final VoidCallback onDown;
  final VoidCallback onUp;

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTapDown: (a) => onDown(),
      onTapUp: (a) => onUp(),
      child: Container(
        width: kDefaultPadding * 2.5,
        height: kDefaultPadding,
        decoration: BoxDecoration(
            gradient: kDefaultGradient,
            borderRadius: const BorderRadius.all(Radius.circular(50)),
            boxShadow: [
              BoxShadow(
                offset: const Offset(0, 10),
                blurRadius: 20,
                color: Colors.black.withOpacity(0.23),
              )
            ]),
        child: Align(alignment: Alignment.center, child: child),
      ),
    );
  }
}

class GameButton extends StatelessWidget {
  const GameButton({
    Key? key,
    this.child,
    required this.onDown,
    required this.onUp,
  }) : super(key: key);

  final Widget? child;
  final VoidCallback onDown;
  final VoidCallback onUp;

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTapDown: (a) => onDown(),
      onTapUp: (a) => onUp(),
      child: Container(
        width: kDefaultPadding * 2.5,
        height: kDefaultPadding * 2.5,
        decoration: BoxDecoration(
            gradient: kDefaultGradient,
            borderRadius: const BorderRadius.all(Radius.circular(50)),
            boxShadow: [
              BoxShadow(
                offset: const Offset(0, 10),
                blurRadius: 20,
                color: Colors.black.withOpacity(0.23),
              )
            ]),
        child: Align(alignment: Alignment.center, child: child),
      ),
    );
  }
}

class TextGameButton extends StatelessWidget {
  const TextGameButton({
    Key? key,
    required this.text,
    required this.onDown,
    required this.onUp,
  }) : super(key: key);

  final VoidCallback onDown;
  final VoidCallback onUp;

  final String text;

  @override
  Widget build(BuildContext context) {
    return GameButton(
      onDown: onDown,
      onUp: onUp,
      child: Text(
        text,
        style: Theme.of(context).textTheme.button?.copyWith(
              color: Colors.white,
              fontSize: 30,
            ),
      ),
    );
  }
}

class IconGameButton extends StatelessWidget {
  const IconGameButton({
    Key? key,
    required this.icon,
    required this.onDown,
    required this.onUp,
  }) : super(key: key);

  final IconData icon;
  final VoidCallback onDown;
  final VoidCallback onUp;

  @override
  Widget build(BuildContext context) {
    return GameButton(
      onDown: onDown,
      onUp: onUp,
      child: Icon(
        icon,
        size: 30,
        color: Colors.white,
      ),
    );
  }
}

class Screen extends StatelessWidget {
  const Screen({
    Key? key,
    required this.run,
    required this.controller,
  }) : super(key: key);

  final bool run;
  final OpenGLTextureController controller;

  @override
  Widget build(BuildContext context) {
    final Size size = MediaQuery.of(context).size;

    return Container(
      width: size.width,
      height: size.height / 2,
      decoration: BoxDecoration(
        boxShadow: [
          BoxShadow(
            color: Colors.black.withOpacity(0.33),
            offset: const Offset(0, 10),
            blurRadius: 20,
          )
        ],
        gradient: kDefaultGradient,
      ),
      child: SafeArea(
        child: Align(
          alignment: Alignment.center,
          child: controller.isInitialized
              ? SizedBox(
                  width: 160 * 2,
                  height: 144 * 2,
                  child: Texture(textureId: controller.textureId!),
                )
              : null,
          /*LCDWidget(
              run: run,
            ),*/
        ),
      ),
    );
  }
}

class LCDWidget extends StatefulWidget {
  const LCDWidget({
    Key? key,
    required this.run,
  }) : super(key: key);

  final bool run;

  @override
  State<LCDWidget> createState() => LCDState();
}

class LCDState extends State<LCDWidget> with SingleTickerProviderStateMixin {
  @override
  Widget build(BuildContext context) {
    if (widget.run) {
      WidgetsBinding.instance?.addPostFrameCallback((_) {
        setState(() {});
      });
    }

    return CustomPaint(
        isComplex: true, willChange: true, painter: LCDPainter(widget.run));
  }
}

/// LCD painter is used to copy the LCD data from the gameboy PPU to the screen.
class LCDPainter extends CustomPainter {
  final bool run;

  /// Indicates if the LCD is drawing new content
  bool drawing = false;

  LCDPainter(this.run);

  @override
  void paint(Canvas canvas, Size size) {
    drawing = true;

    final buffer = getFrameBuffer();
    if (buffer == null) {
      drawing = false;
      return;
    }

    const int LCD_WIDTH = 160;
    const int LCD_HEIGHT = 144;

    int scale = 2;
    int width = LCD_WIDTH * scale;
    int height = LCD_HEIGHT * scale;

    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        Paint color = Paint();
        color.style = PaintingStyle.stroke;
        color.strokeWidth = 1.0;

        final val = buffer[(x ~/ scale) + (y ~/ scale) * LCD_WIDTH];
        color.color = Color(((val << 8) & 0xFFFFFF00) | (val >> 24) & 0xFF);

        List<double> points = [];
        points.add(x.toDouble() - width / 2.0);
        points.add(y.toDouble() + 10);

        canvas.drawRawPoints(
            PointMode.points, Float32List.fromList(points), color);
      }
    }

    drawing = false;
  }

  @override
  bool shouldRepaint(LCDPainter oldDelegate) {
    return !drawing && run;
  }
}
