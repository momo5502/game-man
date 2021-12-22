enum GameConsole {
  unknown,
  gameBoy,
  gameBoyColor,
  gameBoyAdvance,
}

class Game {
  final GameConsole console;
  final String name;
  final String image;

  Game(this.console, this.name, this.image);
}

typedef GameList = List<Game>;
