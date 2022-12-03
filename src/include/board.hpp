#pragma once

#include "defs.hpp"
#include <array>
#include <string>

extern const std::string pieceStr;
extern const std::array<std::string, 65> strCoords;
extern const std::array<int, 64> castlingRights;

struct Position {
    std::array<uint64_t, 12> pieces;
    std::array<uint64_t, 3> units;

    Position();
};

struct State {
    Color side = Color::WHITE;
    Sq enpassant = Sq::noSq;
    int castling = 0;
    int fullMoves = 0;
    int halfMoves = 0;
    uint64_t posKey = 0ULL;
    uint64_t posLock = 0ULL;
    uint64_t pawnKey = 0ULL;

    State() = default;
};

struct Board {
    Position pos;
    State state;

    Board() = default;
    void updateUnits();
    void display() const;
    void printCastling() const;
};

enum class CastlingRights : uint8_t { wk, wq, bk, bq };

int getPieceOnSquare(const Position& pos, const int sq);

bool isSquareAttacked(const Color side, const int sq, const Position& pos);
void printAttacked(const Color side, const Position& pos);

/* ------ FEN -------*/
extern const std::array<std::string, 8> position;
void parseFen(const std::string& fenStr, Board& board);
