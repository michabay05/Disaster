#include "eval.hpp"

#include "attack.hpp"
#include "bitboard.hpp"
#include "eval_constants.hpp"
#include "magics.hpp"

namespace Eval
{
std::array<uint64_t, 8> rankMask;
std::array<uint64_t, 8> fileMask;

std::array<uint64_t, 8> isolatedMask;
std::array<std::array<uint64_t, 64>, 2> passedMask;
std::array<std::array<uint64_t, 64>, 2> outpostMask;
std::array<KingZone, 64> kingZoneMask;

void setMask(const int rank, const int file, uint64_t &mask)
{
    if ((rank < 0 && file < 0) || (rank > 7 && file > 7))
        return;

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            if (file != -1 && f == file)
                setBit(mask, SQ(r, f));
            else if (rank != -1 && r == rank)
                setBit(mask, SQ(r, f));
        }
    }
}

void genKingZones(const int sq)
{
    // King zones - inner ring
    uint64_t zone = 0ULL;

    if (COL(sq) > 1) {
        setBit(zone, sq - 2);
        if (ROW(sq) > 0) {
            setBit(zone, sq - 2 - 8);
        }
        if (ROW(sq) > 1) {
            setBit(zone, sq - 2 - 16);
        }
        if (ROW(sq) < 6) {
            setBit(zone, sq - 2 + 16);
        }
        if (ROW(sq) < 7) {
            setBit(zone, sq - 2 + 8);
        }
    }
    if (COL(sq) < 6) {
        setBit(zone, sq + 2);
        if (ROW(sq) > 0) {
            setBit(zone, sq + 2 - 8);
        }
        if (ROW(sq) > 1) {
            setBit(zone, sq + 2 - 16);
        }
        if (ROW(sq) < 6) {
            setBit(zone, sq + 2 + 16);
        }
        if (ROW(sq) < 7) {
            setBit(zone, sq + 2 + 8);
        }
    }
    if (ROW(sq) > 1) {
        setBit(zone, sq - 16);
    }
    if (ROW(sq) < 6) {
        setBit(zone, sq + 16);
    }
    if (COL(sq) > 0 && ROW(sq) > 1) {
        setBit(zone, sq - 16 - 1);
    }
    if (COL(sq) > 0 && ROW(sq) < 6) {
        setBit(zone, sq + 16 - 1);
    }
    if (COL(sq) < 7 && ROW(sq) < 6) {
        setBit(zone, sq + 16 + 1);
    }
    if (COL(sq) < 7 && ROW(sq) > 1) {
        setBit(zone, sq - 16 + 1);
    }
    Bitboard::printBits(zone);

    kingZoneMask[sq].innerRing = Attack::kingAttacks[sq];
    setBit(kingZoneMask[sq].innerRing, sq);
    kingZoneMask[sq].outerRing = zone;
}

void initMasks()
{
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            // Rank and file mask
            setMask(r, -1, rankMask[r]);
            setMask(-1, f, fileMask[f]);

            // Isolated mask
            setMask(-1, f - 1, isolatedMask[f]);
            setMask(-1, f + 1, isolatedMask[f]);
        }
    }

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            int sq = SQ(r, f);
            // White outpost mask
            setMask(-1, f - 1, outpostMask[(int)Color::WHITE][sq]);
            setMask(-1, f + 1, outpostMask[(int)Color::WHITE][sq]);
            // White passed pawn mask
            setMask(-1, f - 1, passedMask[(int)Color::WHITE][sq]);
            setMask(-1, f, passedMask[(int)Color::WHITE][sq]);
            setMask(-1, f + 1, passedMask[(int)Color::WHITE][sq]);
            for (int i = 7; i >= r; i--) {
                passedMask[(int)Color::WHITE][sq] &= ~rankMask[i];
                outpostMask[(int)Color::WHITE][sq] &= ~rankMask[i];
            }

            // Black outpost mask
            setMask(-1, f - 1, outpostMask[(int)Color::BLACK][sq]);
            setMask(-1, f + 1, outpostMask[(int)Color::BLACK][sq]);
            // Black passed pawn mask
            setMask(-1, f - 1, passedMask[(int)Color::BLACK][sq]);
            setMask(-1, f, passedMask[(int)Color::BLACK][sq]);
            setMask(-1, f + 1, passedMask[(int)Color::BLACK][sq]);
            for (int i = 0; i <= r; i++) {
                passedMask[(int)Color::BLACK][sq] &= ~rankMask[i];
                outpostMask[(int)Color::BLACK][sq] &= ~rankMask[i];
            }

            genKingZones(sq);
        }
    }
}

