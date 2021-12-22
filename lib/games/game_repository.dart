import 'dart:async';

import 'package:flutter/cupertino.dart';
import 'package:path_provider/path_provider.dart';

import 'game.dart';
import 'game_source.dart';

import 'sources/emulatornet_source.dart';

Future<String> get _localPath async {
  final directory = await getApplicationDocumentsDirectory();

  return directory.path;
}

class GameRepository {
  GameRepository();

  final GameSource _gameSource = EmulatorNetSource();
  final localGames = ValueNotifier<GameList>([]);

  void refreshLocalGames() {}

  Future<GameList> searchGames(String text) async {
    final games = await _gameSource.searchGames(text);
    return _removeDuplicates(games);
  }

  GameList _removeDuplicates(GameList games) {
    GameList finalGames = [];

    for (final game in games) {
      bool found = false;

      for (final addedGame in finalGames) {
        if (addedGame.name == game.name) {
          found = true;
          break;
        }
      }

      if (!found) {
        finalGames.add(game);
      }
    }

    return finalGames;
  }
}
