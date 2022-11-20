#pragma once

#include "defs.h"
#include <string>

extern char pieceStr[14];
extern const char *strCoords[65];
extern const int castlingRights[64];

struct Board {
  U64 pieces[12];
  U64 units[3];
  int side;
  int enpassant;
  int castling;
  int fullMoves;
  int halfMoves;

  Board();
  void updateUnits();
  void display();
  void printCastling();
};

enum CastlingRights { wk, wq, bk, bq };

int getPieceOnSquare(Board &board, int sq);

bool isSquareAttacked(int side, int sq, Board& board);
void printAttacked(int side, Board& board);
/* ------ FEN -------*/
extern const char *position[8];
void parseFen(const std::string fenStr, Board& board);
const char *genFen(Board &board);
