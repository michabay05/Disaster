#include "board.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "attack.hpp"
#include "magics.hpp"
#include "zobrist.hpp"

const std::string pieceStr = "PNBRQKpnbrqk ";

const std::array<std::string, 65> strCoords = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "a7", "b7", "c7",
    "d7", "e7", "f7", "g7", "h7", "a6", "b6", "c6", "d6", "e6", "f6",
    "g6", "h6", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "a4",
    "b4", "c4", "d4", "e4", "f4", "g4", "h4", "a3", "b3", "c3", "d3",
    "e3", "f3", "g3", "h3", "a2", "b2", "c2", "d2", "e2", "f2", "g2",
    "h2", "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", " "};

const std::array<int, 64> castlingRights = {
    7,  15, 15, 15, 3,  15, 15, 11, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 13, 15, 15, 15, 12, 15, 15, 14};

Board::Board() {
  pieces.fill(0);
  units.fill(0);
}

void Board::display() const {
  printf("\n    +---+---+---+---+---+---+---+---+\n");
  for (int r = 0; r < 8; r++) {
    printf("  %d |", 8 - r);
    for (int f = 0; f < 8; f++) {
      printf(" %c |", pieceStr[getPieceOnSquare(*this, SQ(r, f))]);
    }
    printf("\n    +---+---+---+---+---+---+---+---+\n");
  }
  printf("      a   b   c   d   e   f   g   h\n\n");
  printf("      Side to move: %s\n", !side ? "white" : "black");
  printCastling();
  printf("         Enpassant: %s\n",
         enpassant != -1 ? strCoords[enpassant].c_str() : "noSq");
  printf("        Full moves: %d\n", fullMoves);
  printf("          Hash key: 0x%llx\n", hashKey);
  printf("         Hash lock: 0x%llx\n\n", hashLock);
}

void Board::printCastling() const {
  std::string castlingLtrs = "----";
  if (castling & (1 << wk))
    castlingLtrs[0] = 'K';
  if (castling & (1 << wq))
    castlingLtrs[1] = 'Q';
  if (castling & (1 << bk))
    castlingLtrs[2] = 'k';
  if (castling & (1 << bq))
    castlingLtrs[3] = 'q';
  printf("          Castling: %s\n", castlingLtrs.c_str());
}

void Board::updateUnits() {
  units.fill(0);
  for (int piece = PAWN; piece <= KING; piece++) {
    units[WHITE] |= pieces[piece];
    units[BLACK] |= pieces[piece + 6];
  }
  units[BOTH] = units[WHITE] | units[BLACK];
}

int getPieceOnSquare(const Board &board, int sq) {
  for (int i = P; i <= k; i++) {
    if (getBit(board.pieces[i], sq))
      return i;
  }
  return E;
}

bool isSquareAttacked(const int side, const int sq, const Board &board) {
  // Attacked by white pawns
  if ((side == WHITE) && (Attack::pawnAttacks[BLACK][sq] & board.pieces[P]))
    return true;
  // Attacked by black pawns
  if ((side == BLACK) && (Attack::pawnAttacks[WHITE][sq] & board.pieces[p]))
    return true;
  // Attacked by knights
  if (Attack::knightAttacks[sq] & board.pieces[!side ? N : n])
    return true;
  // Attacked by bishops
  if (Magics::getBishopAttack(sq, board.units[BOTH]) &
      board.pieces[!side ? B : b])
    return true;
  // Attacked by rooks
  if (Magics::getRookAttack(sq, board.units[BOTH]) &
      board.pieces[!side ? R : r])
    return true;
  // Attacked by queens
  if (Magics::getQueenAttack(sq, board.units[BOTH]) &
      board.pieces[!side ? Q : q])
    return true;
  // Attacked by kings
  if (Attack::kingAttacks[sq] & board.pieces[!side ? K : k])
    return true;

  // If all of the above cases fail, return false
  return false;
}

void printAttacked(const int side, const Board &board) {
  printf("\n");
  for (int r = 0; r < 8; r++) {
    printf("  %d |", 8 - r);
    for (int f = 0; f < 8; f++)
      printf(" %d", isSquareAttacked(side, SQ(r, f), board) ? 1 : 0);

    printf("\n");
  }
  printf("      - - - - - - - -\n      a b c d e f g h\n");
}

/* ------ FEN -------*/
const std::array<std::string, 8> position = {
    "8/8/8/8/8/8/8/8 w - - 0 1",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
    "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1",
};

void parseFen(const std::string &fen, Board &board) {
  int currIndex = 0;

  // Parse the piece portion of the fen and place them on the board
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      if (fen[currIndex] == ' ')
        break;
      if (fen[currIndex] == '/')
        currIndex++;
      if (fen[currIndex] >= '0' && fen[currIndex] <= '8') {
        file += (fen[currIndex] - '0');
        currIndex++;
      }
      if ((fen[currIndex] >= 'A' && fen[currIndex] <= 'Z') ||
          (fen[currIndex] >= 'a' && fen[currIndex] <= 'z')) {
        int pieceInd;
        if ((pieceInd = pieceStr.find(fen[currIndex])) != E)
          setBit(board.pieces[pieceInd], SQ(rank, file));
        currIndex++;
      }
    }
  }
  currIndex++;
  // Parse side to move
  if (fen[currIndex] == 'w')
    board.side = 0;
  else if (fen[currIndex] == 'b')
    board.side = 1;
  currIndex += 2;

  // Parse castling rights
  while (fen[currIndex] != ' ') {
    switch (fen[currIndex]) {
    case 'K':
      setBit(board.castling, wk);
      break;
    case 'Q':
      setBit(board.castling, wq);
      break;
    case 'k':
      setBit(board.castling, bk);
      break;
    case 'q':
      setBit(board.castling, bq);
      break;
    }
    currIndex++;
  }
  currIndex++;

  // Parse enpassant square
  if (fen[currIndex] != '-') {
    int f = fen[currIndex] - 'a';
    currIndex++;
    int r = 8 - (fen[currIndex] - '0');
    board.enpassant = SQ(r, f);
    currIndex++;
  } else {
    board.enpassant = noSq;
    currIndex++;
  }
  currIndex++;

  size_t count;
  // Parse the number of half moves
  std::string spaceInd = fen.substr(currIndex);
  count = spaceInd.find(" ");
  board.halfMoves = atoi(fen.substr(currIndex, count - 1).c_str());
  currIndex += (int)count;

  // Parse the number of full moves
  spaceInd = fen.substr(currIndex);
  count = spaceInd.find(" ");
  board.fullMoves = atoi(fen.substr(currIndex, count - 1).c_str());

  // Update occupancy bitboards
  board.updateUnits();

  board.hashKey = Zobrist::genKey(board);
  board.hashLock = Zobrist::genLock(board);
}
