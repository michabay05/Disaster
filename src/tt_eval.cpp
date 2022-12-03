#include "tt_eval.hpp"

#include "bitboard.hpp"
#include "tt.hpp"
#include "zobrist.hpp"

namespace TT::Eval {

// Pawn Evaluation table
constexpr const int PER_MB = 1'000'000 / sizeof(EvalEntry);
constexpr const int pawnTableSize = 2 * PER_MB;
std::array<EvalEntry, pawnTableSize> pawnTable;

void clearEvalTable() { pawnTable.fill(EvalEntry()); }

int readEntry(const Board &board) { return TT::NO_ENTRY; }
void writeEntry(const Board &board, int eval) {}

} // namespace TT::Eval