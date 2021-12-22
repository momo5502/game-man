import 'dart:typed_data';

import 'package:game_man/games/game.dart';

class GameDownload {
  final Uint8List game;
  final Uint8List image;

  GameDownload(this.game, this.image);
}

abstract class GameSource {
  Future<GameList> searchGames(String text);
  Future<GameDownload> downloadGame(Game game);
}
