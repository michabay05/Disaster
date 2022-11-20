#include "move.h"

#include <iostream>

#include "attack.h"
#include "bitboard.h"
#include "magics.h"

namespace Move {

void MoveList::add(int move) {
  list[count] = move;
  count++;
}

// clang-format off
	void MoveList::printList() {
		printf("    Source   |   Target  |  Piece  |  Promoted  |  Capture  |  Two Square Push  |  Enpassant  |  Castling\n");
		printf("  -----------------------------------------------------------------------------------------------------------\n");
		for (int i = 0; i < count; i++) {
			printf("       %s    |    %s     |    %c    |     %c      |     %d     |         %d         |      %d      |     %d\n",
				strCoords[getSource(list[i])],
				strCoords[getTarget(list[i])],
				pieceStr[getPiece(list[i])],
				pieceStr[getPromoted(list[i])],
				isCapture(list[i]),
				isTwoSquarePush(list[i]),
				isEnpassant(list[i]),
				isCastling(list[i])
			);
		}
		printf("\n    Total number of moves: %d\n", count);
	}
// clang-format on

int encode(int source, int target, int piece, int promoted, bool isCapture,
           bool isTwoSquarePush, bool isEnpassant, bool isCastling) {
  return source | (target << 6) | (piece << 12) | (promoted << 16) |
         (isCapture << 20) | (isTwoSquarePush << 21) | (isEnpassant << 22) |
         (isCastling << 23);
}

int getSource(const int move) { return move & 0x3F; }

int getTarget(const int move) { return (move & 0xFC0) >> 6; }

int getPiece(const int move) { return (move & 0xF000) >> 12; }

int getPromoted(const int move) {
  int promoted = (move & 0xF0000) >> 16;
  return promoted ? promoted : E;
}

bool isCapture(const int move) { return move & 0x100000; }

bool isTwoSquarePush(const int move) { return move & 0x200000; }

bool isEnpassant(const int move) { return move & 0x400000; }

bool isCastling(const int move) { return move & 0x800000; }

const std::string toString(const int move) {
  std::string moveStr = strCoords[getSource(move)];
  moveStr += strCoords[getTarget(move)];
  moveStr += pieceStr[getPromoted(move)];
  return moveStr;
}

void generate(MoveList &moveList, Board &board) {
  generatePawns(moveList, board);
  generateKnights(moveList, board);
  generateBishops(moveList, board);
  generateRooks(moveList, board);
  generateQueens(moveList, board);
  generateKings(moveList, board);
}

void generatePawns(MoveList &moveList, Board &board) {
  /*
          Differences:
          bitboards,
          promotionRank, homeRank
  */
  U64 bitboardCopy, attackCopy;
  int promotionStart, direction, doublePushStart, piece;
  int source, target;
  // If side to move is white
  if (!board.side) {
    piece = P;
    promotionStart = a7;
    direction = SOUTH;
    doublePushStart = a2;
  }
  // If side to move is black
  else {
    piece = p;
    promotionStart = a2;
    direction = NORTH;
    doublePushStart = a7;
  }

  bitboardCopy = board.pieces[piece];

  while (bitboardCopy) {
    source = Bitboard::getLs1bIndex(bitboardCopy);
    target = source + direction;
    if ((!board.side ? target >= a8 : target <= h1) &&
        !getBit(board.units[BOTH], target)) {
      // Quiet moves
      // Promotion
      if ((source >= promotionStart) && (source <= promotionStart + 7)) {
        moveList.add(encode(source, target, piece, (!board.side ? Q : q), 0, 0, 0, 0));
        moveList.add(encode(source, target, piece, (!board.side ? R : r), 0, 0, 0, 0));
        moveList.add(encode(source, target, piece, (!board.side ? B : b), 0, 0, 0, 0));
        moveList.add(encode(source, target, piece, (!board.side ? N : n), 0, 0, 0, 0));
      } else {
        moveList.add(encode(source, target, piece, E, 0, 0, 0, 0));
        if ((source >= doublePushStart && source <= doublePushStart + 7) &&
            !getBit(board.units[BOTH], target + direction))
          moveList.add(
              encode(source, target + direction, piece, E, 0, 1, 0, 0));
      }
    }
    // Capture moves
    attackCopy =
        Attack::pawnAttacks[board.side][source] & board.units[board.side ^ 1];
    while (attackCopy) {
      target = Bitboard::getLs1bIndex(attackCopy);
      // Capture move
      if ((source >= promotionStart) && (source <= promotionStart + 7)) {
        moveList.add(encode(source, target, piece, (!board.side ? Q : q), 1, 0, 0, 0));
        moveList.add(encode(source, target, piece, (!board.side ? R : r), 1, 0, 0, 0));
        moveList.add(encode(source, target, piece, (!board.side ? B : b), 1, 0, 0, 0));
        moveList.add(encode(source, target, piece, (!board.side ? N : n), 1, 0, 0, 0));
      } else
        moveList.add(encode(source, target, piece, E, 1, 0, 0, 0));
      // Remove 'source' bit
      popBit(attackCopy, target);
    }
    // Generate enpassant capture
    if (board.enpassant != noSq) {
      U64 enpassCapture =
          Attack::pawnAttacks[board.side][source] & (1ULL << board.enpassant);
      if (enpassCapture) {
        int enpassTarget = Bitboard::getLs1bIndex(enpassCapture);
        moveList.add(encode(source, enpassTarget, piece, E, 1, 0, 1, 0));
      }
    }
    // Remove bits
    popBit(bitboardCopy, source);
  }
}

void generateKnights(MoveList &moveList, Board &board) {
  int source, target, piece = !board.side ? N : n;
  U64 bitboardCopy = board.pieces[piece], attackCopy;
  while (bitboardCopy) {
    source = Bitboard::getLs1bIndex(bitboardCopy);

    attackCopy = Attack::knightAttacks[source] &
                 (!board.side ? ~board.units[WHITE] : ~board.units[BLACK]);
    while (attackCopy) {
      target = Bitboard::getLs1bIndex(attackCopy);
      if (getBit(board.units[!board.side ? BLACK : WHITE], target))
        moveList.add(encode(source, target, piece, E, 1, 0, 0, 0));
      else
        moveList.add(encode(source, target, piece, E, 0, 0, 0, 0));
      popBit(attackCopy, target);
    }
    popBit(bitboardCopy, source);
  }
}

void generateBishops(MoveList &moveList, Board &board) {
  int source, target, piece = !board.side ? B : b;
  U64 bitboardCopy = board.pieces[piece], attackCopy;
  while (bitboardCopy) {
    source = Bitboard::getLs1bIndex(bitboardCopy);

    attackCopy = Magics::getBishopAttack(source, board.units[BOTH]) &
                 (!board.side ? ~board.units[WHITE] : ~board.units[BLACK]);
    while (attackCopy) {
      target = Bitboard::getLs1bIndex(attackCopy);
      if (getBit(board.units[!board.side ? BLACK : WHITE], target))
        moveList.add(encode(source, target, piece, E, 1, 0, 0, 0));
      else
        moveList.add(encode(source, target, piece, E, 0, 0, 0, 0));
      popBit(attackCopy, target);
    }
    popBit(bitboardCopy, source);
  }
}

void generateRooks(MoveList &moveList, Board &board) {
  int source, target, piece = !board.side ? R : r;
  U64 bitboardCopy = board.pieces[piece], attackCopy;
  while (bitboardCopy) {
    source = Bitboard::getLs1bIndex(bitboardCopy);

    attackCopy = Magics::getRookAttack(source, board.units[BOTH]) &
                 (!board.side ? ~board.units[WHITE] : ~board.units[BLACK]);
    while (attackCopy) {
      target = Bitboard::getLs1bIndex(attackCopy);
      if (getBit(board.units[!board.side ? BLACK : WHITE], target))
        moveList.add(encode(source, target, piece, E, 1, 0, 0, 0));
      else
        moveList.add(encode(source, target, piece, E, 0, 0, 0, 0));
      popBit(attackCopy, target);
    }
    popBit(bitboardCopy, source);
  }
}

void generateQueens(MoveList &moveList, Board &board) {
  int source, target, piece = !board.side ? Q : q;
  U64 bitboardCopy = board.pieces[piece], attackCopy;
  while (bitboardCopy) {
    source = Bitboard::getLs1bIndex(bitboardCopy);

    attackCopy = Magics::getQueenAttack(source, board.units[BOTH]) &
                 (!board.side ? ~board.units[WHITE] : ~board.units[BLACK]);
    while (attackCopy) {
      target = Bitboard::getLs1bIndex(attackCopy);
      if (getBit(board.units[!board.side ? BLACK : WHITE], target))
        moveList.add(encode(source, target, piece, E, 1, 0, 0, 0));
      else
        moveList.add(encode(source, target, piece, E, 0, 0, 0, 0));
      popBit(attackCopy, target);
    }
    popBit(bitboardCopy, source);
  }
}

void generateKings(MoveList &moveList, Board &board) {
  /* NOTE: Checks aren't handled by the move generator,
                    it's handled by the make move function.
  */
  int source, target, piece = !board.side ? K : k;
  U64 bitboard = board.pieces[piece], attack;
  while (bitboard != 0) {
    source = Bitboard::getLs1bIndex(bitboard);

    attack = Attack::kingAttacks[source] &
             (!board.side ? ~board.units[WHITE] : ~board.units[BLACK]);
    while (attack != 0) {
      target = Bitboard::getLs1bIndex(attack);

      if (getBit((board.side == 0 ? board.units[BLACK] : board.units[WHITE]),
                 target))
        moveList.add(encode(source, target, piece, E, 1, 0, 0, 0));
      else
        moveList.add(encode(source, target, piece, E, 0, 0, 0, 0));

      // Remove target bit to move onto the next bit
      popBit(attack, target);
    }
    // Remove source bit to move onto the next bit
    popBit(bitboard, source);
  }
  // Generate castling moves
  genCastling(moveList, board);
}

void genCastling(MoveList &moveList, Board &board) {
  if (!board.side) // WHITE
  {
    // Kingside castling
    if (getBit(board.castling, wk)) {
      // Check if path is obstructed
      if (!getBit(board.units[BOTH], f1) && !getBit(board.units[BOTH], g1)) {
        // Is e1 or f1 attacked by a black piece?
        if (!isSquareAttacked(BLACK, e1, board) &&
            !isSquareAttacked(BLACK, f1, board))
          moveList.add(encode(e1, g1, K, E, 0, 0, 0, 1));
      }
    }
    // Queenside castling
    if (getBit(board.castling, wq)) {
      // Check if path is obstructed
      if (!getBit(board.units[BOTH], b1) && !getBit(board.units[BOTH], c1) &&
          !getBit(board.units[BOTH], d1)) {
        // Is d1 or e1 attacked by a black piece?
        if (!isSquareAttacked(BLACK, d1, board) &&
            !isSquareAttacked(BLACK, e1, board))
          moveList.add(encode(e1, c1, K, E, 0, 0, 0, 1));
      }
    }
  } else // BLACK
  {
    // Kingside castling
    if (getBit(board.castling, bk)) {
      // Check if path is obstructed
      if (!getBit(board.units[BOTH], f8) && !getBit(board.units[BOTH], g8)) {
        // Is e8 or f8 attacked by a white piece?
        if (!isSquareAttacked(WHITE, e8, board) &&
            !isSquareAttacked(WHITE, f8, board))
          moveList.add(encode(e8, g8, k, E, 0, 0, 0, 1));
      }
    }
    // Queenside castling
    if (getBit(board.castling, bq)) {
      // Check if path is obstructed
      if (!getBit(board.units[BOTH], b8) && !getBit(board.units[BOTH], c8) &&
          !getBit(board.units[BOTH], d8)) {
        // Is d8 or e8 attacked by a white piece?
        if (!isSquareAttacked(WHITE, d8, board) &&
            !isSquareAttacked(WHITE, e8, board))
          moveList.add(encode(e8, c8, k, E, 0, 0, 0, 1));
      }
    }
  }
}

bool make(Board *main, int move, MoveType moveFlag) {
  if (moveFlag == allMoves) {
    Board clone = *main;

    // Parse move information
    int source = getSource(move);
    int target = getTarget(move);
    int piece = getPiece(move);
    int promoted = getPromoted(move);
    bool capture = isCapture(move);
    bool twoSquarePush = isTwoSquarePush(move);
    bool enpassant = isEnpassant(move);
    bool castling = isCastling(move);

    // Remove piece from 'source' and place on 'target'
    popBit(main->pieces[piece], source);
    setBit(main->pieces[piece], target);

    // If capture, remove piece of opponent bitboard
    if (capture) {
      for (int bbPiece = (!main->side ? p : P); bbPiece <= (!main->side ? k : K);
           bbPiece++) {
        if (getBit(main->pieces[bbPiece], target)) {
          popBit(main->pieces[bbPiece], target);
          break;
        }
      }
    }

    // Promotion move
    if (promoted != E) {
      popBit(main->pieces[piece], target);
      setBit(main->pieces[promoted], target);
    }

    // Enpassant capture
    if (enpassant) {
      // If white to move
      if (!main->side) {
        popBit(main->pieces[p], target + 8);
      } else {
        popBit(main->pieces[P], target - 8);
      }
    }
    // Reset enpassant, regardless of an enpassant capture
    main->enpassant = noSq;

    // Two Square Push move
    if (twoSquarePush) {
      if (!main->side)
        main->enpassant = target + 8;
      else
        main->enpassant = target - 8;
    }

    // Castling
    if (castling) {
      switch (target) {
      case g1:
        popBit(main->pieces[R], h1);
        setBit(main->pieces[R], f1);
        break;
      case c1:
        popBit(main->pieces[R], a1);
        setBit(main->pieces[R], d1);
        break;
      case g8:
        popBit(main->pieces[r], h8);
        setBit(main->pieces[r], f8);
        break;
      case c8:
        popBit(main->pieces[r], a8);
        setBit(main->pieces[r], d8);
        break;
      }
    }

    // Update castling rights
    main->castling &= castlingRights[source];
    main->castling &= castlingRights[target];

    // Update units (or occupancies)
    main->updateUnits();

    // Change side
    main->side ^= 1;

    // Check if king is in check
    if (isSquareAttacked(
            main->side,
            (Bitboard::getLs1bIndex(main->pieces[!main->side ? k : K])), *main)) {
      // Restore board and return false
      *main = clone;
      return false;
    } else {
      main->fullMoves++;
      // Move is legal and was made, return true
      return true;
    }

  } else {
    // If capture, recall make() and make move
    if (isCapture(move))
      return make(main, move, MoveType::allMoves);
    // If not capture, don't make move
    return false;
  }
}

} // namespace Move