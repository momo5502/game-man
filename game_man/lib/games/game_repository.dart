import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:typed_data';

import 'package:flutter/cupertino.dart';
import 'package:flutter_archive/flutter_archive.dart';
import 'package:path/path.dart';
import 'package:path_provider/path_provider.dart';
import 'package:path/path.dart' as p;

import 'game.dart';
import 'game_source.dart';

import 'sources/emulatornet_source.dart';

Future<String> get _localPath async {
  final directory = await getApplicationDocumentsDirectory();

  return directory.path;
}

Future<String> get _cachePath async {
  Directory tempDir = await getTemporaryDirectory();
  return tempDir.path;
}

Future<String> get _romPath async {
  return p.join(await _localPath, "roms");
}

class GameRepository {
  GameRepository();

  final GameSource _gameSource = EmulatorNetSource();
  final localGames = ValueNotifier<GameList>([]);

  Future<Game> _loadGame(String path) async {
    var imagePath = p.join(path, "image.dat");
    final dataPath = p.join(path, "info.json");

    if (!await File(imagePath).exists()) {
      imagePath = "";
    }

    final gameData = await File(dataPath).readAsString();
    var game = Game.fromJson(jsonDecode(gameData));
    return Game(game.console, game.name, imagePath, game.identifier);
  }

  Future<void> importGame(String path) async {
    final file = File(path);
    final gameName = basenameWithoutExtension(file.path);
    final identifier = gameName.toLowerCase().replaceAll(" ", "-");

    final fileData = await file.readAsBytes();
    final game = Game(GameConsole.unknown, gameName, "", identifier);
    final gameDownload = GameDownload(fileData, Uint8List(0));

    await _storeGameDownload(game, gameDownload);
    await refreshLocalGames();
  }

  Future<void> refreshLocalGames() async {
    final dir = Directory(await _romPath);
    final entities = await dir.list().toList();
    final directories = entities.whereType<Directory>().toList().reversed;

    GameList newGames = [];

    for (final directory in directories) {
      final game = await _loadGame(directory.absolute.path);
      newGames.add(game);
    }

    localGames.value = newGames;
  }

  bool isLocalGame(Game game) {
    for (final localGame in localGames.value) {
      if (localGame.identifier == game.identifier) {
        return true;
      }
    }

    return false;
  }

  Future<void> deleteState(Game game) async {
    if (!isLocalGame(game)) {
      throw Exception("Game must have been downloaded");
    }

    final gamePath = p.join(await _romPath, game.identifier);
    final saveName = p.join(gamePath, "save.dat");
    await File(saveName).delete();
  }

  Future<void> storeState(Game game, Uint8List state) async {
    if (!isLocalGame(game)) {
      throw Exception("Game must have been downloaded");
    }

    final gamePath = p.join(await _romPath, game.identifier);
    final saveName = p.join(gamePath, "save.dat");
    await File(saveName).writeAsBytes(state);
  }

  Future<Uint8List> loadState(Game game) async {
    if (!isLocalGame(game)) {
      throw Exception("Game must have been downloaded");
    }

    final gamePath = p.join(await _romPath, game.identifier);
    final saveName = p.join(gamePath, "save.dat");
    return await File(saveName).readAsBytes();
  }

  Future<Uint8List> getGameRom(Game game) async {
    if (!isLocalGame(game)) {
      throw Exception("Game must have been downloaded");
    }

    final gamePath = p.join(await _romPath, game.identifier);
    final romName = p.join(gamePath, "rom.dat");

    String cacheDir = p.join(await _cachePath, "tempZip");
    final cacheDirectory = Directory(cacheDir);
    try {
      await cacheDirectory.delete(recursive: true);
    } catch (e) {}

    await ZipFile.extractToDirectory(
        zipFile: File(romName), destinationDir: cacheDirectory);

    final entities = await cacheDirectory.list().toList();
    final files = entities.whereType<File>().toList();
    for (final file in files) {
      if (file.path.endsWith(".gb") ||
          file.path.endsWith(".gba") ||
          file.path.endsWith(".gbc")) {
        return await file.readAsBytes();
      }
    }

    throw Exception("No rom found to load");
  }

  Future<void> deleteGame(Game game) async {
    final gamePath = p.join(await _romPath, game.identifier);
    await Directory(gamePath).delete(recursive: true);
    await refreshLocalGames();
  }

  Future<void> downloadGame(Game game) async {
    if (isLocalGame(game)) {
      return;
    }

    final download = await _gameSource.downloadGame(game);

    await _storeGameDownload(game, download);
    await refreshLocalGames();
  }

  Future<GameList> searchGames(String text) async {
    var games = await _gameSource.searchGames(text);
    // Only display gameboy games for now
    games = games.where((g) => g.console == GameConsole.gameBoy).toList();
    return _removeDuplicates(games);
  }

  Future<void> _storeGameDownload(Game game, GameDownload gameDownload) async {
    final gameData = jsonEncode(game);

    final gamePath = p.join(await _romPath, game.identifier);

    if (gameDownload.image.isNotEmpty) {
      final imageName = p.join(gamePath, "image.dat");
      final imageFile = await File(imageName).create(recursive: true);
      final imageFuture = imageFile.writeAsBytes(gameDownload.image);
      await imageFuture;
    }

    final romName = p.join(gamePath, "rom.dat");
    final romFile = await File(romName).create(recursive: true);
    final romFuture = romFile.writeAsBytes(gameDownload.game);

    final dataName = p.join(gamePath, "info.json");
    final dataFile = await File(dataName).create(recursive: true);
    final dataFuture = dataFile.writeAsString(gameData);

    await romFuture;
    await dataFuture;
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
