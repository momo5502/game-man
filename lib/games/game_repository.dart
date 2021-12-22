import 'dart:async';

import 'package:flutter/cupertino.dart';

import 'game.dart';
import 'game_source.dart';

import 'sources/emulatornet_source.dart';

class GameRepository {
  GameRepository();

  final GameSource _gameSource = EmulatorNetSource();
  final localGames = ValueNotifier<GameList>([]);

  void refreshLocalGames() {
    
  }

  Future<GameList> searchGames(String text) async {
    final games = await _gameSource.searchGames(text);

    return games;
  }
}
