#include "uci.hpp"

#include "board.hpp"
#include "misc.hpp"
#include "move.hpp"
#include "perft.hpp"
#include "search.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

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
int64_t startTime = 0L;
int64_t stopTime = 0L;

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
        if (input == "quit")
            break;
        parse(input);
    }
}

void parse(const std::string& command) {
    if (command.compare(0, 4, "stop") == 0) {
        printf("Stopping whatever I'm doing...\n");
        stop = true;
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

void parsePos(const std::string& command) {
    if (command.empty())
        return;
    // Shift pointer to the beginning of args
    int currentInd = 9;
    // Reset board before setting piece up
    mainBoard = Board();
    if (command.compare(currentInd, 8, "startpos") == 0) {
        currentInd += 8 + 1; // (+ 1) for the space
        mainBoard = Board();
    } else if (command.compare(currentInd, 3, "fen") == 0) {
        currentInd += 3 + 1;
        mainBoard = Board(command.substr(currentInd));
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
            Move::make(&mainBoard, move, Move::MoveType::allMoves);
            moveStr = "";
        }
    }
}

void parseGo(const std::string& command) {
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
    int depth = -1;
    if (command.compare(currentInd, 5, "perft") == 0) {
        currentInd += 5 + 1;
        Perft::test(mainBoard, atoi(command.substr(currentInd).c_str()));
        return;
    } else if (command.compare(currentInd, 5, "depth") == 0) {
        currentInd += 5 + 1;
        depth = atoi(command.substr(currentInd).c_str());
        Search::position(mainBoard, depth);
        return;
    }

    if (mainBoard.state.side == Color::WHITE) {
        parseParam(command.substr(currentInd), "wtime", timeLeft);
        parseParam(command.substr(currentInd), "winc", increment);
    } else {
        parseParam(command.substr(currentInd), "btime", timeLeft);
        parseParam(command.substr(currentInd), "binc", increment);
    }
    parseParam(command.substr(currentInd), "movetime", moveTime);
    parseParam(command.substr(currentInd), "movestogo", movesToGo);

    if (moveTime != -1) {
        timeLeft = moveTime;
        movesToGo = 1;
    }
    startTime = Time::now();
    if (timeLeft != -1) {
        isTimeControlled = true;
        timeLeft /= movesToGo;
        // Just to be safe, reduce time per move by 50 ms
        if (timeLeft > 1500)
            timeLeft -= 50;
        if (timeLeft < 1500 && increment && depth == Search::MAX_PLY)
            stopTime = startTime + increment - 50;
        else
            stopTime = startTime + increment + timeLeft;
    }

    if (depth == -1)
        depth = Search::MAX_PLY;
    Search::position(mainBoard, depth);
}

void parseParam(const std::string& cmdArgs, const std::string& cmdName, int& output) {
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

void checkUp() {
    if (isTimeControlled && Time::now() >= stopTime)
        stop = true;
}

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

/*
int input_waiting() {
#ifdef _WIN32
  fd_set readfds;
  struct timeval tv;
  FD_ZERO(&readfds);
  FD_SET(fileno(stdin), &readfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  select(16, &readfds, 0, 0, &tv);

  return (FD_ISSET(fileno(stdin), &readfds));
#else
  static int init = 0, pipe;
  static HANDLE inh;
  DWORD dw;

  if (!init) {
    init = 1;
    inh = GetStdHandle(STD_INPUT_HANDLE);
    pipe = !GetConsoleMode(inh, &dw);
    if (!pipe) {
      SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
      FlushConsoleInputBuffer(inh);
    }
  }

  if (pipe) {
    if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL))
      return 1;
    return dw;
  }

  else {
    GetNumberOfConsoleInputEvents(inh, &dw);
    return dw <= 1 ? 0 : dw;
  }

#endif
}

// read GUI/user input
void read_input() {
  // bytes to read holder
  int bytes;

  // GUI/user input
  std::string input = "";
  char *endc;

  // "listen" to STDIN
  if (input_waiting()) {
    // tell engine to stop calculating
    stop = true;

    // loop to read bytes from STDIN
    do {
      // read bytes from STDIN
      bytes = read(_fileno(stdin), input, 256);
    }

    // until bytes available
    while (bytes < 0);

    // searches for the first occurrence of '\n'
    endc = strchr(input, '\n');

    // if found new line set value at pointer to 0
    if (endc)
      *endc = 0;

    // if input is available
    if (strlen(input) > 0) {
      // match UCI "quit" command
      if (!strncmp(input, "quit", 4))
        // tell engine to terminate exacution
        quit = 1;

      // // match UCI "stop" command
      else if (!strncmp(input, "stop", 4))
        // tell engine to terminate exacution
        quit = 1;
    }
  }
}*/

} // namespace UCI
