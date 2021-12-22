import 'dart:collection';
import 'dart:convert';

import 'package:game_man/download_manager.dart';
import 'package:html/dom.dart';
import 'package:http/http.dart' as http;
import 'package:html/parser.dart';

import 'package:game_man/games/game.dart';
import 'package:game_man/games/game_source.dart';

final Map<String, GameConsole> _consoleMapping = {
  "gameboy-advance": GameConsole.gameBoyAdvance,
  "gameboy-color": GameConsole.gameBoyColor,
  "gameboy": GameConsole.gameBoy,
};

String _consoleToString(GameConsole console) {
  for (final entry in _consoleMapping.entries) {
    if (entry.value == console) {
      return entry.key;
    }
  }

  return "unknown";
}

GameConsole _stringToConsole(String consoleName) {
  final entry = _consoleMapping[consoleName];
  if (entry == null) {
    return GameConsole.unknown;
  }

  return entry;
}

String? _extractIdentifier(String url) {
  // https://www.emulatorgames.net/roms/gameboy-advance/super-mario-advance-4-super-mario-bros-3-v11/
  String regexString = r'^.+//www.emulatorgames.net/roms/[^/]+/([^/]+)/?$';
  RegExp regExp = RegExp(regexString);
  final matches = regExp.allMatches(url);
  if (matches.length != 1) {
    return null;
  }

  final match = matches.first;
  final group = match.group(1);
  final identifier = group.toString();
  return identifier;
}

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
  return _stringToConsole(console);
}

Game? _extractGame(Element element) {
  final links = element.getElementsByTagName("a");
  final source = element.getElementsByTagName("source");
  final img = element.getElementsByTagName("img");

  if (source.isEmpty || img.isEmpty || links.isEmpty) {
    return null;
  }

  final link = links[0];
  final imageSource = source[0];
  final image = img[0];

  String? pageUrl = link.attributes["href"];
  String? imageUrl = imageSource.attributes["data-srcset"];
  String? name = image.attributes["alt"];

  if (imageUrl == null || name == null || pageUrl == null) {
    return null;
  }

  final console = _getConsoleFromUrl(imageUrl);
  final identifier = _extractIdentifier(pageUrl);

  if (console == GameConsole.unknown || identifier == null) {
    return null;
  }

  return Game(
    console,
    name,
    imageUrl,
    identifier,
  );
}

String? _extractPostId(String data) {
  Document doc = parse(data);
  final wrappers = doc.getElementsByClassName("site-wrapper");
  if (wrappers.isEmpty) {
    return null;
  }

  final wrapper = wrappers[0];
  final forms = wrapper.getElementsByTagName("form");
  if (forms.isEmpty) {
    return null;
  }

  final form = forms[0];
  final inputs = form.getElementsByTagName("input");
  if (inputs.isEmpty) {
    return null;
  }

  final input = inputs[0];
  final name = input.attributes["name"];
  if (name == null || name != "post_id") {
    return null;
  }

  return input.attributes["value"];
}

Future<String> _fetchPostId(Game game) async {
  var url = Uri.https('www.emulatorgames.net',
      '/roms/${_consoleToString(game.console)}/${game.identifier}');

  var response = await http.get(url);
  if (response.statusCode != 200) {
    throw Exception("Failed to get post id");
  }

  String body = String.fromCharCodes(response.bodyBytes);
  final postId = _extractPostId(body);
  if (postId == null) {
    throw Exception("Failed to extract post id");
  }

  return postId;
}

Future<String> _determineDownloadUrl(Game game, String postId) async {
  var referrerUrl =
      "https://www.emulatorgames.net/download/?rom=${game.identifier}";
  var url = Uri.https('www.emulatorgames.net', '/prompt/');

  var response = await http.post(
    url,
    headers: {
      "referer": referrerUrl,
    },
    body: {
      "get_type": "post",
      "get_id": postId,
    },
  );

  if (response.statusCode != 200) {
    throw Exception("Failed to get download url");
  }

  String body = String.fromCharCodes(response.bodyBytes);
  final info = jsonDecode(body);
  final downloadUrl = info[0] as String;
  return downloadUrl;
}

bool yay = false;

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
      final game = _extractGame(element);
      if (game != null) {
        result.add(game);

        if (!yay) {
          downloadGame(game);
          yay = true;
        }
      }
    }

    return result;
  }

  @override
  Future<GameDownload> downloadGame(Game game) async {
    final imageDownload = gDownloadManager.download(game.image);

    final postId = await _fetchPostId(game);
    final downloadUrl = await _determineDownloadUrl(game, postId);
    return Future.error("O");
  }
}
