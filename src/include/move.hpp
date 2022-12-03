#pragma once

#include "board.hpp"
#include "defs.hpp"

#include <string>

namespace Move {

enum class MoveType { allMoves, onlyCaptures };

struct MoveList {
    std::array<int, 256> list;
    short count = 0;
    MoveList();
    void add(const int move);
    int search(const int source, const int target, const int promoted = (int)Piece::E) const;
    void printList() const;
};

int encode(int source, int target, int piece, int promoted, bool isCapture, bool isTwoSquarePush,
           bool isEnpassant, bool isCastling);
int getSource(const int move);
int getTarget(const int move);
int getPiece(const int move);
int getPromoted(const int move);
bool isCapture(const int move);
bool isTwoSquarePush(const int move);
bool isEnpassant(const int move);
bool isCastling(const int move);
std::string toString(const int move);
int parse(const std::string& moveStr, const Board& board);
void generate(MoveList& moveList, const Board& board);
void generatePawns(MoveList& moveList, const Board& board);
void generateKnights(MoveList& moveList, const Board& board);
void generateBishops(MoveList& moveList, const Board& board);
void generateRooks(MoveList& moveList, const Board& board);
void generateQueens(MoveList& moveList, const Board& board);
void generateKings(MoveList& moveList, const Board& board);
void genWhiteCastling(MoveList& moveList, const Board& board);
void genBlackCastling(MoveList& moveList, const Board& board);
bool make(Board* main, const int move, MoveType moveFlag);

} // namespace Move
