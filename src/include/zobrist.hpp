#pragma once

#include "board.hpp"
#include "defs.hpp"

namespace Zobrist
{
extern std::array<std::array<uint64_t, 64>, 12> pieceKeys;
extern std::array<uint64_t, 8> enpassKeys;
extern std::array<uint64_t, 16> castlingKeys;
extern uint64_t sideKey;

extern std::array<std::array<uint64_t, 64>, 12> pieceLocks;
extern std::array<uint64_t, 8> enpassLocks;
extern std::array<uint64_t, 16> castlingLocks;
extern uint64_t sideLock;

void init();
uint64_t genKey(const Board& board);
uint64_t genLock(const Board& board);
uint64_t genPawnKey(const Board& board);

inline void togglePiece(Board& board, const int piece, const int sq)
{
    board.state.posKey ^= pieceKeys[piece][sq];
    board.state.posLock ^= pieceLocks[piece][sq];
}

inline void toggleEnpass(Board& board, const int sq)
{
    board.state.posKey ^= enpassKeys[COL(sq)];
    board.state.posLock ^= enpassLocks[COL(sq)];
}

inline void toggleCastling(Board& board, const int castlingRights)
{
    board.state.posKey ^= castlingKeys[castlingRights];
    board.state.posLock ^= castlingLocks[castlingRights];
}

inline void toggleSide(Board& board)
{
    board.state.posKey ^= sideKey;
    board.state.posLock ^= sideLock;
}

} // namespace Zobrist
