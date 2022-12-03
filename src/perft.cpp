#include "perft.hpp"

#include "bitboard.hpp"
#include "misc.hpp"
#include "move.hpp"
#include "zobrist.hpp"

namespace Perft {

uint64_t totalNodes;

void driver(Board& board, int depth) {
    if (depth == 0) {
        totalNodes++;
        return;
    }
    Move::MoveList moveList;
    Move::generate(moveList, board);
    Board clone;
    for (int i = 0; i < moveList.count; i++) {
        // Clone the current state of the board
        clone = board;
        // Make move if it's not illegal (or check)
        if (!Move::make(&board, moveList.list[i], Move::MoveType::allMoves))
            continue;

        driver(board, depth - 1);

        // Restore board state
        board = clone;
        /* ============= FOR DEBUG PURPOSES ONLY ===============/
        uint64_t updatedKey = board.state.posKey;
        uint64_t updatedLock = board.state.posLock;
        Zobrist::genKey(board);
        Zobrist::genLock(board);
        if (board.state.posKey != updatedKey) {
            std::cout << "\nBoard.MakeMove(" << Move::toString(moveList.list[i]) << ")\n";
            board.display();
            std::cout << "Key should've been " << std::hex << board.state.posKey
                      << "ULL instead of 0x" << updatedKey << std::dec << "ULL\n";
        }
        if (board.state.posLock != updatedLock) {
            std::cout << "\nBoard.MakeMove(" << Move::toString(moveList.list[i]) << ")\n";
            board.display();
            std::cout << "Key should've been " << std::hex << board.state.posKey
                      << "ULL instead of 0x" << updatedKey << std::dec << "ULL\n";
        }
        /*============= FOR DEBUG PURPOSES ONLY =============== */
    }
}

void test(Board& board, const int depth) {
    std::cout << "\n----------------- Performance Test (" << depth << ") -----------------\n";
    totalNodes = 0L;
    Move::MoveList moveList;
    Move::generate(moveList, board);
    Time::start();
    Board clone;
    for (int i = 0; i < moveList.count; i++) {
        // Clone the current state of the board
        clone = board;
        // Make move if it's not illegal (or check)
        if (!Move::make(&board, moveList.list[i], Move::MoveType::allMoves))
            continue;

        uint64_t nodesSearchedSoFar = totalNodes;
        driver(board, depth - 1);

        // Restore board state
        board = clone;

        std::cout << "     " << Move::toString(moveList.list[i]) << ": "
                  << (totalNodes - nodesSearchedSoFar) << "\n";
    }
    std::cout << "\n     Depth: " << depth << "\n";
    std::cout << "     Nodes: " << totalNodes << "\n";
    std::cout << "      Time: " << Time::end() << "\n";
}
} // namespace Perft
