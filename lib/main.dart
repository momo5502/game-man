import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_svg/svg.dart';
import 'package:game_man/constants.dart';
import 'package:game_man/download_widget.dart';
import 'package:game_man/game_list.dart';
import 'package:game_man/player.dart';
import 'package:google_fonts/google_fonts.dart';

import 'no_glow_scroll_behaviour.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'GameMan',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: const MyHomePage(),
      builder: (context, child) {
        return ScrollConfiguration(
          behavior: NoGlowScrollBehavior(),
          child: child!,
        );
      },
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key}) : super(key: key);

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  final GameList _gameList = GameList();
  final _searchController = TextEditingController();

  @override
  void initState() {
    super.initState();
    _searchController.addListener(_searchGames);
  }

  @override
  void dispose() {
    super.dispose();
    _searchController.removeListener(_searchGames);
  }

  List<Game>? _games;
  String? _lastQuery;

  void _setGames(List<Game>? games) {
    setState(() {
      _games = games;
    });
  }

  void _searchGames() {
    final text = _searchController.text;
    _lastQuery = text;
    if (text.isEmpty) {
      _setGames(null);
    } else {
      Future.delayed(const Duration(milliseconds: 300), () {
        if (text != _lastQuery) {
          return;
        }

        _gameList.searchGames(text).then((games) {
          if (text == _lastQuery) {
            _setGames(games);
          }
        });
      });
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
            GameCards(games: _games ?? _gameList.getDownloadedGames()),
            Header(searchController: _searchController),
          ],
        ),
      ),
      /*bottomNavigationBar: Container(
        height: 60,
        decoration: BoxDecoration(
          color: Colors.white,
          boxShadow: [
            BoxShadow(
              offset: const Offset(0, -10),
              blurRadius: 20,
              color: Colors.black.withOpacity(0.23),
            ),
          ],
        ),
      ),*/
    );
  }
}

class GameCards extends StatelessWidget {
  const GameCards({
    Key? key,
    required this.games,
  }) : super(key: key);

  final List<Game> games;

