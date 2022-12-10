#include "attack.hpp"

#include "bitboard.hpp"
#include "magics.hpp"

namespace Attack
{

// Store piece attacks
std::array<std::array<uint64_t, 64>, 2> pawnAttacks;     // [color][square]
std::array<uint64_t, 64> knightAttacks;                  // [square]
std::array<uint64_t, 64> kingAttacks;                    // [square]
std::array<uint64_t, 64> bishopOccMasks;                 // [square]
std::array<std::array<uint64_t, 512>, 64> bishopAttacks; // [square][occupancy variations]
std::array<uint64_t, 64> rookOccMasks;                   // [square]
std::array<std::array<uint64_t, 4096>, 64> rookAttacks;  // [square][occupancy variations]

// clang-format off

// Total number of square a bishop can go to from a certain square
const std::array<int, 64> bishopRelevantBits = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6,
};

// Total number of square a rook can go to from a certain square
const std::array<int, 64> rookRelevantBits = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12,
};
// clang-format on

/* Initializes attack tables for all pieces */
void init()
{
    initLeapers();
    initSliding(PieceTypes::BISHOP);
    initSliding(PieceTypes::ROOK);
}

/* Initializes attack tables for leaper pieces
   King, Knight, Pawn
*/
void initLeapers()
{
    for (int sq = 0; sq < 64; sq++) {
        genPawnAttacks(Color::WHITE, sq);
        genPawnAttacks(Color::BLACK, sq);
        genKnightAttacks(sq);
        genKingAttacks(sq);
    }
}

/* Initializes attack tables for sliding pieces
   Queen, Bishop, Rook
*/
void initSliding(const PieceTypes piece)
{
    for (int sq = 0; sq < 64; sq++) {
        // Generate all possible variations which can obstruct the path of the
        // bishop or rook
        bishopOccMasks[sq] = genBishopOccupancy(sq);
        rookOccMasks[sq] = genRookOccupancy(sq);

        uint64_t currentMask =
            (piece == PieceTypes::BISHOP) ? bishopOccMasks[sq] : rookOccMasks[sq];
        int bitCount = Bitboard::countBits(currentMask);
        for (int count = 0; count < (1 << bitCount); count++) {
            // Generate a 'blocking' variation based on the current 'blocking' mask
            uint64_t occupancy = setOccupancy(count, bitCount, currentMask);
            int magicInd;
            // Generate a magic index that can be used to store the attack's sliding
            // pieces
            if (piece == PieceTypes::BISHOP) {
                magicInd = (int)((occupancy * Magics::bishopMagics[sq]) >> (64 - bitCount));
                bishopAttacks[sq][magicInd] = genBishopAttack(sq, occupancy);
            } else {
                magicInd = (int)((occupancy * Magics::rookMagics[sq]) >> (64 - bitCount));
                rookAttacks[sq][magicInd] = genRookAttack(sq, occupancy);
            }
        }
    }
}

void genPawnAttacks(const Color side, const int sq)
{
    /* Since the board is set up where a8 is 0 and h1 is 63,
       the white pieces attack towards 0 while the black pieces
       attack towards 63.
    */
    if (side == Color::WHITE) {
        if (ROW(sq) > 0 && COL(sq) > 0)
            setBit(pawnAttacks[(int)Color::WHITE][sq], sq + (int)Direction::SW);
        if (ROW(sq) > 0 && COL(sq) < 7)
            setBit(pawnAttacks[(int)Color::WHITE][sq], sq + (int)Direction::SE);
    } else {
        if (ROW(sq) < 7 && COL(sq) > 0)
            setBit(pawnAttacks[(int)Color::BLACK][sq], sq + (int)Direction::NW);
        if (ROW(sq) < 7 && COL(sq) < 7)
            setBit(pawnAttacks[(int)Color::BLACK][sq], sq + (int)Direction::NE);
    }
}

void genKnightAttacks(const int sq)
{
    /* Knight attacks are generated regardless of the
       side to move because knights can go in all directions.
       Both sides use this attack table for knights.
    */
    if (ROW(sq) <= 5 && COL(sq) >= 1)
        setBit(knightAttacks[sq], sq + (int)Direction::NW_N);

    if (ROW(sq) <= 6 && COL(sq) >= 2)
        setBit(knightAttacks[sq], sq + (int)Direction::NW_W);

    if (ROW(sq) <= 6 && COL(sq) <= 5)
        setBit(knightAttacks[sq], sq + (int)Direction::NE_E);

    if (ROW(sq) <= 5 && COL(sq) <= 6)
        setBit(knightAttacks[sq], sq + (int)Direction::NE_N);

    if (ROW(sq) >= 2 && COL(sq) <= 6)
        setBit(knightAttacks[sq], sq + (int)Direction::SE_S);

    if (ROW(sq) >= 1 && COL(sq) <= 5)
        setBit(knightAttacks[sq], sq + (int)Direction::SE_E);

    if (ROW(sq) >= 1 && COL(sq) >= 2)
        setBit(knightAttacks[sq], sq + (int)Direction::SW_W);

    if (ROW(sq) >= 2 && COL(sq) >= 1)
        setBit(knightAttacks[sq], sq + (int)Direction::SW_S);
}

