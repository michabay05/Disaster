#include "move.hpp"

#include <iostream>

#include "attack.hpp"
#include "bitboard.hpp"
#include "magics.hpp"
#include "zobrist.hpp"

namespace Move
{

MoveList::MoveList() { list.fill(0); }

void MoveList::add(const int move)
{
    list[count] = move;
    count++;
}

// Returns index from move list, if move is found
int MoveList::search(const int source, const int target, const int promoted) const
{
    for (int i = 0; i < count; i++) {
        // Parse move info
        int listMoveSource = getSource(list[i]);
        int listMoveTarget = getTarget(list[i]);
        int listMovePromoted = getPromoted(list[i]);
        // Check if source and target match
        if (listMoveSource == source && listMoveTarget == target && listMovePromoted == promoted)
            // Return index of move from movelist, if true
            return list[i];
    }
    return 0;
}
void MoveList::printList() const
{
    printf("    Source   |   Target  |  Piece  |  Promoted  |  Capture  |  Two "
           "Square Push  |  Enpassant  |  Castling\n");
    printf("  "
           "---------------------------------------------------------------------"
           "--------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("       %s    |    %s     |    %c    |     %c      |     %d     |   "
               "      %d         |      %d      |     %d\n",
               strCoords[getSource(list[i])].c_str(), strCoords[getTarget(list[i])].c_str(),
               pieceStr[getPiece(list[i])], pieceStr[getPromoted(list[i])], isCapture(list[i]),
               isTwoSquarePush(list[i]), isEnpassant(list[i]), isCastling(list[i]));
    }
    std::cout << "\n    Total number of moves: %d\n" << count;
}

int encode(int source, int target, int piece, int promoted, bool isCapture, bool isTwoSquarePush,
           bool isEnpassant, bool isCastling)
{
    return source | (target << 6) | (piece << 12) | (promoted << 16) | (isCapture << 20) |
           (isTwoSquarePush << 21) | (isEnpassant << 22) | (isCastling << 23);
}

int getSource(const int move) { return move & 0x3F; }

int getTarget(const int move) { return (move & 0xFC0) >> 6; }

int getPiece(const int move) { return (move & 0xF000) >> 12; }

int getPromoted(const int move)
{
    int promoted = (move & 0xF0000) >> 16;
    return promoted ? promoted : (int)Piece::E;
}

bool isCapture(const int move) { return move & 0x100000; }

bool isTwoSquarePush(const int move) { return move & 0x200000; }

bool isEnpassant(const int move) { return move & 0x400000; }

bool isCastling(const int move) { return move & 0x800000; }

std::string toString(const int move)
{
    std::string moveStr = strCoords[getSource(move)];
    moveStr += strCoords[getTarget(move)];
    int piece;
    if ((piece = getPromoted(move)) != (int)Piece::E)
        moveStr += std::tolower(pieceStr[piece]);
    return moveStr;
}

int parse(const std::string& moveStr, const Board& board)
{
    int source = SQ((8 - (moveStr[1] - '0')), (moveStr[0] - 'a'));
    int target = SQ((8 - (moveStr[3] - '0')), (moveStr[2] - 'a'));
    int promoted = (int)Piece::E;
    if (moveStr.length() == 5) {
        promoted = (int)pieceStr.find(moveStr[4]);
        if (ROW(target) == 0)
            promoted %= 6;
    }
    MoveList mL;
    generate(mL, board);
    int searchedMove = mL.search(source, target, promoted);
    return searchedMove;
}

void generate(MoveList& moveList, const Board& board)
{
    generatePawns(moveList, board);
    generateKnights(moveList, board);
    generateBishops(moveList, board);
    generateRooks(moveList, board);
    generateQueens(moveList, board);
    generateKings(moveList, board);
}

