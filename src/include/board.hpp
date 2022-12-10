#pragma once

#include "defs.hpp"
#include <array>
#include <string>

extern const std::string pieceStr;
extern const std::array<std::string, 65> strCoords;
extern const std::array<int, 64> castlingRights;

struct Position
{
    std::array<uint64_t, 12> pieces{};
    std::array<uint64_t, 3> units{};

    Position();
    int getPieceOnSquare(const int sq) const;
    void updateUnits();
};

struct State
{
    Color side = Color::WHITE;
    Color xside = Color::BLACK;
    Sq enpassant = Sq::noSq;
    int castling = 0;
    int fullMoves = 0;
    int halfMoves = 0;
    uint64_t posKey = 0ULL;
    uint64_t posLock = 0ULL;

    State() = default;
    inline void changeSide()
    {
        side = xside;
        xside = (Color)((int)side ^ 1);
    }
};

struct EvalState
{
    int16_t phase = 0;
    std::array<int16_t, 2> mgScores{0 ,0};
    std::array<int16_t, 2> egScores{0, 0};

    EvalState() = default;
	void addPieceScores(const Piece piece, const Sq target);
};

struct Board
{
    static const std::array<std::string, 8> position;
    Position pos;
    State state;
    EvalState evalState;

    Board();
    Board(const std::string& fen);
    void display() const;
    void printCastling() const;
    static void parseFen(const std::string& fenStr, Board& board);
    bool isSquareAttacked(const int sq) const;
    bool isInCheck() const;
};

enum class CastlingRights : uint8_t { wk, wq, bk, bq };
