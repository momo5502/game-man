import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:game_man/components/game_cards.dart';
import 'package:game_man/components/search_header.dart';
import 'package:game_man/games/game.dart';

import '../constants.dart';
import '../games/game_repository.dart';

class GamePage extends StatefulWidget {
  const GamePage({Key? key}) : super(key: key);

  @override
  State<GamePage> createState() => _GamePageState();
}

class _GamePageState extends State<GamePage> {
  final GameRepository _gameRepository = GameRepository();
  final _searchController = TextEditingController();

  GameList? _currentGames;
  String? _lastQuery;

  void _onLocalGamesChanged() {
    if (mounted) {
      setState(() {});
    }
  }

  @override
  void initState() {
    super.initState();

    _gameRepository.localGames.addListener(_onLocalGamesChanged);
    _searchController.addListener(_searchGames);
  }

  @override
  void dispose() {
    super.dispose();
    _searchController.removeListener(_searchGames);
    _gameRepository.localGames.removeListener(_onLocalGamesChanged);
  }

  GameList _getGames() {
    return _currentGames ?? _gameRepository.localGames.value;
  }

  void _setGames(List<Game>? games) {
    setState(() {
      _currentGames = games;
    });
  }

  void _searchGames() {
    final text = _searchController.text;
    if (text == _lastQuery) {
      return;
    }

    _lastQuery = text;
    if (text.isEmpty) {
      _setGames(null);
    } else {
      Future.delayed(const Duration(milliseconds: 300), () {
        if (text != _lastQuery) {
          return;
        }

        _gameRepository.searchGames(text).then((games) {
          if (text == _lastQuery) {
            _setGames(games);
          }
        });
      });

      setState(() {});
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: kBackgroundColor,
      body: AnnotatedRegion<SystemUiOverlayStyle>(
        value: const SystemUiOverlayStyle(
          statusBarColor: Colors.transparent,
          statusBarIconBrightness: Brightness.light,
          statusBarBrightness: Brightness.dark,
        ),
        child: Stack(
          children: [
            GameCards(games: _getGames()),
            SearchHeader(searchController: _searchController),
          ],
        ),
      ),
    );
  }
}
