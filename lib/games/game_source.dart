import 'package:game_man/games/game.dart';

abstract class GameSource {
  Future<GameList> searchGames(String text);
  Future<void> downloadGame(Game game);
}
