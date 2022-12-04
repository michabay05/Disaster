#pragma once

#include "defs.hpp"

namespace Eval
{
/*
    MG -> Middlegame
    EG -> Endgame
*/
enum class Phase : uint8_t { MG, EG };

extern const std::array<std::array<int16_t, 6>, 2> PIECE_VALUES;
extern const std::array<std::array<int8_t, 5>, 2> PIECE_MOBILITY;

extern const int8_t DRAW_SCORE;

extern const int16_t PAWN_PHASE;
extern const int16_t KNIGHT_PHASE;
extern const int16_t BISHOP_PHASE;
extern const int16_t ROOK_PHASE;
extern const int16_t QUEEN_PHASE;
extern const int16_t TOTAL_PHASE;

extern const std::array<int8_t, 6> PHASE_VALUES;

// Penalties
extern const std::array<int8_t, 2> ISOLATED_PAWN_PENALTY;
extern const std::array<int8_t, 2> DOUBLED_PAWN_PENALTY;

// Bonuses
extern const std::array<int8_t, 2> BISHOP_PAIR_BONUS;
extern const std::array<int8_t, 2> KNIGHT_OUTPOST_BONUS;
extern const std::array<int8_t, 2> BISHOP_OUTPOUT_BONUS;
extern const int8_t ROOK_OR_QUEEN_ON_SEVENTH_BONUS_EG;
extern const int8_t ROOK_ON_OPEN_FILE_BONUS_MG;
extern const int8_t TEMPO_BONUS_MG;

extern const std::array<std::array<int16_t, 64>, 6> PSQT_MG;
extern const std::array<std::array<int16_t, 64>, 6> PSQT_EG;
extern const std::array<std::array<int8_t, 64>, 2> PASSED_PAWN_PSQT;

} // namespace Eval
