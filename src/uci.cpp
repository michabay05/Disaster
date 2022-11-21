#include "uci.hpp"

#include "board.hpp"
#include "misc.hpp"
#include "move.hpp"
#include "perft.hpp"
#include "search.hpp"

namespace UCI {

Board mainBoard;
bool quit = false;

void loop() {
  printEngineInfo();

  char input[2000];
  while (!quit) {
    memset(input, 0, sizeof(input));
    fflush(stdout);
    // Get input
    // If input is null, continue
    if (!fgets(input, 2000, stdin))
      continue;
    // Make sure input is available
    if (input[0] == '\n')
      // continue the loop
      continue;
    parse(input);
  }
}

void parse(char *command) {
  // ucinewgame command
  // if (command.find("quit") != std::string::npos) {
  if (!strncmp(command, "quit", 4)) {
    quit = true;
    return;
  } else if (!strncmp(command, "ucinewgame", 10))
    parsePos("position startpos");
  // uci command
  else if (!strncmp(command, "uci", 3))
    printf("uciok\n");
  // isready command
  else if (!strncmp(command, "isready", 7))
    printf("readyok\n");
  // position command
  else if (!strncmp(command, "position", 8))
    parsePos(command);
  // go command
  else if (!strncmp(command, "go", 2))
    parseGo(command);
  else if (!strncmp(command, "display", 7))
    mainBoard.display();
  else if (!strncmp(command, "help", 4))
    printHelpInfo();
  else
    printf("Unknown command: %s\n", command);
}

// TODO: add 'moves' feature
void parsePos(const char *command) {
  if (command == NULL)
    return;
  // Shift pointer to the beginning of args
  command += 9;
  // Reset board before setting piece up
  mainBoard = Board();
  char *currentChar = (char *)command;
  if (strncmp(command, "startpos", 8) == 0)
    parseFen(position[1], mainBoard);
  else if (strncmp(command, "fen", 3) == 0) {
    if (currentChar == NULL)
      parseFen(position[1], mainBoard);
    else {
      currentChar += 4;
      parseFen(currentChar, mainBoard);
    }
  }
  currentChar = (char *)strstr(command, "moves");
  if (currentChar != NULL) {
    currentChar += 6;
    while (*currentChar) {
      int move = Move::parse(currentChar, mainBoard);
      if (move == 0)
        break;
      Move::make(&mainBoard, move, Move::allMoves);
      // move current character mointer to the end of current move
      while (*currentChar && *currentChar != ' ')
        currentChar++;

      // go to the next move
      currentChar++;
    }
  }
}

void parseGo(const char *command) {
  // Shift pointer to the beginning of args
  command += 3;
  if (!strncmp(command, "perft", 5)) {
    command += 6;
    Perft::Test(mainBoard, atoi(command));
  } else if (!strncmp(command, "depth", 5)) {
    command += 6;
    Search::position(mainBoard, atoi(command));
  }
}

void printEngineInfo() {
  printf("id name Disaster\n");
  printf("id author michabay05\n");
  printf("uciok\n");
}

void printHelpInfo() { printf("Here are a list of all the commands\n"); }
} // namespace UCI