void generatePawns(MoveList& moveList, const Board& board)
{
    uint64_t bitboardCopy, attackCopy;
    int promotionStart, direction, doublePushStart, piece;
    int source, target;
    // If side to move is white
    if (board.state.side == Color::WHITE) {
        piece = (int)Piece::P;
        promotionStart = (int)Sq::a7;
        direction = (int)Direction::SOUTH;
        doublePushStart = (int)Sq::a2;
    }
    // If side to move is black
    else {
        piece = (int)Piece::p;
        promotionStart = (int)Sq::a2;
        direction = (int)Direction::NORTH;
        doublePushStart = (int)Sq::a7;
    }

    bitboardCopy = board.pos.pieces[piece];

    while (bitboardCopy) {
        source = Bitboard::lsbIndex(bitboardCopy);
        target = source + direction;
        if ((board.state.side == Color::WHITE ? target >= (int)Sq::a8 : target <= (int)Sq::h1) &&
            !getBit(board.pos.units[(int)Color::BOTH], target)) {
            // Quiet moves
            // Promotion
            if ((source >= promotionStart) && (source <= promotionStart + 7)) {
                moveList.add(
                    encode(source, target, piece,
                           (board.state.side == Color::WHITE ? (int)Piece::Q : (int)Piece::q), 0, 0,
                           0, 0));
                moveList.add(
                    encode(source, target, piece,
                           (board.state.side == Color::WHITE ? (int)Piece::R : (int)Piece::r), 0, 0,
                           0, 0));
                moveList.add(
                    encode(source, target, piece,
                           (board.state.side == Color::WHITE ? (int)Piece::B : (int)Piece::b), 0, 0,
                           0, 0));
                moveList.add(
                    encode(source, target, piece,
                           (board.state.side == Color::WHITE ? (int)Piece::N : (int)Piece::n), 0, 0,
                           0, 0));
            } else {
                moveList.add(encode(source, target, piece, (int)Piece::E, 0, 0, 0, 0));
                if ((source >= doublePushStart && source <= doublePushStart + 7) &&
                    !getBit(board.pos.units[(int)Color::BOTH], target + direction))
                    moveList.add(
                        encode(source, target + direction, piece, (int)Piece::E, 0, 1, 0, 0));
            }
        }
        // Capture moves
        attackCopy = Attack::pawnAttacks[(int)board.state.side][source] &
                     board.pos.units[(int)board.state.side ^ 1];
        while (attackCopy) {
            target = Bitboard::lsbIndex(attackCopy);
            // Capture move
            if ((source >= promotionStart) && (source <= promotionStart + 7)) {
                moveList.add(
                    encode(source, target, piece,
                           (board.state.side == Color::WHITE ? (int)Piece::Q : (int)Piece::q), 1, 0,
                           0, 0));
                moveList.add(
                    encode(source, target, piece,
                           (board.state.side == Color::WHITE ? (int)Piece::R : (int)Piece::r), 1, 0,
                           0, 0));
                moveList.add(
                    encode(source, target, piece,
                           (board.state.side == Color::WHITE ? (int)Piece::B : (int)Piece::b), 1, 0,
                           0, 0));
                moveList.add(
                    encode(source, target, piece,
                           (board.state.side == Color::WHITE ? (int)Piece::N : (int)Piece::n), 1, 0,
                           0, 0));
            } else
                moveList.add(encode(source, target, piece, (int)Piece::E, 1, 0, 0, 0));
            // Remove 'source' bit
            popBit(attackCopy, target);
        }
        // Generate enpassant capture
        if (board.state.enpassant != Sq::noSq) {
            uint64_t enpassCapture = Attack::pawnAttacks[(int)board.state.side][source] &
                                     (1ULL << (int)board.state.enpassant);
            if (enpassCapture) {
                int enpassTarget = Bitboard::lsbIndex(enpassCapture);
                moveList.add(encode(source, enpassTarget, piece, (int)Piece::E, 1, 0, 1, 0));
            }
        }
        // Remove bits
        popBit(bitboardCopy, source);
    }
}

void generateKnights(MoveList& moveList, const Board& board)
{
    int source, target, piece = board.state.side == Color::WHITE ? (int)Piece::N : (int)Piece::n;
    uint64_t bitboardCopy = board.pos.pieces[piece], attackCopy;
    while (bitboardCopy) {
        source = Bitboard::lsbIndex(bitboardCopy);

        attackCopy = Attack::knightAttacks[source] &
                     (board.state.side == Color::WHITE ? ~board.pos.units[(int)Color::WHITE]
                                                       : ~board.pos.units[(int)Color::BLACK]);
        while (attackCopy) {
            target = Bitboard::lsbIndex(attackCopy);
            if (getBit(board.pos.units[board.state.side == Color::WHITE ? (int)Color::BLACK
                                                                        : (int)Color::WHITE],
                       target))
                moveList.add(encode(source, target, piece, (int)Piece::E, 1, 0, 0, 0));
            else
                moveList.add(encode(source, target, piece, (int)Piece::E, 0, 0, 0, 0));
            popBit(attackCopy, target);
        }
        popBit(bitboardCopy, source);
    }
}

