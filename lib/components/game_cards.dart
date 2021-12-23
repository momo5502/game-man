import 'package:flutter/material.dart';
import '../games/game_repository.dart';

import '../constants.dart';
import '../games/game.dart';
import '../pages/player_page.dart';
import 'game_card.dart';

class GameCards extends StatelessWidget {
  const GameCards({
    Key? key,
    required this.games,
    required this.gameRepository,
  }) : super(key: key);

  final GameList games;
  final GameRepository gameRepository;

  void _showDeleteDialog(BuildContext context, Game game) {
    Widget cancelButton = TextButton(
      child: const Text("Cancel"),
      onPressed: () {
        Navigator.of(context).pop();
      },
    );
    Widget deleteButton = TextButton(
      child: const Text(
        "Delete",
        style: TextStyle(color: Colors.red),
      ),
      onPressed: () {
        gameRepository.deleteGame(game);
        Navigator.of(context).pop();
      },
    );

    showDialog(
        context: context,
        builder: (context) => AlertDialog(
              title: const Text("Delete"),
              content: Text("Are you sure you want to delete ${game.name}?"),
              actions: [
                deleteButton,
                cancelButton,
              ],
              shape: const RoundedRectangleBorder(
                  borderRadius: BorderRadius.all(Radius.circular(20.0))),
            ));
  }

  void _showDownloadDialog(BuildContext context, Game game) {
    Widget cancelButton = TextButton(
      child: const Text("Cancel"),
      onPressed: () {
        Navigator.of(context).pop();
      },
    );
    Widget continueButton = TextButton(
      child: const Text("Download"),
      onPressed: () {
        gameRepository.downloadGame(game);
        Navigator.of(context).pop();
      },
    );

    showDialog(
        context: context,
        builder: (context) => AlertDialog(
              title: const Text("Download"),
              content: Text("Do you want to download ${game.name}?"),
              actions: [
                continueButton,
                cancelButton,
              ],
              shape: const RoundedRectangleBorder(
                  borderRadius: BorderRadius.all(Radius.circular(20.0))),
            ));
  }

  GameCard _buildGameCard(BuildContext context, Game game) {
    return GameCard(
      isLocal: gameRepository.isLocalGame(game),
      image: game.image,
      title: game.name,
      onTap: () {
        if (gameRepository.isLocalGame(game)) {
          Navigator.push(
              context,
              MaterialPageRoute(
                builder: (context) => PlayerPage(
                  heroTag: game.image,
                ),
              ));
        } else {
          _showDownloadDialog(context, game);
        }
      },
      onLongPress: () {
        if (gameRepository.isLocalGame(game)) {
          _showDeleteDialog(context, game);
        }
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
