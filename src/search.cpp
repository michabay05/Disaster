#include "search.hpp"

#include "bitboard.hpp"
#include "eval.hpp"
#include "misc.hpp"
#include "tt.hpp"
#include "uci.hpp"
#include "zobrist.hpp"

namespace Search
{
// clang-format off
// [attacker][victim]
const std::array<std::array<int, 6>, 6> mvvLva =
{
	{{105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600}}
};
// clang-format on
int ply;
// Total positions searched counter
int nodes;

// Quiet moves that caused a beta-cutoff
std::array<std::array<int, MAX_PLY>, 2> killerMoves; // [id][ply]
// Quiet moves that updated the alpha value
std::array<std::array<int, 64>, 12> historyMoves;      // [piece][square]
std::array<int, MAX_PLY> pvLength;                     // [ply]
std::array<std::array<int, MAX_PLY>, MAX_PLY> pvTable; // [ply][ply]

// PV flags
bool followPV, scorePV;

void clearSearchTable()
{
    for (auto& elem : killerMoves)
        elem.fill(0);
    for (auto& elem : historyMoves)
        elem.fill(0);
    for (auto& elem : pvTable)
        elem.fill(0);
    pvLength.fill(0);
}

void position(Board& board, const int depth)
{
    int score = 0;
    nodes = 0L;
    followPV = false;
    scorePV = false;
    clearSearchTable();
    UCI::stop = false;
    int alpha = -INF, beta = INF;
    int64_t totalTime = 1;
    for (int currDepth = 1; currDepth <= depth; currDepth++) {
        if (UCI::stop)
            break;
        // Enable followPV
        followPV = true;

        Time::start();

        score = negamax(&board, alpha, beta, currDepth);
        totalTime += Time::end();
        // Aspiration window
        if ((score <= alpha) || (score >= beta)) {
            alpha = -INF;
            beta = INF;
            continue;
        }
        // Set up the window for the next iteration
        alpha = score - 50;
        beta = score + 50;
        if (pvLength[0]) {
            std::cout << "info score ";
            getCPOrMateScore(score);
            std::cout << " depth " << currDepth << " nodes " << nodes << " time " << totalTime
                      << " nps " << (uint64_t)((nodes * 1000) / (float)totalTime) << " pv";
            for (int i = 0; i < pvLength[0]; i++)
                std::cout << " " << Move::toString(pvTable[0][i]);
            std::cout << "\n";
        }
    }
    std::cout << "bestmove " << Move::toString(pvTable[0][0]) << "\n";
}

void getCPOrMateScore(const int& score)
{
    // Print information about current depth
    if (score > -MATE_VALUE && score < -MATE_SCORE) {
        std::cout << "mate " << (-(score + MATE_VALUE) / 2 - 1);
    } else if (score > MATE_SCORE && score < MATE_VALUE) {
        std::cout << "mate " << ((MATE_VALUE - score) / 2 + 1);
    } else {
        std::cout << "cp " << score;
    }
}

int negamax(Board* board, int alpha, int beta, int depth)
{
    pvLength[ply] = ply;
    int score;
    TT::TTFlags hashFlag = TT::F_ALPHA;

    bool isPVNode = (beta - alpha) > 1;

    // Read score from transposition table if position already exists inside the
    // table
    if (ply > 0 && (score = TT::readEntry(*board, alpha, beta, depth)) != TT::NO_ENTRY && !isPVNode)
        return score;

    // every 2047 nodes
    if ((nodes & 2047) == 0)
        // "listen" to the GUI/user input
        UCI::checkUp();

    // Escape condition
    if (depth == 0)
        return quiescence(board, alpha, beta);

    // Exit if ply > max ply; ply should be <= 63
    if (ply > MAX_PLY - 1)
        return Eval::EvalPosition(*board);
    // Increment nodes
    nodes++;

    // Is the king in check?
    bool inCheck = board->isInCheck();

    // Check extension
    if (inCheck)
        depth++;

    int legalMoves = 0;

    // NULL move pruning
    if (depth >= 3 && !inCheck && ply) {
        Board anotherClone = *board;
        ply++;
        // Hash enpassant if available
        if (board->state.enpassant != Sq::noSq)
            Zobrist::toggleEnpass(*board, (int)board->state.enpassant);
        // Reset enpassant
        board->state.enpassant = Sq::noSq;

        // Give opponent an extra move; 2 moves in one turn
        board->state.changeSide();
        // Hash the extra turn given by hashing the side one more time
        Zobrist::toggleSide(*board);

        // Search move with reduced depth to find beta-cutoffs
        score = -negamax(board, -beta, -beta + 1, depth - 1 - 2);

        ply--;
        *board = anotherClone;
        if (UCI::stop)
            return 0;
        // Fail hard; beta-cutoffs
        if (score >= beta)
            return beta;
    }

    // Generate and sort moves
    Move::MoveList moveList;
    Move::generate(moveList, *board);
    if (followPV)
        enablePVScoring(moveList);
    sortMoves(moveList, *board);

    Board clone;
    int movesSearched = 0;
    // Loop over all the generated moves
    for (int i = 0; i < moveList.count; i++) {
        // Bookmark current state of board
        clone = *board;

        // Increment half move
        ply++;

        // Play move if move is legal
        if (!Move::make(board, moveList.list[i], Move::MoveType::allMoves)) {
            // Decrement move and move onto next move
            ply--;
            continue;
        }

        // Increment legal moves
        legalMoves++;

        // Full depth search
        if (movesSearched == 0)
            // Do normal alpha-beta search
            score = -negamax(board, -beta, -alpha, depth - 1);
        else // Late move reduction (LMR)
        {
            if (movesSearched >= FULL_DEPTH_MOVES && depth >= REDUCTION_LIMIT && !inCheck &&
                Move::getPromoted(moveList.list[i]) == (int)Piece::E &&
                !Move::isCapture(moveList.list[i]))
                score = -negamax(board, -alpha - 1, -alpha, depth - 2);
            else
                // Hack to ensure full depth search is done
                score = alpha + 1;

            // PVS (Principal Variation Search)
            if (score > alpha) {
                // re-search at full depth but with narrowed score bandwith
                score = -negamax(board, -alpha - 1, -alpha, depth - 1);

                // if LMR fails re-search at full depth and full score bandwith
                if ((score > alpha) && (score < beta))
                    score = -negamax(board, -beta, -alpha, depth - 1);
            }
        }
        // Decrement ply and restore board state
        ply--;
        *board = clone;

        if (UCI::stop)
            return 0;

        movesSearched++;

        // If current move is better, update move
        if (score > alpha) {
            // Switch flag to EXACT(PV node) from ALPHA (fail-low node)
            hashFlag = TT::F_EXACT;
            if (!Move::isCapture(moveList.list[i]))
                // Store history move
                historyMoves[Move::getPiece(moveList.list[i])][Move::getTarget(moveList.list[i])] +=
                    depth;

            // Principal Variation (PV) node
            alpha = score;
            // Write PV move
            pvTable[ply][ply] = moveList.list[i];
            // Copy move from deeper ply into current ply
            for (int j = ply + 1; j < pvLength[ply + 1]; j++)
                pvTable[ply][j] = pvTable[ply + 1][j];
            // Adjust pv length
            pvLength[ply] = pvLength[ply + 1];

            // Fail-hard beta cutoff
            if (score >= beta) {
                // Store hash entry with score equal to beta
                TT::writeEntry(*board, depth, beta, TT::F_BETA);

                if (!Move::isCapture(moveList.list[i])) {
                    // Move 1st killer move to 2nd killer move
                    killerMoves[1][ply] = killerMoves[0][ply];
                    // Update 1st killer move to current move
                    killerMoves[0][ply] = moveList.list[i];
                }
                // Move that fails high
                return beta;
            }
        }
    }
    // If no legal moves, it's either checkmate or stalemate
    if (legalMoves == 0) {
        // If check, return checkmate score
        if (inCheck)
            return -MATE_VALUE + ply;
        // If not check, return stalemate score
        else
            return 0;
    }
    // Store hash entry with score equal to alpha
    TT::writeEntry(*board, depth, alpha, hashFlag);
    // Move that failed low
    return alpha;
}

int quiescence(Board* board, int alpha, int beta)
{
    // every 2047 nodes
    if ((nodes & 2047) == 0)
        // "listen" to the GUI/user input
        UCI::checkUp();

    // Increment nodes
    nodes++;

    // Escape condition - fail-hard beta cutoff
    int positionEval = Eval::EvalPosition(*board);

    // Exit if ply > max ply; ply should be <= 63
    if (ply > MAX_PLY - 1)
        return positionEval;

    // Fail-hard beta cutoff
    if (positionEval >= beta)
        // Move that fails high
        return beta;

    // If current move is better, update move
    if (positionEval > alpha)
        // Principal Variation (PV) node
        alpha = positionEval;

    // Generate and sort moves
    Move::MoveList moveList;
    Move::generate(moveList, *board);
    sortMoves(moveList, *board);

    Board clone;
    // Loop over all the generated moves
    for (int i = 0; i < moveList.count; i++) {
        // Bookmark current state of board
        clone = *board;

        // Increment half move
        ply++;

        // Play move if move is legal
        if (!Move::make(board, moveList.list[i], Move::MoveType::onlyCaptures)) {
            // Decrement move and move onto next move
            ply--;
            continue;
        }

        // Score current move
        int score = -quiescence(board, -beta, -alpha);

        // Decrement ply and restore board state
        ply--;
        *board = clone;

        if (UCI::stop)
            return 0;

        // If current move is better, update move
        if (score > alpha) {
            alpha = score;

            // Fail-hard beta cutoff
            if (score >= beta)
                // Move that fails high
                return beta;
        }
        // Principal Variation (PV) node
    }
    // Move that failed low
    return alpha;
}

void printMoveScores(const Move::MoveList& moveList, const Board& board)
{
    std::cout << "Move scores: \n";
    for (int i = 0; i < moveList.count; i++)
        std::cout << "    " << Move::toString(moveList.list[i]).c_str() << ": "
                  << scoreMoves(board, moveList.list[i]) << "\n";
}

void sortMoves(Move::MoveList& moveList, const Board& board)
{
    std::array<int, 256> moveScores;
    // Initialize moveScores with move scores
    for (int i = 0; i < moveList.count; i++)
        moveScores[i] = scoreMoves(board, moveList.list[i]);

    // Sort moves based on scores
    for (int i = 0; i < moveList.count; i++) {
        for (int j = i + 1; j < moveList.count; j++) {
            if (moveScores[i] < moveScores[j]) {
                // Swap moves
                int temp = moveList.list[i];
                moveList.list[i] = moveList.list[j];
                moveList.list[j] = temp;
                // Swap scores
                temp = moveScores[i];
                moveScores[i] = moveScores[j];
                moveScores[j] = temp;
            }
        }
    }
}

/*
        Move Scoring Order or Priority
          1. PV moves        ( = 20,000 pts)
          2. MVV LVA move    (>= 10,000 pts)
          3. 1st killer move ( =  9,000 pts)
          4. 2nd killer move ( =  8,000 pts)
          5. History move
          6. Unsorted move
*/
int scoreMoves(const Board& board, const int move)
{
    // PV (Principal variation move) scoring
    if (scorePV && pvTable[0][ply] == move) {
        scorePV = false;
        return 20'000;
    }
    // Capture move scoring
    if (Move::isCapture(move)) {
        int victimPiece = (int)Piece::P;
        for (int bbPiece = (board.state.side == Color::WHITE ? (int)Piece::p : (int)Piece::P);
             bbPiece <= (board.state.side == Color::WHITE ? (int)Piece::k : (int)Piece::K);
             bbPiece++) {
            if (getBit(board.pos.pieces[bbPiece], Move::getTarget(move))) {
                victimPiece = bbPiece;
                break;
            }
        }
        return mvvLva[Move::getPiece(move) % 6][victimPiece % 6] + 10'000;
    }
    // Quiet move scoring
    else {
        // Score 1st killer move
        if (killerMoves[0][ply] == move)
            return 9'000;
        // Score 2nd killer move
        else if (killerMoves[1][ply] == move)
            return 8'000;
        // Score history moves
        else
            return historyMoves[Move::getPiece(move)][Move::getTarget(move)];
    }
}

void enablePVScoring(Move::MoveList& moveList)
{
    followPV = false;
    for (int i = 0; i < moveList.count; i++) {
        if (pvTable[0][ply] == moveList.list[i]) {
            // Enable PV scoring and following
            scorePV = true;
            followPV = true;
        }
    }
}

} // namespace Search