void generateBishops(MoveList& moveList, const Board& board)
{
    int source, target, piece = board.state.side == Color::WHITE ? (int)Piece::B : (int)Piece::b;
    uint64_t bitboardCopy = board.pos.pieces[piece], attackCopy;
    while (bitboardCopy) {
        source = Bitboard::lsbIndex(bitboardCopy);

        attackCopy = Magics::getBishopAttack(source, board.pos.units[(int)Color::BOTH]) &
                     (board.state.side == Color::WHITE ? ~board.pos.units[(int)Color::WHITE]
                                                       : ~board.pos.units[(int)Color::BLACK]);
        while (attackCopy) {
            target = Bitboard::lsbIndex(attackCopy);
            if (getBit(board.pos.units[board.state.side == Color::WHITE ? (int)Color::BLACK
                                                                        : (int)Color::WHITE],
                       target))
                moveList.add(encode(source, target, piece, (int)Piece::E, 1, 0, 0, 0));
            else
                moveList.add(encode(source, target, piece, (int)Piece::E, 0, 0, 0, 0));
            popBit(attackCopy, target);
        }
        popBit(bitboardCopy, source);
    }
}

void generateRooks(MoveList& moveList, const Board& board)
{
    int source, target, piece = board.state.side == Color::WHITE ? (int)Piece::R : (int)Piece::r;
    uint64_t bitboardCopy = board.pos.pieces[piece], attackCopy;
    while (bitboardCopy) {
        source = Bitboard::lsbIndex(bitboardCopy);

        attackCopy = Magics::getRookAttack(source, board.pos.units[(int)Color::BOTH]) &
                     (board.state.side == Color::WHITE ? ~board.pos.units[(int)Color::WHITE]
                                                       : ~board.pos.units[(int)Color::BLACK]);
        while (attackCopy) {
            target = Bitboard::lsbIndex(attackCopy);
            if (getBit(board.pos.units[board.state.side == Color::WHITE ? (int)Color::BLACK
                                                                        : (int)Color::WHITE],
                       target))
                moveList.add(encode(source, target, piece, (int)Piece::E, 1, 0, 0, 0));
            else
                moveList.add(encode(source, target, piece, (int)Piece::E, 0, 0, 0, 0));
            popBit(attackCopy, target);
        }
        popBit(bitboardCopy, source);
    }
}

void generateQueens(MoveList& moveList, const Board& board)
{
    int source, target, piece = board.state.side == Color::WHITE ? (int)Piece::Q : (int)Piece::q;
    uint64_t bitboardCopy = board.pos.pieces[piece], attackCopy;
    while (bitboardCopy) {
        source = Bitboard::lsbIndex(bitboardCopy);

        attackCopy = Magics::getQueenAttack(source, board.pos.units[(int)Color::BOTH]) &
                     (board.state.side == Color::WHITE ? ~board.pos.units[(int)Color::WHITE]
                                                       : ~board.pos.units[(int)Color::BLACK]);
        while (attackCopy) {
            target = Bitboard::lsbIndex(attackCopy);
            if (getBit(board.pos.units[board.state.side == Color::WHITE ? (int)Color::BLACK
                                                                        : (int)Color::WHITE],
                       target))
                moveList.add(encode(source, target, piece, (int)Piece::E, 1, 0, 0, 0));
            else
                moveList.add(encode(source, target, piece, (int)Piece::E, 0, 0, 0, 0));
            popBit(attackCopy, target);
        }
        popBit(bitboardCopy, source);
    }
}

