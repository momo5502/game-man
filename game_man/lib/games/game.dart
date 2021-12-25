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

  Game(
    this.console,
    this.name,
    this.image,
    this.identifier,
  );

  Game.fromJson(Map<String, dynamic> json)
      : name = json['name'],
        image = json['image'],
        identifier = json['identifier'],
        console = GameConsole.values[json['console']];

  Map<String, dynamic> toJson() => {
        'console': console.index,
        'name': name,
        'image': image,
        'identifier': identifier,
      };
}

typedef GameList = List<Game>;
