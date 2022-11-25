#include "tt.hpp"

#include "search.hpp"

namespace TT {
const int F_HASH_EXACT = 0;
const int F_HASH_ALPHA = 1;
const int F_HASH_BETA = 2;

const int NO_ENTRY = 100'000;

constexpr const int PER_MB = 1'000'000 / sizeof(TTEntry);
constexpr int hashSize = 50 * PER_MB;
std::array<TTEntry, hashSize> ttTable;

void init() { ttTable.fill(TTEntry()); }

int readEntry(const Board &board, const int alpha, const int beta,
              const int depth) {
  TTEntry entry = ttTable[(board.hashKey * board.hashLock) % hashSize];
  if (entry.hashKey == board.hashKey && entry.hashLock == board.hashLock) {
      // Extract score from hash entry
      // Or extract mate distance from actual position
      if (entry.score < -MATE_SCORE) entry.score += Search::ply;
      if (entry.score > MATE_SCORE) entry.score -= Search::ply;

      // Check if depth is the same
      if (entry.depth >= depth)
      {
          // Match EXACT (PV node) score
          if (entry.flag == F_EXACT)
              return entry.score;
          // Match ALPHA (fail-low node) score
          if ((entry.flag == F_ALPHA) && (entry.score <= alpha))
              return alpha;
          // Match BETA (fail-high node) score
          if ((entry.flag == F_BETA) && (entry.score >= beta))
              return beta;
      }
  }
  return NO_ENTRY;
}

void writeEntry(const Board &board, const int depth, int score,
                const int flag) {
  U64 index = (board.hashKey * board.hashLock) % hashSize;

  // Store mate score independent from the actual path
  if (score < -MATE_SCORE)
    score -= Search::ply;
  if (score > MATE_SCORE)
    score += Search::ply;

  // Write data into TTEntry
  ttTable[index].hashKey = board.hashKey;
  ttTable[index].hashLock = board.hashLock;
  ttTable[index].score = score;
  ttTable[index].depth = depth;
  ttTable[index].flag = flag;
}
} // namespace TT