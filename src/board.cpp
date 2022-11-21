#include "board.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "attack.hpp"
#include "magics.hpp"

char pieceStr[14] = "PNBRQKpnbrqk ";

const char *strCoords[65] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "a7", "b7", "c7",
    "d7", "e7", "f7", "g7", "h7", "a6", "b6", "c6", "d6", "e6", "f6",
    "g6", "h6", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "a4",
    "b4", "c4", "d4", "e4", "f4", "g4", "h4", "a3", "b3", "c3", "d3",
    "e3", "f3", "g3", "h3", "a2", "b2", "c2", "d2", "e2", "f2", "g2",
    "h2", "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", " "};

const int castlingRights[] = {
    7,  15, 15, 15, 3,  15, 15, 11, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 13, 15, 15, 15, 12, 15, 15, 14};

Board::Board() {
  memset(pieces, 0, sizeof(pieces));
  memset(units, 0, sizeof(units));
  side = 0;
  enpassant = noSq;
  castling = 0;
  fullMoves = 0;
  halfMoves = 0;
}

void Board::display() {
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
         enpassant != -1 ? strCoords[enpassant] : "noSq");
  printf("        Full moves: %d\n\n", fullMoves);
}

void Board::printCastling() {
  char castlingLtrs[5] = "----";
  if (castling & (1 << wk))
    castlingLtrs[0] = 'K';
  if (castling & (1 << wq))
    castlingLtrs[1] = 'Q';
  if (castling & (1 << bk))
    castlingLtrs[2] = 'k';
  if (castling & (1 << bq))
    castlingLtrs[3] = 'q';
  printf("          Castling: %s\n", castlingLtrs);
}

void Board::updateUnits() {
  memset(units, 0, sizeof(units));
  for (int piece = PAWN; piece <= KING; piece++) {
    units[WHITE] |= pieces[piece];
    units[BLACK] |= pieces[piece + 6];
  }
  units[BOTH] = units[WHITE] | units[BLACK];
}

int getPieceOnSquare(Board &board, int sq) {
  assert(sq >= a8 && sq <= h1);
  for (int i = P; i <= k; i++) {
    if (getBit(board.pieces[i], sq))
      return i;
  }
  return E;
}

bool isSquareAttacked(int side, int sq, Board &board) {
  // Ensure correct value of parameters
  assert(side == WHITE || side == BLACK);
  assert(sq >= a8 && sq <= h1);
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

void printAttacked(int side, Board &board) {
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
const char *position[8] = {
    "8/8/8/8/8/8/8/8 w - - 0 1",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
    "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1",
};

void parseFen(const std::string fenStr, Board &board) {
  assert(!fenStr.empty());
  char *fen = (char *)fenStr.c_str();

  // Parse the piece portion of the fen and place them on the board
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      if (*fen == ' ')
        break;
      if (*fen == '/')
        fen++;
      if (*fen >= '0' && *fen <= '8') {
        file += (*fen - '0');
        fen++;
      }
      if ((*fen >= 'A' && *fen <= 'Z') || (*fen >= 'a' && *fen <= 'z')) {
        char *foundPiece = strchr(pieceStr, *fen);
        if (foundPiece != NULL) {
          int index = foundPiece - pieceStr;
          setBit(board.pieces[index], SQ(rank, file));
        }
        fen++;
      }
    }
  }
  fen++;
  // Parse side to move
  if (*fen == 'w')
    board.side = 0;
  else if (*fen == 'b')
    board.side = 1;
  fen += 2;

  // Parse castling rights
  while (*fen != ' ') {
    switch (*fen) {
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
    fen++;
  }
  fen++;

  // Parse enpassant square
  if (*fen != '-') {
    int f = *fen - 'a';
    fen++;
    int r = 8 - (*fen - '0');
    board.enpassant = SQ(r, f);
    fen++;
  } else {
    board.enpassant = noSq;
    fen++;
  }
  fen++;

  // Parse the number of half moves
  int count;
  for (count = 0; *fen != ' '; fen++, count++)
    ;
  board.halfMoves = atoi(fen - count);
  // Parse the number of full moves
  for (count = 0; *fen != ' '; fen++, count++)
    ;
  board.fullMoves = atoi(fen - count);

  // Update occupancy bitboards
  board.updateUnits();
}

// TODO: not complete
std::string genFen(Board &board) {
  std::string fen = "";
  // Pieces
  int offset;
  for (int r = 0; r < 8; r++) {
    offset = 0;
    for (int f = 0; f < 8; f++) {
      int currPiece;
      if ((currPiece = getPieceOnSquare(board, SQ(r, f))) != E) {
        if (offset > 0) {
          // Store offset and reset it
          fen += std::to_string(offset);
          offset = 0;
        }
        fen += pieceStr[currPiece];
      } else
        offset++;
    }
    if (offset > 0)
      fen += std::to_string(offset);
    if (r != 7)
      fen += '/';
  }
  fen += " ";

  // Side to move
  if (board.side == WHITE)
      fen += 'w';
  else if (board.side == BLACK)
      fen += 'b';
  fen += " ";

  // Castling rights
  char castlingLtrs[5] = {'-', '-', '-', '-'};
  if (getBit(board.castling, wk))
      castlingLtrs[wk] = 'K';
  if (getBit(board.castling, wq))
      castlingLtrs[wq] = 'Q';
  if (getBit(board.castling, bk))
      castlingLtrs[bk] = 'k';
  if (getBit(board.castling, bq))
      castlingLtrs[bq] = 'q';

  fen += castlingLtrs;
  fen += " ";
  // Enpassant squares
  if (board.enpassant != -1)
      fen += strCoords[board.enpassant];
  else
      fen += '-';
  fen += " ";
  // Half moves
  fen += std::to_string(board.halfMoves) + " ";
  // Full moves
  fen += std::to_string(board.fullMoves) + " ";
  return fen;
}