int EvalPosition(const Board &board)
{
    // Don't evaluate position if it's a draw
    if (isDrawn(board.pos))
        return DRAW_SCORE;

    EvalInfo evalInfo;
    int sq = 0;
    Color pieceColor;
    int16_t phase = board.evalState.phase;
    uint64_t bitboardCopy = 0;
    for (int i = (int)Piece::P; i <= (int)Piece::k; i++) {
        bitboardCopy = board.pos.pieces[i];
        while (bitboardCopy) {
            sq = Bitboard::lsbIndex(bitboardCopy);
            pieceColor = (board.pos.getPieceOnSquare(sq) < 6) ? Color::WHITE : Color::BLACK;
            switch (COLORLESS(i)) {
            case (int)PieceTypes::PAWN:
                evalPawn(board.pos, sq, pieceColor, evalInfo);
                break;
            case (int)PieceTypes::KNIGHT:
                evalKnight(board.pos, sq, pieceColor, evalInfo);
                break;
            case (int)PieceTypes::BISHOP:
                evalBishop(board.pos, sq, pieceColor, evalInfo);
                break;
            case (int)PieceTypes::ROOK:
                evalRook(board.pos, sq, pieceColor, evalInfo);
                break;
            case (int)PieceTypes::QUEEN:
                evalQueen(board.pos, sq, pieceColor, evalInfo);
                break;
            default:
                break;
            }
            popBit(bitboardCopy, sq);
        }
    }
    if (Bitboard::countBits(board.pos.pieces[(int)Piece::B]) >= 2) {
        evalInfo.mgScores[(int)Color::WHITE] += BISHOP_PAIR_BONUS[(int)Phase::MG];
        evalInfo.egScores[(int)Color::WHITE] += BISHOP_PAIR_BONUS[(int)Phase::EG];
    }
    if (Bitboard::countBits(board.pos.pieces[(int)Piece::b]) >= 2) {
        evalInfo.mgScores[(int)Color::WHITE] += BISHOP_PAIR_BONUS[(int)Phase::MG];
        evalInfo.egScores[(int)Color::BLACK] += BISHOP_PAIR_BONUS[(int)Phase::EG];
    }

    evalKing(board.pos, Bitboard::lsbIndex(board.pos.pieces[(int)Piece::K]), Color::WHITE,
             evalInfo);
    evalKing(board.pos, Bitboard::lsbIndex(board.pos.pieces[(int)Piece::k]), Color::BLACK,
             evalInfo);

    evalInfo.mgScores[(int)board.state.side] += TEMPO_BONUS_MG;

    int mgScore =
        evalInfo.mgScores[(int)board.state.side] - evalInfo.mgScores[(int)board.state.xside];
    int egScore =
        evalInfo.egScores[(int)board.state.side] - evalInfo.egScores[(int)board.state.xside];

    phase = ((phase * 256) + (TOTAL_PHASE / 2)) / TOTAL_PHASE;

    return (((mgScore * (256 - phase)) + (egScore * phase)) / 256);
}

void evalPawn(const Position &pos, const int sq, const Color pieceColor, EvalInfo &eInfo)
{
    uint8_t sidePawn = pieceColor == Color::WHITE ? (int)Piece::P : (int)Piece::p;
    uint8_t xsidePawn = pieceColor == Color::WHITE ? (int)Piece::p : (int)Piece::P;
    // Isolated pawns penalty
    if ((isolatedMask[COL(sq)] & pos.pieces[sidePawn]) == 0) {
        eInfo.mgScores[(int)pieceColor] -= ISOLATED_PAWN_PENALTY[(int)Phase::MG];
        eInfo.egScores[(int)pieceColor] -= ISOLATED_PAWN_PENALTY[(int)Phase::EG];
    }
    // Doubled pawns penalty
    auto doubledPawnCount = (uint8_t)Bitboard::countBits(pos.pieces[sidePawn] & fileMask[COL(sq)]);
    if (doubledPawnCount > 1) {
        eInfo.mgScores[(int)pieceColor] -= DOUBLED_PAWN_PENALTY[(int)Phase::MG];
        eInfo.egScores[(int)pieceColor] -= DOUBLED_PAWN_PENALTY[(int)Phase::EG];
    }
    // Passed pawns bonus
    // Give passed pawn bonus for non doubled pawns
    if (((passedMask[(int)pieceColor][sq] & pos.pieces[xsidePawn]) == 0) &&
        (doubledPawnCount == 0)) {
        eInfo.mgScores[(int)pieceColor] += PASSED_PAWN_PSQT[(int)Phase::MG][sq];
        eInfo.egScores[(int)pieceColor] += PASSED_PAWN_PSQT[(int)Phase::EG][sq];
    }
}