void generateKings(MoveList& moveList, const Board& board)
{
    /* NOTE: Checks aren't handled by the move generator,
                                      it's handled by the make move function.
    */
    int source, target, piece = board.state.side == Color::WHITE ? (int)Piece::K : (int)Piece::k;
    uint64_t bitboard = board.pos.pieces[piece], attack;
    while (bitboard != 0) {
        source = Bitboard::lsbIndex(bitboard);

        attack = Attack::kingAttacks[source] &
                 (board.state.side == Color::WHITE ? ~board.pos.units[(int)Color::WHITE]
                                                   : ~board.pos.units[(int)Color::BLACK]);
        while (attack != 0) {
            target = Bitboard::lsbIndex(attack);

            if (getBit((board.pos.units[board.state.side == Color::WHITE ? (int)Color::BLACK
                                                                         : (int)Color::WHITE]),
                       target))
                moveList.add(encode(source, target, piece, (int)Piece::E, 1, 0, 0, 0));
            else
                moveList.add(encode(source, target, piece, (int)Piece::E, 0, 0, 0, 0));

            // Remove target bit to move onto the next bit
            popBit(attack, target);
        }
        // Remove source bit to move onto the next bit
        popBit(bitboard, source);
    }
    // Generate castling moves
    if (board.state.side == Color::WHITE)
        genWhiteCastling(moveList, board);
    else
        genBlackCastling(moveList, board);
}

void genWhiteCastling(MoveList& moveList, const Board& board)
{
    if (board.state.side == Color::WHITE) // WHITE
    {
        // Kingside castling
        if (getBit(board.state.castling, (int)CastlingRights::wk)) {
            // Check if path is obstructed
            if (!getBit(board.pos.units[(int)Color::BOTH], (int)Sq::f1) &&
                !getBit(board.pos.units[(int)Color::BOTH], (int)Sq::g1)) {
                // Is e1 or f1 attacked by a black piece?
                if (!board.isSquareAttacked((int)Sq::e1) && !board.isSquareAttacked((int)Sq::f1))
                    moveList.add(
                        encode((int)Sq::e1, (int)Sq::g1, (int)Piece::K, (int)Piece::E, 0, 0, 0, 1));
            }
        }
        // Queenside castling
        if (getBit(board.state.castling, (int)CastlingRights::wq)) {
            // Check if path is obstructed
            if (!getBit(board.pos.units[(int)Color::BOTH], (int)Sq::b1) &&
                !getBit(board.pos.units[(int)Color::BOTH], (int)Sq::c1) &&
                !getBit(board.pos.units[(int)Color::BOTH], (int)Sq::d1)) {
                // Is d1 or e1 attacked by a black piece?
                if (!board.isSquareAttacked((int)Sq::d1) && !board.isSquareAttacked((int)Sq::e1))
                    moveList.add(
                        encode((int)Sq::e1, (int)Sq::c1, (int)Piece::K, (int)Piece::E, 0, 0, 0, 1));
            }
        }
    }
}
void genBlackCastling(MoveList& moveList, const Board& board)
{
    // Kingside castling
    if (getBit(board.state.castling, (int)CastlingRights::bk)) {
        // Check if path is obstructed
        if (!getBit(board.pos.units[(int)Color::BOTH], (int)Sq::f8) &&
            !getBit(board.pos.units[(int)Color::BOTH], (int)Sq::g8)) {
            // Is e8 or f8 attacked by a white piece?
            if (!board.isSquareAttacked((int)Sq::e8) && !board.isSquareAttacked((int)Sq::f8))
                moveList.add(
                    encode((int)Sq::e8, (int)Sq::g8, (int)Piece::k, (int)Piece::E, 0, 0, 0, 1));
        }
    }
    // Queenside castling
    if (getBit(board.state.castling, (int)CastlingRights::bq)) {
        // Check if path is obstructed
        if (!getBit(board.pos.units[(int)Color::BOTH], (int)Sq::b8) &&
            !getBit(board.pos.units[(int)Color::BOTH], (int)Sq::c8) &&
            !getBit(board.pos.units[(int)Color::BOTH], (int)Sq::d8)) {
            // Is d8 or e8 attacked by a white piece?
            if (!board.isSquareAttacked((int)Sq::d8) && !board.isSquareAttacked((int)Sq::e8))
                moveList.add(
                    encode((int)Sq::e8, (int)Sq::c8, (int)Piece::k, (int)Piece::E, 0, 0, 0, 1));
        }
    }
}

