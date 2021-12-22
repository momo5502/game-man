import 'package:flutter/material.dart';
import 'package:game_man/constants.dart';

import 'download_widget.dart';
import 'fade_in.dart';

class GameCard extends StatelessWidget {
  const GameCard({
    Key? key,
    required this.image,
    required this.title,
    required this.onTap,
  }) : super(key: key);

  final String image, title;
  final GestureTapCallback onTap;

  Widget _getImage() {
    if (!image.startsWith("http")) {
      return Image.asset(
        image,
        fit: BoxFit.cover,
      );
    }

    return DownloadWidget(
      url: image,
      buildWaiting: () => Container(
        padding: const EdgeInsets.all(kDefaultPadding * 2),
        child: const CircularProgressIndicator(
          value: null,
          color: Colors.white,
        ),
      ),
      buildError: () => const Icon(
        Icons.error,
        color: Colors.white,
        size: 60,
      ),
      buildLoaded: (data, skipAnimation) => FadeIn(
        duration: const Duration(milliseconds: 100),
        skipAnimation: skipAnimation,
        child: Image.memory(
          data,
          fit: BoxFit.cover,
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final Size size = MediaQuery.of(context).size;

    return Container(
      margin: const EdgeInsets.all(kDefaultPadding * 0.5),
      width: size.width * 0.4,
      child: GestureDetector(
        onTap: onTap,
        child: Column(
          children: [
            ClipRRect(
              borderRadius: const BorderRadius.only(
                topLeft: Radius.circular(10),
                topRight: Radius.circular(10),
              ),
              child: Container(
                width: size.width * 0.4,
                height: size.width * 0.4,
                decoration: const BoxDecoration(
                  gradient: kDefaultGradient,
                ),
                child: Hero(
                  child: _getImage(),
                  tag: image,
                ),
              ),
            ),
            Container(
              width: size.width * 0.4,
              height: kDefaultPadding * 2.5,
              padding: const EdgeInsets.all(kDefaultPadding / 2),
              decoration: BoxDecoration(
                color: Colors.white,
                borderRadius: const BorderRadius.only(
                  bottomLeft: Radius.circular(10),
                  bottomRight: Radius.circular(10),
                ),
                boxShadow: [
                  BoxShadow(
                    offset: const Offset(0, 10),
                    blurRadius: 30,
                    color: Colors.black.withOpacity(0.23),
                  ),
                ],
              ),
              child: Align(
                alignment: Alignment.center,
                child: Text(
                  title,
                  style: Theme.of(context).textTheme.button,
                  overflow: TextOverflow.ellipsis,
                  maxLines: 2,
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