void genKingAttacks(const int sq)
{
    /* king attacks are generated regardless of the
       side to move because kings can go in all directions.
       Both sides use this attack table for kings.
    */
    if (ROW(sq) > 0)
        setBit(kingAttacks[sq], sq + (int)Direction::SOUTH);
    if (ROW(sq) < 7)
        setBit(kingAttacks[sq], sq + (int)Direction::NORTH);
    if (COL(sq) > 0)
        setBit(kingAttacks[sq], sq + (int)Direction::WEST);
    if (COL(sq) < 7)
        setBit(kingAttacks[sq], sq + (int)Direction::EAST);
    if (ROW(sq) > 0 && COL(sq) > 0)
        setBit(kingAttacks[sq], sq + (int)Direction::SW);
    if (ROW(sq) > 0 && COL(sq) < 7)
        setBit(kingAttacks[sq], sq + (int)Direction::SE);
    if (ROW(sq) < 7 && COL(sq) > 0)
        setBit(kingAttacks[sq], sq + (int)Direction::NW);
    if (ROW(sq) < 7 && COL(sq) < 7)
        setBit(kingAttacks[sq], sq + (int)Direction::NE);
}

/* Generates all the maximum occupancy on a bishop's path on its given square */
uint64_t genBishopOccupancy(const int sq)
{
    uint64_t output = 0ULL;
    int r, f;
    int sr = ROW(sq), sf = COL(sq);

    // NE direction
    for (r = sr + 1, f = sf + 1; r < 7 && f < 7; r++, f++)
        setBit(output, SQ(r, f));
    // NW direction
    for (r = sr + 1, f = sf - 1; r < 7 && f > 0; r++, f--)
        setBit(output, SQ(r, f));
    // SE direction
    for (r = sr - 1, f = sf + 1; r > 0 && f < 7; r--, f++)
        setBit(output, SQ(r, f));
    // SW direction
    for (r = sr - 1, f = sf - 1; r > 0 && f > 0; r--, f--)
        setBit(output, SQ(r, f));

    return output;
}

/* Generates a bishop's attack given its sq and a 'blocking' pieces on its
   path */
uint64_t genBishopAttack(const int sq, uint64_t blockerBoard)
{
    uint64_t output = 0ULL;
    int r, f;
    int sr = ROW(sq), sf = COL(sq);

    // NE direction
    for (r = sr + 1, f = sf + 1; r <= 7 && f <= 7; r++, f++) {
        setBit(output, SQ(r, f));
        if (getBit(blockerBoard, SQ(r, f)))
            break;
    }
    // NW direction
    for (r = sr + 1, f = sf - 1; r <= 7 && f >= 0; r++, f--) {
        setBit(output, SQ(r, f));
        if (getBit(blockerBoard, SQ(r, f)))
            break;
    }
    // SE direction
    for (r = sr - 1, f = sf + 1; r >= 0 && f <= 7; r--, f++) {
        setBit(output, SQ(r, f));
        if (getBit(blockerBoard, SQ(r, f)))
            break;
    }
    // SW direction
    for (r = sr - 1, f = sf - 1; r >= 0 && f >= 0; r--, f--) {
        setBit(output, SQ(r, f));
        if (getBit(blockerBoard, SQ(r, f)))
            break;
    }

    return output;
}

/* Generates all the maximum occupancy on a rook's path on its given square */
uint64_t genRookOccupancy(const int sq)
{
    uint64_t output = 0ULL;
    int r, f;
    int sr = ROW(sq), sf = COL(sq);

    // N direction
    for (r = sr + 1; r < 7; r++)
        setBit(output, SQ(r, sf));
    // S direction
    for (r = sr - 1; r > 0; r--)
        setBit(output, SQ(r, sf));
    // E direction
    for (f = sf + 1; f < 7; f++)
        setBit(output, SQ(sr, f));
    // W direction
    for (f = sf - 1; f > 0; f--)
        setBit(output, SQ(sr, f));

    return output;
}

/* Generates a rook's attack given its sq and a 'blocking' pieces on its
   path */
uint64_t genRookAttack(const int sq, const uint64_t blockerBoard)
{
    uint64_t output = 0ULL;
    int r, f;
    int sr = ROW(sq), sf = COL(sq);

    // N direction
    for (r = sr + 1; r <= 7; r++) {
        setBit(output, SQ(r, sf));
        if (getBit(blockerBoard, SQ(r, sf)))
            break;
    }
    // S direction
    for (r = sr - 1; r >= 0; r--) {
        setBit(output, SQ(r, sf));
        if (getBit(blockerBoard, SQ(r, sf)))
            break;
    }
    // E direction
    for (f = sf + 1; f <= 7; f++) {
        setBit(output, SQ(sr, f));
        if (getBit(blockerBoard, SQ(sr, f)))
            break;
    }
    // W direction
    for (f = sf - 1; f >= 0; f--) {
        setBit(output, SQ(sr, f));
        if (getBit(blockerBoard, SQ(sr, f)))
            break;
    }

    return output;
}

/* Generates a variation of 'blocking' pieces given an index, relevant bits, and
   occupancy mask */
uint64_t setOccupancy(const int index, const int relevantBits, uint64_t occMask)
{
    uint64_t occupancy = 0ULL;
    for (int count = 0; count < relevantBits; count++) {
        int ls1bIndex = Bitboard::lsbIndex(occMask);
        popBit(occMask, ls1bIndex);
        if ((index & (1 << count)) > 0)
            setBit(occupancy, ls1bIndex);
    }
    return occupancy;
}
} // namespace Attack