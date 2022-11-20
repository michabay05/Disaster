#pragma once

#include "board.h"
#include "defs.h"

#include <string>

namespace Move {

enum MoveType { allMoves, onlyCaptures };

struct MoveList {
  int list[256];
  short count = 0;
  void add(int move);
  void printList();
};

int encode(int source, int target, int piece, int promoted, bool isCapture,
           bool isTwoSquarePush, bool isEnpassant, bool isCastling);
int getSource(const int move);
int getTarget(const int move);
int getPiece(const int move);
int getPromoted(const int move);
bool isCapture(const int move);
bool isTwoSquarePush(const int move);
bool isEnpassant(const int move);
bool isCastling(const int move);
const std::string toString(const int move);
void generate(MoveList &moveList, Board &board);
void generatePawns(MoveList &moveList, Board &board);
void generateKnights(MoveList &moveList, Board &board);
void generateBishops(MoveList &moveList, Board &board);
void generateRooks(MoveList &moveList, Board &board);
void generateQueens(MoveList &moveList, Board &board);
void generateKings(MoveList &moveList, Board &board);
void genCastling(MoveList &moveList, Board &board);
bool make(Board *main, int move, MoveType moveFlag);

} // namespace Move
