#include "eval.hpp"

#include "attack.hpp"
#include "bitboard.hpp"
#include "eval_constants.hpp"

namespace Eval
{
std::array<uint64_t, 8> rankMask;
std::array<uint64_t, 8> fileMask;

std::array<uint64_t, 8> isolatedMask;
std::array<std::array<uint64_t, 64>, 2> passedMask;
std::array<std::array<uint64_t, 64>, 2> outpostMask;

void setMask(const int rank, const int file, uint64_t& mask)
{
    if (rank == -1 && file == -1)
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

uint64_t setMask(const int rank, const int file)
{
    if (rank == -1 && file == -1)
        return 0ULL;
    uint64_t mask = 0;

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            if (file != -1 && f == file)
                setBit(mask, SQ(r, f));
            else if (rank != -1 && r == rank)
                setBit(mask, SQ(r, f));
        }
    }
    return mask;
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
            // Passed white mask
            setMask(-1, f - 1, passedMask[(int)Color::WHITE][SQ(r, f)]);
            setMask(-1, f, passedMask[(int)Color::WHITE][SQ(r, f)]);
            setMask(-1, f + 1, passedMask[(int)Color::WHITE][SQ(r, f)]);
            for (int i = 7; i >= r; i--)
                passedMask[(int)Color::WHITE][SQ(r, f)] &= ~rankMask[i];

            // Passed black mask
            setMask(-1, f - 1, passedMask[(int)Color::BLACK][SQ(r, f)]);
            setMask(-1, f, passedMask[(int)Color::BLACK][SQ(r, f)]);
            setMask(-1, f + 1, passedMask[(int)Color::BLACK][SQ(r, f)]);
            for (int i = 0; i <= r; i++)
                passedMask[(int)Color::BLACK][SQ(r, f)] &= ~rankMask[i];
        }
    }
}

int EvalPosition(const Board& board)
{
    // Don't evaluate position if it's a draw
    if (isDrawn(board.pos))
        return DRAW_SCORE;

    EvalInfo evalInfo;
    int sq = 0;
    int16_t phase = board.evalState.phase;
    uint64_t bitboardCopy = 0;
    for (int i = (int)Piece::P; i < (int)Piece::k; i++) {
        bitboardCopy = board.pos.pieces[i];
        while (bitboardCopy) {
            sq = Bitboard::lsbIndex(bitboardCopy);
            switch (COLORLESS(i)) {
            case (int)PieceTypes::PAWN:
                evalPawn(board.pos, sq, board.state.side, evalInfo);
                break;
            case (int)PieceTypes::KNIGHT:
                evalKnight(board.pos, sq, board.state.side, evalInfo);
                break;
            case (int)PieceTypes::BISHOP:
                evalBishop(board.pos, sq, board.state.side, evalInfo);
                break;
            case (int)PieceTypes::ROOK:
                evalRook(board.pos, sq, board.state.side, evalInfo);
                break;
            case (int)PieceTypes::QUEEN:
                evalQueen(board.pos, sq, board.state.side, evalInfo);
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

    evalKing(board.pos, Bitboard::lsbIndex(board.pos.pieces[(int)Piece::K]), Color::WHITE, evalInfo);
    evalKing(board.pos, Bitboard::lsbIndex(board.pos.pieces[(int)Piece::k]), Color::BLACK, evalInfo);

    evalInfo.mgScores[(int)board.state.side] += TEMPO_BONUS_MG;

    int mgScore =
        evalInfo.mgScores[(int)board.state.side] - evalInfo.mgScores[(int)board.state.xside];
    int egScore =
        evalInfo.egScores[(int)board.state.side] - evalInfo.egScores[(int)board.state.xside];

    phase = ((phase * 256) + (TOTAL_PHASE / 2)) / TOTAL_PHASE;

    return (((mgScore * (256 - phase)) + (egScore * phase)) / 256);
}

void evalPawn(const Position& pos, const int sq, const Color clr, EvalInfo& eInfo)
{
    uint8_t sidePawn = clr == Color::WHITE ? (int)Piece::P : (int)Piece::p;
    uint8_t xsidePawn = clr == Color::WHITE ? (int)Piece::p : (int)Piece::P;
    // Isolated pawns penalty
    if ((isolatedMask[COL(sq)] & pos.pieces[sidePawn]) == 0) {
        eInfo.mgScores[(int)clr] -= ISOLATED_PAWN_PENALTY[(int)Phase::MG];
        eInfo.egScores[(int)clr] -= ISOLATED_PAWN_PENALTY[(int)Phase::EG];
    }
    // Doubled pawns penalty
    auto doubledPawnCount = (uint8_t)Bitboard::countBits(pos.pieces[sidePawn] & fileMask[COL(sq)]);
    if (doubledPawnCount > 1) {
        eInfo.mgScores[(int)clr] -= DOUBLED_PAWN_PENALTY[(int)Phase::MG];
        eInfo.egScores[(int)clr] -= DOUBLED_PAWN_PENALTY[(int)Phase::EG];
    }
    // Passed pawns bonus
    // Give passed pawn bonus for non doubled pawns
    if (((passedMask[(int)clr][sq] & pos.pieces[xsidePawn]) == 0) && (doubledPawnCount == 0)) {
        eInfo.mgScores[(int)clr] += PASSED_PAWN_PSQT[(int)Phase::MG][sq];
        eInfo.egScores[(int)clr] += PASSED_PAWN_PSQT[(int)Phase::EG][sq];
    }
}

void evalKnight(const Position& pos, const int sq, const Color clr, EvalInfo& eInfo) {
    // Outpost bonus
    // Mobility bonus
    uint64_t possibleKnightMoves = Attack::knightAttacks[sq] & ~pos.units[(int)clr];
}

void evalBishop(const Position& pos, const int sq, const Color clr, EvalInfo& eInfo) {}

void evalRook(const Position& pos, const int sq, const Color clr, EvalInfo& eInfo) {}

void evalQueen(const Position& pos, const int sq, const Color clr, EvalInfo& eInfo) {}

void evalKing(const Position& pos, const int sq, const Color clr, EvalInfo& eInfo) {}

bool isDrawn(const Position& pos)
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