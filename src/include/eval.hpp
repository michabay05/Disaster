#pragma once

#include "board.hpp"
#include "defs.hpp"

namespace Eval {
enum class Phase : uint8_t { OPENING, ENDGAME, MIDDLEGAME };

int EvalPosition(const Board& board);
void interpolateScores(const Phase phase, const int phaseScore, const int openingScore,
                       const int endgameScore, int& score);
int getPhaseScore(const Board& board);

} // namespace Eval
