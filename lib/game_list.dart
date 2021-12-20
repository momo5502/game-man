import 'package:html/dom.dart';
import 'package:http/http.dart' as http;
import 'package:html/parser.dart';

enum Console {
  unknown,
  gameBoy,
  gameBoyColor,
  gameBoyAdvance,
}

class Game {
  final Console console;
  final String name;
  final String image;

  Game(this.console, this.name, this.image);
}

Console _getConsoleFromUrl(String url) {
// https://static.emulatorgames.net/images/gameboy-advance/pokemon-ultra-violet.webp
  String regexString = r'^.+//static.emulatorgames.net/images/([^/]+)/.*$';
  RegExp regExp = RegExp(regexString);
  final matches = regExp.allMatches(url);
  if (matches.length != 1) {
    return Console.unknown;
  }

  final match = matches.first;
  final group = match.group(1);
  final console = group.toString();
  if (console == "gameboy-advance") {
    return Console.gameBoyAdvance;
  }

  if (console == "gameboy-color") {
    return Console.gameBoyColor;
  }

  if (console == "gameboy") {
    return Console.gameBoy;
  }

  return Console.unknown;
}

class GameList {
  final List<Game> _games = [
    Game(
      Console.gameBoy,
      "Tetris (JUE) (V1.0)",
      "assets/images/tetris-jue-v1-0.webp",
    ),
    Game(
      Console.gameBoyAdvance,
      "Super Mario Advance 4 - Super Mario Bros. 3 (V1.1)",
      "assets/images/super-mario-advance-4-super-mario-bros-3-v1-1.webp",
    ),
    Game(
      Console.gameBoyAdvance,
      "Pokemon - Fire Red Version [a1]",
      "assets/images/pokemon-fire-red-version-v1-1.webp",
    ),
  ];

  List<Game> getGames() {
    return _games;
  }

  Future<List<Game>> searchGames(String text) async {
    var url = Uri.https('www.emulatorgames.net', '/search', {'kw': text});

    // Await the http get response, then decode the json-formatted response.
    var response = await http.get(url);
    if (response.statusCode != 200) {
      return [];
    }

    String body = String.fromCharCodes(response.bodyBytes);
    Document doc = parse(body);
    final lists = doc.getElementsByClassName("site-list");
    if (lists.isEmpty) {
      return [];
    }

    final list = lists[0];
    final elements = list.getElementsByTagName("li");

    List<Game> result = [];

    for (final element in elements) {
      final source = element.getElementsByTagName("source");
      final img = element.getElementsByTagName("img");
      if (source.isEmpty || img.isEmpty) {
        continue;
      }

      final imageSource = source[0];
      final image = img[0];

      String? imageUrl = imageSource.attributes["data-srcset"];
      String? name = image.attributes["alt"];

      if (imageUrl == null || name == null) {
        continue;
      }

      final console = _getConsoleFromUrl(imageUrl);
      if (console == Console.unknown) {
        continue;
      }

      result.add(
        Game(
          console,
          name,
          imageUrl,
        ),
      );
    }

    return result;
  }
}
