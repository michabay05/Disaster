#include "attack.hpp"
#include "bitboard.hpp"
#include "board.hpp"
#include "eval.hpp"
#include "eval_constants.hpp"
#include "magics.hpp"
#include "misc.hpp"
#include "move.hpp"
#include "perft.hpp"
#include "search.hpp"
#include "tt.hpp"
#include "tt_eval.hpp"
#include "uci.hpp"
#include "zobrist.hpp"

#ifdef _DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

void test()
{
    Board b;
    b.display();
    std::cout << "mg w = " << b.evalState.mgScores[0] << "\n";
    std::cout << "eg b = " << b.evalState.mgScores[1] << "\n";
    std::cout << "mg w = " << b.evalState.egScores[0] << "\n";
    std::cout << "eg b = " << b.evalState.egScores[1] << "\n";
    std::cout << "phase = " << b.evalState.phase << "\n";
    std::cout << "Total phase = " << Eval::TOTAL_PHASE << '\n';
    Search::position(b, 5);
}

int main(int argc, char **argv)
{
    std::ios::sync_with_stdio(false);

    // Initializations
    Attack::init();
    Zobrist::init();
    TT::clearTTtable();
    TT::Eval::clearEvalTable();
    Eval::initMasks();

    if (DEBUG)
        test();
    else
        UCI::loop();

    return 0;
}