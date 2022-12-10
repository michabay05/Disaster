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
    //const std::string fen = "4k3/2R5/4K3/8/8/8/8/8 w - - 0 1";
    //Board b(fen);
    Board b;
    b.display();
    for (int sq = 0; sq < 64; sq++) {
        std::cout << "sq = " << strCoords[sq] << "\n";
		Eval::genKingZones(sq);
        std::cout << "\n------------------------------------------------------\n";
    }
}

int main(int argc, char **argv)
{
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