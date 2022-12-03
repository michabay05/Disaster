#include "eval.hpp"

#include "bitboard.hpp"
#include "psqt.hpp"

namespace Eval {

const std::array<std::array<int, 12>, 2> materialScore = {{
    // opening material score
    {82, 337, 365, 477, 1025, 12000, -82, -337, -365, -477, -1025, -12000},

    // endgame material score
    {94, 281, 297, 512, 936, 12000, -94, -281, -297, -512, -936, -12000}
}};

const int OPENING_PHASE_SCORE = 6192;
const int ENDGAME_PHASE_SCORE = 518;

int EvalPosition(const Board& board) {
    using enum Piece;

    int phaseScore = getPhaseScore(board);
    Phase phase;
    if (phaseScore >= OPENING_PHASE_SCORE)
        phase = Phase::OPENING;
    else if (phaseScore <= ENDGAME_PHASE_SCORE)
        phase = Phase::ENDGAME;
    else
        phase = Phase::MIDDLEGAME;
    uint64_t bitboard;
    int sq;
    int openingScore = 0, endgameScore = 0;
    for (int piece = (int)P; piece <= (int)k; piece++) {
        bitboard = board.pos.pieces[piece];
        while (bitboard) {
            sq = Bitboard::getLs1bIndex(bitboard);
            // Material score
            openingScore += materialScore[(int)Phase::OPENING][piece];
            endgameScore += materialScore[(int)Phase::ENDGAME][piece];
            // Score positional piece scores
            switch (piece) {
                // evaluate white pieces
            case (int)P:
                openingScore += PSQT::PSQTEval(Phase::OPENING, PieceTypes::PAWN, sq);
                endgameScore += PSQT::PSQTEval(Phase::ENDGAME, PieceTypes::PAWN, sq);
                break;
            case (int)N:
                openingScore += PSQT::PSQTEval(Phase::OPENING, PieceTypes::KNIGHT, sq);
                endgameScore += PSQT::PSQTEval(Phase::ENDGAME, PieceTypes::KNIGHT, sq);
                break;
            case (int)B:
                openingScore += PSQT::PSQTEval(Phase::OPENING, PieceTypes::BISHOP, sq);
                endgameScore += PSQT::PSQTEval(Phase::ENDGAME, PieceTypes::BISHOP, sq);
                break;
            case (int)R:
                openingScore += PSQT::PSQTEval(Phase::OPENING, PieceTypes::ROOK, sq);
                endgameScore += PSQT::PSQTEval(Phase::ENDGAME, PieceTypes::ROOK, sq);
                break;
            case (int)Q:
                openingScore += PSQT::PSQTEval(Phase::OPENING, PieceTypes::QUEEN, sq);
                endgameScore += PSQT::PSQTEval(Phase::ENDGAME, PieceTypes::QUEEN, sq);
                break;
            case (int)K:
                openingScore += PSQT::PSQTEval(Phase::OPENING, PieceTypes::KING, sq);
                endgameScore += PSQT::PSQTEval(Phase::ENDGAME, PieceTypes::KING, sq);
                break;

                // evaluate black pieces
            case (int)p:
                openingScore -= PSQT::PSQTEval(Phase::OPENING, PieceTypes::PAWN, FLIP(sq));
                endgameScore -= PSQT::PSQTEval(Phase::ENDGAME, PieceTypes::PAWN, FLIP(sq));
                break;
            case (int)n:
                openingScore -= PSQT::PSQTEval(Phase::OPENING, PieceTypes::KNIGHT, FLIP(sq));
                endgameScore -= PSQT::PSQTEval(Phase::ENDGAME, PieceTypes::KNIGHT, FLIP(sq));
                break;
            case (int)b:
                openingScore -= PSQT::PSQTEval(Phase::OPENING, PieceTypes::BISHOP, FLIP(sq));
                endgameScore -= PSQT::PSQTEval(Phase::ENDGAME, PieceTypes::BISHOP, FLIP(sq));
                break;
            case (int)r:
                openingScore -= PSQT::PSQTEval(Phase::OPENING, PieceTypes::ROOK, FLIP(sq));
                endgameScore -= PSQT::PSQTEval(Phase::ENDGAME, PieceTypes::ROOK, FLIP(sq));
                break;
            case (int)q:
                openingScore -= PSQT::PSQTEval(Phase::OPENING, PieceTypes::QUEEN, FLIP(sq));
                endgameScore -= PSQT::PSQTEval(Phase::ENDGAME, PieceTypes::QUEEN, FLIP(sq));
                break;
            case (int)k:
                openingScore -= PSQT::PSQTEval(Phase::OPENING, PieceTypes::KING, FLIP(sq));
                endgameScore -= PSQT::PSQTEval(Phase::ENDGAME, PieceTypes::KING, FLIP(sq));
                break;
            }
            popBit(bitboard, sq);
        }
    }
    // Linearly interpolate the opening and endgame scores
    // and store it in the scores variable
    int score = 0;
    interpolateScores(phase, phaseScore, openingScore, endgameScore, score);

    return (board.state.side == Color::WHITE) ? score : -score;
}

void interpolateScores(const Phase phase, const int phaseScore, const int openingScore,
                      const int endgameScore, int& score) {
    // Score interpolation
    if (phase == Phase::MIDDLEGAME)
        score = (openingScore * phaseScore + endgameScore * (OPENING_PHASE_SCORE - phaseScore)) /
                OPENING_PHASE_SCORE;

    else if (phase == Phase::OPENING)
        score = openingScore;
    else if (phase == Phase::ENDGAME)
        score = endgameScore;
}

int getPhaseScore(const Board& board) {
    /*
        The game phase score of the game is derived from the pieces
        of both sides (not counting pawns and kings) that are still
        on the board. The full material starting position game phase
        score is:
        4 * knight material score in the opening +
        4 * bishop material score in the opening +
        4 * rook material score in the opening +
        2 * queen material score in the opening
    */
    int whitePieceScore = 0, blackPieceScore = 0;
    for (int piece = 1; piece <= 4; piece++) {
        whitePieceScore +=
            Bitboard::countBits(board.pos.pieces[piece]) * materialScore[(int)Phase::OPENING][piece];
        blackPieceScore += Bitboard::countBits(board.pos.pieces[piece + 6]) *
                           -materialScore[(int)Phase::OPENING][piece + 6];
    }
    return whitePieceScore + blackPieceScore;
}
} // namespace Eval