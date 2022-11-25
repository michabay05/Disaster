#pragma once

#include "defs.hpp"
#include <string>
#include <array>

extern const std::string pieceStr;
extern const std::array<std::string, 65> strCoords;
extern const std::array<int, 64> castlingRights;

struct Board {
  std::array<U64, 12> pieces;
  std::array<U64, 3> units;
  int side = 0;
  int enpassant = noSq;
  int castling = 0;
  int fullMoves = 0;
  int halfMoves = 0;
  U64 hashKey = 0ULL;
  U64 hashLock = 0ULL;

  Board();
  void updateUnits();
  void display() const;
  void printCastling() const;
};

enum CastlingRights { wk, wq, bk, bq };

int getPieceOnSquare(const Board &board, int sq);

bool isSquareAttacked(const int side, const int sq, const Board &board);
void printAttacked(const int side, const Board& board);
/* ------ FEN -------*/
extern const std::array<std::string, 8> position;
void parseFen(const std::string &fenStr, Board &board);
