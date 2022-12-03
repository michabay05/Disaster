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

void clearTTtable() { ttTable.fill(TTEntry()); }

int readEntry(const Board& board, const int alpha, const int beta, const int depth) {
    TTEntry entry = ttTable[(board.state.posKey * board.state.posLock) % hashSize];
    if (entry.hashKey == board.state.posKey && entry.hashLock == board.state.posLock) {
        // Extract score from hash entry
        // Or extract mate distance from actual position
        if (entry.score < -Search::MATE_SCORE)
            entry.score += Search::ply;
        if (entry.score > Search::MATE_SCORE)
            entry.score -= Search::ply;

        // Check if depth is the same
        if (entry.depth >= depth) {
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

void writeEntry(const Board& board, const int depth, int score, const int flag) {
    uint64_t index = (board.state.posKey * board.state.posLock) % hashSize;

    // Store mate score independent from the actual path
    if (score < -Search::MATE_SCORE)
        score -= Search::ply;
    if (score > Search::MATE_SCORE)
        score += Search::ply;

    // Write data into TTEntry
    ttTable[index].hashKey = board.state.posKey;
    ttTable[index].hashLock = board.state.posLock;
    ttTable[index].score = score;
    ttTable[index].depth = depth;
    ttTable[index].flag = flag;
}

} // namespace TT