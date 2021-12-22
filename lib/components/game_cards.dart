import 'package:flutter/material.dart';

import '../constants.dart';
import '../games/game.dart';
import '../pages/player_page.dart';
import 'game_card.dart';

class GameCards extends StatelessWidget {
  const GameCards({
    Key? key,
    required this.games,
  }) : super(key: key);

  final GameList games;

  GameCard _buildGameCard(BuildContext context, Game game) {
    return GameCard(
      image: game.image,
      title: game.name,
      onTap: () {
        Navigator.push(
            context,
            MaterialPageRoute(
              builder: (context) => PlayerPage(
                heroTag: game.image,
              ),
            ));
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    final Size size = MediaQuery.of(context).size;

    return AnimatedSwitcher(
      duration: const Duration(milliseconds: 100),
      child: SingleChildScrollView(
        key: ObjectKey(games),
        child: Column(
          children: [
            Container(height: size.height / 4.5 + (kDefaultPadding * 2)),
            Wrap(
              children:
                  games.map((game) => _buildGameCard(context, game)).toList(),
            ),
            Container(height: kDefaultPadding),
          ],
        ),
      ),
    );
  }
}