  GameCard _buildGameCard(BuildContext context, Game game) {
    return GameCard(
      image: game.image,
      title: game.name,
      onTap: () {
        Navigator.push(
            context,
            MaterialPageRoute(
              builder: (context) => PlayerScreen(
                heroTag: game.image,
              ),
            ));
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    final Size size = MediaQuery.of(context).size;

    return SingleChildScrollView(
      key: games.isEmpty ? null : ObjectKey(games[0]),
      child: Column(
        children: [
          Container(height: size.height / 4.5 + (kDefaultPadding * 2)),
          Wrap(
            children:
                games.map((game) => _buildGameCard(context, game)).toList(),
          ),
          Container(height: kDefaultPadding),
        ],
      ),
    );
  }
}

class Header extends StatelessWidget {
  const Header({
    Key? key,
    required this.searchController,
  }) : super(key: key);

  final TextEditingController searchController;

  @override
  Widget build(BuildContext context) {
    final Size size = MediaQuery.of(context).size;

    return Stack(
      children: [
        Container(
          margin: const EdgeInsets.only(bottom: kDefaultPadding),
          padding: const EdgeInsets.symmetric(horizontal: kDefaultPadding),
          height: size.height / 4.5,
          decoration: BoxDecoration(
            boxShadow: [
              BoxShadow(
                color: Colors.black.withOpacity(0.33),
                offset: const Offset(0, 10),
                blurRadius: 20,
              )
            ],
            gradient: kDefaultGradient,
            borderRadius: const BorderRadius.only(
              bottomLeft: Radius.circular(39),
              bottomRight: Radius.circular(39),
            ),
          ),
          child: Center(
            child: Row(
              children: [
                const Spacer(),
                Text(
                  'GameMan',
                  style: GoogleFonts.quicksand(
                    textStyle: Theme.of(context).textTheme.headline5,
                    color: Colors.white,
                    fontSize: 40,
                    fontWeight: FontWeight.w500,
                  ),
                ),
                Container(width: 10),
                const Icon(
                  Icons.gamepad_rounded,
                  color: Colors.white,
                  size: 30.0,
                ),
                const Spacer(),
              ],
            ),
          ),
        ),
        Positioned(
          bottom: 0,
          left: 0,
          right: 0,
          child: Container(
            height: 54,
            alignment: Alignment.center,
            margin: const EdgeInsets.symmetric(horizontal: kDefaultPadding),
            padding: const EdgeInsets.symmetric(horizontal: kDefaultPadding),
            decoration: BoxDecoration(
                color: Colors.white,
                borderRadius: BorderRadius.circular(20),
                boxShadow: [
                  BoxShadow(
                    offset: const Offset(0, 10),
                    blurRadius: 50,
                    color: Colors.black.withOpacity(0.23),
                  ),
                ]),
            child: Row(
              children: [
                Expanded(
                  child: TextField(
                    controller: searchController,
                    decoration: InputDecoration(
                      hintText: "Search",
                      hintStyle: TextStyle(
                        color: kPrimaryColor.withOpacity(0.5),
                      ),
                      enabledBorder: InputBorder.none,
                      focusedBorder: InputBorder.none,
                    ),
                  ),
                ),
                searchController.text.isEmpty
                    ? const Icon(
                        Icons.search_rounded,
                        color: kPrimaryColor,
                      )
                    : GestureDetector(
                        onTap: () => searchController.clear(),
                        child: const Icon(
                          Icons.clear_rounded,
                          color: kPrimaryColor,
                        ),
                      ),
              ],
            ),
          ),
        ),
        SafeArea(
          child: Row(
            children: [
              TextButton(
                onPressed: () {
                  print("HI2");
                },
                child: SvgPicture.asset(
                  "assets/icons/menu.svg",
                  color: Colors.white,
                ),
              ),
              const Spacer(),
              TextButton(
                onPressed: () {
                  print("HI");
                },
                child:
                    const Icon(Icons.add_sharp, color: Colors.white, size: 30),
              ),
            ],
          ),
        ),
      ],
    );
  }
}

class GameCard extends StatelessWidget {
  const GameCard({
    Key? key,
    required this.image,
    required this.title,
    required this.onTap,
  }) : super(key: key);

  final String image, title;
  final GestureTapCallback onTap;

  Widget _getImage() {
    if (!image.startsWith("http")) {
      return Image.asset(
        image,
        fit: BoxFit.cover,
      );
    }

    return DownloadWidget(
        url: image,
        buildWaiting: () => Container(
              padding: const EdgeInsets.all(kDefaultPadding * 2),
              child: const CircularProgressIndicator(
                value: null,
                color: Colors.white,
              ),
            ),
        buildError: () => const Icon(
              Icons.error,
              color: Colors.white,
              size: 60,
            ),
        buildLoaded: (data) => Image.memory(
              data,
              fit: BoxFit.cover,
            ));
  }

  @override
  Widget build(BuildContext context) {
    final Size size = MediaQuery.of(context).size;

    return Container(
      margin: const EdgeInsets.all(kDefaultPadding * 0.5),
      width: size.width * 0.4,
      child: GestureDetector(
        onTap: onTap,
        child: Column(
          children: [
            ClipRRect(
              borderRadius: const BorderRadius.only(
                topLeft: Radius.circular(10),
                topRight: Radius.circular(10),
              ),
              child: Container(
                width: size.width * 0.4,
                height: size.width * 0.4,
                decoration: const BoxDecoration(
                  gradient: kDefaultGradient,
                ),
                child: Hero(
                  child: _getImage(),
                  tag: image,
                ),
              ),
            ),
            Container(
              width: size.width * 0.4,
              height: kDefaultPadding * 2.5,
              padding: const EdgeInsets.all(kDefaultPadding / 2),
              decoration: BoxDecoration(
                color: Colors.white,
                borderRadius: const BorderRadius.only(
                  bottomLeft: Radius.circular(10),
                  bottomRight: Radius.circular(10),
                ),
                boxShadow: [
                  BoxShadow(
                    offset: const Offset(0, 10),
                    blurRadius: 30,
                    color: Colors.black.withOpacity(0.23),
                  ),
                ],
              ),
              child: Align(
                alignment: Alignment.center,
                child: Text(
                  title,
                  style: Theme.of(context).textTheme.button,
                  overflow: TextOverflow.ellipsis,
                  maxLines: 2,
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