bool make(Board* main, const int move, MoveType moveFlag)
{
    if (moveFlag == MoveType::allMoves) {
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
        popBit(main->pos.pieces[piece], source);
        Zobrist::togglePiece(*main, piece, source);

        setBit(main->pos.pieces[piece], target);
        Zobrist::togglePiece(*main, piece, target);

        // If capture, remove piece of opponent bitboard
        if (capture) {
            for (int bbPiece = (main->state.side == Color::WHITE ? (int)Piece::p : (int)Piece::P);
                 bbPiece <= (main->state.side == Color::WHITE ? (int)Piece::k : (int)Piece::K);
                 bbPiece++) {
                if (getBit(main->pos.pieces[bbPiece], target)) {
                    popBit(main->pos.pieces[bbPiece], target);
                    Zobrist::togglePiece(*main, bbPiece, target);
                    break;
                }
            }
        }

        // Promotion move
        if (promoted != (int)Piece::E) {
            popBit(main->pos.pieces[piece], target);
            Zobrist::togglePiece(*main, piece, target);

            setBit(main->pos.pieces[promoted], target);
            Zobrist::togglePiece(*main, promoted, target);
        }

        // Enpassant capture
        if (enpassant) {
            // If white to move
            if (main->state.side == Color::WHITE) {
                popBit(main->pos.pieces[(int)Piece::p], target + (int)Direction::NORTH);
                Zobrist::togglePiece(*main, (int)Piece::p, target + (int)Direction::NORTH);
            } else {
                popBit(main->pos.pieces[(int)Piece::P], target + (int)Direction::SOUTH);
                Zobrist::togglePiece(*main, (int)Piece::P, target + (int)Direction::SOUTH);
            }
        }
        if (main->state.enpassant != Sq::noSq)
            Zobrist::toggleEnpass(*main, (int)main->state.enpassant);

        // Reset enpassant, regardless of an enpassant capture
        main->state.enpassant = Sq::noSq;

        // Two Square Push move
        if (twoSquarePush) {
            if (main->state.side == Color::WHITE)
                main->state.enpassant = (Sq)(target + (int)Direction::NORTH);
            else
                main->state.enpassant = (Sq)(target + (int)Direction::SOUTH);
            Zobrist::toggleEnpass(*main, (int)main->state.enpassant);
        }

        // Castling
        if (castling) {
            switch (target) {
            case (int)Sq::g1:
                popBit(main->pos.pieces[(int)Piece::R], (int)Sq::h1);
                Zobrist::togglePiece(*main, (int)Piece::R, (int)Sq::h1);

                setBit(main->pos.pieces[(int)Piece::R], (int)Sq::f1);
                Zobrist::togglePiece(*main, (int)Piece::R, (int)Sq::f1);
                break;
            case (int)Sq::c1:
                popBit(main->pos.pieces[(int)Piece::R], (int)Sq::a1);
                Zobrist::togglePiece(*main, (int)Piece::R, (int)Sq::a1);

                setBit(main->pos.pieces[(int)Piece::R], (int)Sq::d1);
                Zobrist::togglePiece(*main, (int)Piece::R, (int)Sq::d1);
                break;
            case (int)Sq::g8:
                popBit(main->pos.pieces[(int)Piece::r], (int)Sq::h8);
                Zobrist::togglePiece(*main, (int)Piece::r, (int)Sq::h8);

                setBit(main->pos.pieces[(int)Piece::r], (int)Sq::f8);
                Zobrist::togglePiece(*main, (int)Piece::r, (int)Sq::f8);
                break;
            case (int)Sq::c8:
                popBit(main->pos.pieces[(int)Piece::r], (int)Sq::a8);
                Zobrist::togglePiece(*main, (int)Piece::r, (int)Sq::a8);

                setBit(main->pos.pieces[(int)Piece::r], (int)Sq::d8);
                Zobrist::togglePiece(*main, (int)Piece::r, (int)Sq::d8);
                break;
            }
        }
        Zobrist::toggleCastling(*main, main->state.castling);

        // Update castling rights
        main->state.castling &= castlingRights[source];
        main->state.castling &= castlingRights[target];
        Zobrist::toggleCastling(*main, main->state.castling);

        // Update units (or occupancies)
        main->pos.updateUnits();

        // Change side
        main->state.changeSide();
        Zobrist::toggleSide(*main);

        // Check if king is in check
        if (main->isInCheck()) {
            // Restore board and return false
            *main = clone;
            return false;
        } else {
            main->state.fullMoves++;
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