void evalKnight(const Position &pos, const int sq, const Color pieceColor, EvalInfo &eInfo)
{
    uint64_t sidePawn =
        pieceColor == Color::WHITE ? pos.pieces[(int)Piece::P] : pos.pieces[(int)Piece::p];
    uint64_t xsidePawn =
        pieceColor == Color::WHITE ? pos.pieces[(int)Piece::p] : pos.pieces[(int)Piece::P];
    auto xside = (Color)((int)pieceColor ^ 1);
    int rankFromSidePOV = pieceColor == Color::WHITE ? ROW(sq) : ROW(FLIP(sq));

    // Outpost bonus
    if (((outpostMask[(int)pieceColor][sq] & xsidePawn) == 0) &&
        ((Attack::pawnAttacks[(int)xside][sq] & sidePawn) != 0) && rankFromSidePOV < 4) {
        eInfo.mgScores[(int)pieceColor] += KNIGHT_OUTPOST_BONUS[(int)Phase::MG];
        eInfo.egScores[(int)pieceColor] += KNIGHT_OUTPOST_BONUS[(int)Phase::EG];
    }
    // Mobility bonus
    uint64_t allMoves = Attack::knightAttacks[sq] & ~pos.units[(int)pieceColor];
    uint64_t safeMoves = allMoves;
    int bitInd = 0;
    while (xsidePawn) {
        bitInd = Bitboard::lsbIndex(xsidePawn);
        safeMoves &= ~Attack::pawnAttacks[(int)xside][bitInd];
        popBit(xsidePawn, bitInd);
    }
    int mobility = Bitboard::countBits(safeMoves);
    eInfo.mgScores[(int)pieceColor] +=
        (mobility - 4) * PIECE_MOBILITY[(int)Phase::MG][(int)PieceTypes::KNIGHT];
    eInfo.egScores[(int)pieceColor] +=
        (mobility - 4) * PIECE_MOBILITY[(int)Phase::EG][(int)PieceTypes::KNIGHT];
}

void evalBishop(const Position &pos, const int sq, const Color pieceColor, EvalInfo &eInfo)
{
    uint64_t sidePawn =
        pieceColor == Color::WHITE ? pos.pieces[(int)Piece::P] : pos.pieces[(int)Piece::p];
    uint64_t xsidePawn =
        pieceColor == Color::WHITE ? pos.pieces[(int)Piece::p] : pos.pieces[(int)Piece::P];
    auto xside = (Color)((int)pieceColor ^ 1);
    int rankFromSidePOV = pieceColor == Color::WHITE ? ROW(sq) : ROW(FLIP(sq));

    // Outpost bonus
    if (((outpostMask[(int)pieceColor][sq] & xsidePawn) == 0) &&
        ((Attack::pawnAttacks[(int)xside][sq] & sidePawn) != 0) && rankFromSidePOV < 4) {
        eInfo.mgScores[(int)pieceColor] += BISHOP_OUTPOUT_BONUS[(int)Phase::MG];
        eInfo.egScores[(int)pieceColor] += BISHOP_OUTPOUT_BONUS[(int)Phase::EG];
    }
    // Mobility bonus
    uint64_t safeMoves =
        Magics::getBishopAttack(sq, pos.units[(int)Color::BOTH]) & ~pos.units[(int)pieceColor];

    int mobility = Bitboard::countBits(safeMoves);
    eInfo.mgScores[(int)pieceColor] +=
        (mobility - 7) * PIECE_MOBILITY[(int)Phase::MG][(int)PieceTypes::BISHOP];
    eInfo.egScores[(int)pieceColor] +=
        (mobility - 7) * PIECE_MOBILITY[(int)Phase::EG][(int)PieceTypes::BISHOP];
}

void evalRook(const Position &pos, const int sq, const Color pieceColor, EvalInfo &eInfo)
{
    Color xside = pieceColor == Color::WHITE ? Color::BLACK : Color::WHITE;
    int enemyKingSq = Bitboard::lsbIndex(
        pos.pieces[(pieceColor == Color::WHITE) ? (int)Piece::k : (int)Piece::K]);
    int rankFromSidePOV = pieceColor == Color::WHITE ? ROW(sq) : ROW(FLIP(sq));
    int enemyKingRankFromSidePOV =
        pieceColor == Color::WHITE ? ROW(enemyKingSq) : ROW(FLIP(enemyKingSq));
    // Bonus for if the rook is on the seventh rank and cuts off the king from the rest of the board
    if (rankFromSidePOV == 1 && enemyKingRankFromSidePOV <= 1)
        eInfo.egScores[(int)pieceColor] += ROOK_OR_QUEEN_ON_SEVENTH_BONUS_EG;

    // If the rook on an open file (If file not blocked by any pawn)
    if ((fileMask[COL(sq)] & (pos.pieces[(int)Piece::P] | pos.pieces[(int)Piece::p])) == 0)
        eInfo.mgScores[(int)pieceColor] += ROOK_ON_OPEN_FILE_BONUS_MG;

    // Mobility bonus
    uint64_t safeMoves =
        Magics::getRookAttack(sq, pos.units[(int)Color::BOTH]) & ~pos.units[(int)pieceColor];

    int mobility = Bitboard::countBits(safeMoves);
    eInfo.mgScores[(int)pieceColor] +=
        (mobility - 7) * PIECE_MOBILITY[(int)Phase::MG][(int)PieceTypes::ROOK];
    eInfo.egScores[(int)pieceColor] +=
        (mobility - 7) * PIECE_MOBILITY[(int)Phase::EG][(int)PieceTypes::ROOK];
}

