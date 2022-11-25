#include "uci.hpp"

#include "board.hpp"
#include "misc.hpp"
#include "move.hpp"
#include "perft.hpp"
#include "search.hpp"

namespace UCI {

Board mainBoard;
bool quit = false;
bool stop = false;
bool isInfinite = false;
bool isTimeControlled = false;

int timeLeft = -1;
int increment = 0;
int movesToGo = 40;
int moveTime = -1;
long startTime = 0L;
long stopTime = 0L;

void loop() {
  printEngineInfo();

  std::string input;
  while (!quit) {
    input = "";
    // Get input
    std::getline(std::cin, input);
    // If input is null, continue
    if (input.empty())
      continue;
    // Make sure input is available
    if (input[0] == '\n')
      // continue the loop
      continue;
    parse(input);
  }
}

void parse(const std::string &command) {
  if (command.compare(0, 4, "quit") == 0) {
    quit = true;
    return;
  } else if (command.compare(0, 10, "ucinewgame") == 0)
    parsePos("position startpos");
  // uci command
  else if (command.compare(0, 3, "uci") == 0)
    printf("uciok\n");
  // isready command
  else if (command.compare(0, 7, "isready") == 0)
    printf("readyok\n");
  // position command
  else if (command.compare(0, 8, "position") == 0)
    parsePos(command);
  // go command
  else if (command.compare(0, 2, "go") == 0)
    parseGo(command);
  else if (command.compare(0, 7, "display") == 0)
    mainBoard.display();
  else if (command.compare(0, 4, "help") == 0)
    printHelpInfo();
  else
    printf("Unknown command: %s\n", command.c_str());
}

// TODO: add 'moves' feature
// void parsePos(const char *command) {
void parsePos(const std::string &command) {
  if (command.empty())
    return;
  // Shift pointer to the beginning of args
  int currentInd = 9;
  // Reset board before setting piece up
  mainBoard = Board();
  if (command.compare(currentInd, 8, "startpos") == 0) {
    currentInd += 8 + 1; // (+ 1) for the space
    parseFen(position[1], mainBoard);
  } else if (command.compare(currentInd, 3, "fen") == 0) {
    currentInd += 3 + 1;
    // 4k3/8/4K3/8/8/5Q2/8/8 b - - 0 1
    parseFen(command.substr(currentInd), mainBoard);
  }
  currentInd = (int)command.find("moves", currentInd);
  if (currentInd == std::string::npos)
    return;
  currentInd += 6;
  std::string moveStr;
  int move;
  for (int i = currentInd; i <= command.length(); i++) {
    if (std::isdigit(command[i]) || std::isalpha(command[i]))
      moveStr += command[i];
    else {
      move = Move::parse(moveStr, mainBoard);
      if (move == 0)
        continue;
      Move::make(&mainBoard, move, Move::allMoves);
      moveStr = "";
    }
  }
}

// void parseGo(const char *command) {
void parseGo(const std::string &command) {
  // Reset time control related variables
  quit = false;
  stop = false;
  isTimeControlled = false;
  timeLeft = -1;
  increment = 0;
  movesToGo = 40;
  moveTime = -1;
  // Shift pointer to the beginning of args
  int currentInd = 3;
  if (command.compare(currentInd, 5, "perft") == 0) {
    currentInd += 5 + 1;
    Perft::Test(mainBoard, atoi(command.substr(currentInd).c_str()));
    return;
  } else if (command.compare(currentInd, 5, "depth") == 0) {
    currentInd += 5 + 1;
    Search::position(mainBoard, atoi(command.substr(currentInd).c_str()));
    return;
  }

  if (mainBoard.side != WHITE) {
    parseParam(command.substr(currentInd), "wtime", timeLeft);
    parseParam(command.substr(currentInd), "winc", increment);
  } else {
    parseParam(command.substr(currentInd), "btime", timeLeft);
    parseParam(command.substr(currentInd), "binc", increment);
  }
  printf("timeLeft = %d\n", timeLeft);
  printf("increment = %d\n", increment);
}

void parseParam(const std::string &cmdArgs, const std::string &cmdName,
                int &output) {
  size_t currentIndex, nextSpaceInd;
  std::string param;
  if ((currentIndex = cmdArgs.find(cmdName)) != std::string::npos &&
      cmdArgs.length() >= cmdName.length() + 1) {
    param = cmdArgs.substr(currentIndex + cmdName.length() + 1);
    if ((nextSpaceInd = param.find(" ")) != std::string::npos)
      param = param.substr(0, nextSpaceInd);
    output = std::stoi(param);
  }
}

void checkUp() {}

void printEngineInfo() {
  printf("id name Disaster\n");
  printf("id author michabay05\n");
  printf("uciok\n");
}

void printHelpInfo() {
  printf("              Command name                 |         Description\n");
  printf("---------------------------------------------------------------------"
         "----------------------------------\n");
  printf("                  uci                      |    Prints engine info "
         "and 'uciok'\n");
  printf("              isready                      |    Prints 'readyok' if "
         "the engine is ready\n");
  printf("    position startpos                      |    Set board to "
         "starting position\n");
  printf("    position startpos moves <move1> ...    |    Set board to "
         "starting position then playing following moves\n");
  printf("   position fen <FEN>                      |    Set board to a "
         "custom FEN\n");
  printf("   position fen <FEN> moves <move1> ...    |    Set board to a "
         "custom FEN then playing following moves\n");
  printf("     go depth <depth>                      |    Returns the best "
         "move after search for given amount of depth\n");
  printf("     go movetime <time>                    |    Returns the best "
         "move given the time for a single move\n");
  printf("go (wtime/btime) <time>(winc/binc) <time>  |    Returns the best "
         "move given the total amount of time for a move with increment\n");
  printf("                 quit                      |    Exit the UCI mode\n");
  printf("\n------------------------------------ EXTENSIONS "
         "----------------------------------------\n");
  printf("              display                      |    Display board\n");
  printf("     go perft <depth>                      |    Calculate the total "
         "number of moves from a position for a given depth\n");
}

} // namespace UCI
