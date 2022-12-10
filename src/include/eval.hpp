#pragma once

#include "board.hpp"
#include "defs.hpp"

namespace Eval
{

extern std::array<uint64_t, 8> rankMask;
extern std::array<uint64_t, 8> fileMask;

extern std::array<uint64_t, 8> isolatedMask;
extern std::array<std::array<uint64_t, 64>, 2> doubledMask;
extern std::array<std::array<uint64_t, 64>, 2> passedMask;
extern std::array<std::array<uint64_t, 64>, 2> outpostMask;

struct KingZone
{
    uint64_t outerRing;
    uint64_t innerRing;
};

struct EvalInfo
{
    std::array<int16_t, 2> mgScores{0, 0};
    std::array<int16_t, 2> egScores{0, 0};

    std::array<KingZone, 2> kingZones;
    std::array<uint16_t, 2> kingAttackers;
    std::array<uint16_t, 2> kingAttackPoints;
};

void setMask(const int rank, const int file, uint64_t &mask);
void initMasks();
void genKingZones(const int sq);
int EvalPosition(const Board &board);
void evalPawn(const Position &pos, const int sq, const Color pieceColor, EvalInfo &eInfo);
void evalKnight(const Position &pos, const int sq, const Color pieceColor, EvalInfo &eInfo);
void evalBishop(const Position &pos, const int sq, const Color pieceColor, EvalInfo &eInfo);
void evalRook(const Position &pos, const int sq, const Color pieceColor, EvalInfo &eInfo);
void evalQueen(const Position &pos, const int sq, const Color pieceColor, EvalInfo &eInfo);
void evalKing(const Position &pos, const int sq, const Color pieceColor, EvalInfo &eInfo);
bool isDrawn(const Position &pos);
int getPhaseScore(const Board &board);

} // namespace Eval
