import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:game_man/constants.dart';

class PlayerScreen extends StatelessWidget {
  const PlayerScreen({
    Key? key,
    required this.heroTag,
  }) : super(key: key);

  final String heroTag;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: kBackgroundColor,
      body: AnnotatedRegion<SystemUiOverlayStyle>(
        value: const SystemUiOverlayStyle(
          statusBarColor: Colors.transparent,
          statusBarIconBrightness: Brightness.light,
          statusBarBrightness: Brightness.dark,
        ),
        child: buildPlayer(context, heroTag),
      ),
    );
  }
}

Widget buildPlayer(BuildContext context, String heroTag) {
  return Column(
    children: [
      Hero(child: const Screen(), tag: heroTag),
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

    return SizedBox(
      width: size.width,
      height: size.height / 2,
      child: Stack(
        children: [
          Positioned(
            right: kDefaultPadding,
            bottom: bottomOffset + kDefaultPadding * 1.5,
            child: const TextGameButton(text: "A"),
          ),
          Positioned(
            right: kDefaultPadding * 3.5,
            bottom: bottomOffset - kDefaultPadding * 0.5,
            child: const TextGameButton(text: "B"),
          ),
          Positioned(
            left: kDefaultPadding * 3,
            bottom: bottomOffset + kDefaultPadding * 2,
            child: const IconGameButton(icon: Icons.keyboard_arrow_up_rounded),
          ),
          Positioned(
            left: kDefaultPadding * 3,
            bottom: bottomOffset - kDefaultPadding * 2,
            child:
                const IconGameButton(icon: Icons.keyboard_arrow_down_rounded),
          ),
          Positioned(
              left: kDefaultPadding,
              bottom: bottomOffset,
              child: const IconGameButton(
                  icon: Icons.keyboard_arrow_left_rounded)),
          Positioned(
              left: kDefaultPadding * (1 + 3 + 1),
              bottom: bottomOffset,
              child: const IconGameButton(
                  icon: Icons.keyboard_arrow_right_rounded)),
        ],
      ),
    );
  }
}

class GameButton extends StatelessWidget {
  const GameButton({
    Key? key,
    this.child,
  }) : super(key: key);

  final Widget? child;

  @override
  Widget build(BuildContext context) {
    return Container(
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
    );
  }
}

class TextGameButton extends StatelessWidget {
  const TextGameButton({Key? key, required this.text}) : super(key: key);

  final String text;

  @override
  Widget build(BuildContext context) {
    return GameButton(
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
  const IconGameButton({Key? key, required this.icon}) : super(key: key);

  final IconData icon;

  @override
  Widget build(BuildContext context) {
    return GameButton(
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
  }) : super(key: key);

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
    );
  }
}