void evalQueen(const Position &pos, const int sq, const Color pieceColor, EvalInfo &eInfo)
{
    Color xside = pieceColor == Color::WHITE ? Color::BLACK : Color::WHITE;
    int enemyKingSq = Bitboard::lsbIndex(
        pos.pieces[(pieceColor == Color::WHITE) ? (int)Piece::k : (int)Piece::K]);
    int rankFromSidePOV = pieceColor == Color::WHITE ? ROW(sq) : ROW(FLIP(sq));
    int enemyKingRankFromSidePOV =
        pieceColor == Color::WHITE ? ROW(enemyKingSq) : ROW(FLIP(enemyKingSq));
    // Bonus for if the rook is on the seventh rank and cuts off the king from the rest of the board
    if (rankFromSidePOV == 1 && enemyKingRankFromSidePOV <= 1)
        eInfo.egScores[(int)pieceColor] += ROOK_OR_QUEEN_ON_SEVENTH_BONUS_EG;

    // Mobility bonus
    uint64_t safeMoves =
        Magics::getQueenAttack(sq, pos.units[(int)Color::BOTH]) & ~pos.units[(int)pieceColor];

    int mobility = Bitboard::countBits(safeMoves);
    eInfo.mgScores[(int)pieceColor] +=
        (mobility - 14) * PIECE_MOBILITY[(int)Phase::MG][(int)PieceTypes::QUEEN];
    eInfo.egScores[(int)pieceColor] +=
        (mobility - 14) * PIECE_MOBILITY[(int)Phase::EG][(int)PieceTypes::QUEEN];
}

void evalKing(const Position &pos, const int sq, const Color pieceColor, EvalInfo &eInfo) {}

bool isDrawn(const Position &pos)
{
    std::array<std::array<uint8_t, 6>, 2> count;
    // White piece count excluding the king
    count[0][0] = (uint8_t)Bitboard::countBits(pos.pieces[(int)Piece::P]);
    count[0][1] = (uint8_t)Bitboard::countBits(pos.pieces[(int)Piece::N]);
    count[0][2] = (uint8_t)Bitboard::countBits(pos.pieces[(int)Piece::B]);
    count[0][3] = (uint8_t)Bitboard::countBits(pos.pieces[(int)Piece::R]);
    count[0][4] = (uint8_t)Bitboard::countBits(pos.pieces[(int)Piece::Q]);

    // Black piece count excluding the king
    count[1][0] = (uint8_t)Bitboard::countBits(pos.pieces[(int)Piece::p]);
    count[1][1] = (uint8_t)Bitboard::countBits(pos.pieces[(int)Piece::n]);
    count[1][2] = (uint8_t)Bitboard::countBits(pos.pieces[(int)Piece::b]);
    count[1][3] = (uint8_t)Bitboard::countBits(pos.pieces[(int)Piece::r]);
    count[1][4] = (uint8_t)Bitboard::countBits(pos.pieces[(int)Piece::q]);

    // Piece count for both colors
    uint8_t majorCount = (count[0][3] + count[0][4]) + (count[1][3] + count[1][4]);
    uint8_t minorCount = (count[0][1] + count[0][2]) + (count[1][1] + count[1][2]);
    uint8_t pawnCount = (count[0][0] + count[1][0]);

    if (pawnCount == 0 && majorCount == 0 && minorCount == 0) {
        // K vs K position
        return true;
    } else if (majorCount + pawnCount == 0) {
        // Minor piece endgame
        if (minorCount == 1)
            // KB vs K; KN vs K
            return true;
        else if (minorCount == 2 && count[0][1] == 1 && count[1][1] == 1)
            // KN vs KN
            return true;
        else if (minorCount == 2 && count[0][2] == 1 && count[1][2] == 1) {
            auto whiteBishopSquare = (uint8_t)Bitboard::lsbIndex(pos.pieces[(int)Piece::B]);
            auto blackBishopSquare = (uint8_t)Bitboard::lsbIndex(pos.pieces[(int)Piece::b]);
            return SQCLR(ROW(whiteBishopSquare), COL(whiteBishopSquare)) ==
                   SQCLR(ROW(blackBishopSquare), COL(blackBishopSquare));
        }
    }
    return false;
}
} // namespace Eval