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
  final String identifier;

  Game(this.console, this.name, this.image, this.identifier);
}

typedef GameList = List<Game>;
