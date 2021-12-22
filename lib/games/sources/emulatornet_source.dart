import 'package:html/dom.dart';
import 'package:http/http.dart' as http;
import 'package:html/parser.dart';

import 'package:game_man/games/game.dart';
import 'package:game_man/games/game_source.dart';

GameConsole _getConsoleFromUrl(String url) {
  // https://static.emulatorgames.net/images/gameboy-advance/pokemon-ultra-violet.webp
  String regexString = r'^.+//static.emulatorgames.net/images/([^/]+)/.*$';
  RegExp regExp = RegExp(regexString);
  final matches = regExp.allMatches(url);
  if (matches.length != 1) {
    return GameConsole.unknown;
  }

  final match = matches.first;
  final group = match.group(1);
  final console = group.toString();
  if (console == "gameboy-advance") {
    return GameConsole.gameBoyAdvance;
  }

  if (console == "gameboy-color") {
    return GameConsole.gameBoyColor;
  }

  if (console == "gameboy") {
    return GameConsole.gameBoy;
  }

  return GameConsole.unknown;
}

class EmulatorNetSource extends GameSource {
  @override
  Future<GameList> searchGames(String text) async {
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
      if (console == GameConsole.unknown) {
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

  @override
  Future<void> downloadGame(Game game) {
    // TODO: implement downloadGame
    throw UnimplementedError();
  }
}
