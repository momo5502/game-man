import 'package:file_picker/file_picker.dart';
import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';
import 'package:game_man/games/game_repository.dart';
import 'package:google_fonts/google_fonts.dart';

import '../constants.dart';

class SearchHeader extends StatelessWidget {
  const SearchHeader({
    Key? key,
    required this.searchController,
    required this.gameRepository,
  }) : super(key: key);

  final TextEditingController searchController;
  final GameRepository gameRepository;

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
                AnimatedCrossFade(
                  duration: const Duration(milliseconds: 100),
                  crossFadeState: searchController.text.isEmpty
                      ? CrossFadeState.showFirst
                      : CrossFadeState.showSecond,
                  firstChild: const Icon(
                    Icons.search_rounded,
                    color: kPrimaryColor,
                  ),
                  secondChild: GestureDetector(
                    onTap: () => searchController.clear(),
                    child: const Icon(
                      Icons.clear_rounded,
                      color: kPrimaryColor,
                    ),
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
                  importGame(gameRepository);
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

Future<void> importGame(GameRepository gameRepository) async {
  FilePickerResult? result = await FilePicker.platform.pickFiles(
    type: FileType.custom,
    allowedExtensions: ['zip', 'gb', 'gbc', 'gba'],
  );

  if (result == null) {
    return;
  }

  for (final file in result.files) {
    final String? filePath = file.path;
    if (filePath != null) {
      gameRepository.importGame(filePath);
    }
  }
}
