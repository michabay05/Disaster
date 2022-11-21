#include "search.hpp"

#include "bitboard.hpp"
#include "eval.hpp"
#include "move.hpp"

namespace Search {
const int MAX_VALUE = 50'000;
// Upper and lower bound
const int MATE_VALUE = 49'000;
const int MATE_SCORE = 48'000;

const int MAX_PLY = 64;
const int FULL_DEPTH_MOVES = 4;
const int REDUCTION_LIMIT = 3;

int killerMoves[2][MAX_PLY];
int historyMoves[12][64]; // [piece][square]
int pvLength[MAX_PLY];
int pvTable[MAX_PLY][MAX_PLY];

// clang-format off
// [attacker][victim]
const int mvvLva[6][6] = {
    {105, 205, 305, 405, 505, 605},
    {104, 204, 304, 404, 504, 604},
    {103, 203, 303, 403, 503, 603},
    {102, 202, 302, 402, 502, 602},
    {101, 201, 301, 401, 501, 601},
    {100, 200, 300, 400, 500, 600},
};
// clang-format on

// Half move counter
int ply;
// Total positions searched counter
int nodes;

// --------------- TEMPORARY VARIABLES
int bestMove;
// --------------- TEMPORARY VARIABLES

void position(Board &board, int depth) {
  int score = negamax(&board, -MAX_VALUE, MAX_VALUE, depth);
  if (bestMove) {
    printf("info score cp %d depth %d nodes %ld\n", score, depth, nodes);
    printf("bestmove %s\n", Move::toString(bestMove).c_str());
  } else {
    printf("bestmove isn't found!\n");
  }
}

int negamax(Board *board, int alpha, int beta, int depth) {
  if (depth == 0)
    return quiescence(board, alpha, beta);
  // Increment nodes
  nodes++;

  // Is the king in check?
  bool inCheck = isSquareAttacked(
      board->side ^ 1,
      Bitboard::getLs1bIndex(board->pieces[(!board->side ? K : k)]), *board);
  // Check extension
  // if (inCheck) depth++;

  // Generate moves
  Move::MoveList moveList;
  Move::generate(moveList, *board);

  // --------------- TEMPORARY VARIABLES
  int bestSoFar = 0, oldAlpha = alpha;
  // --------------- TEMPORARY VARIABLES

  Board clone;
  int legalMoves = 0;
  // Loop over all the generated moves
  for (int i = 0; i < moveList.count; i++) {
    // Bookmark current state of board
    clone = *board;

    // Increment half move
    ply++;

    // Play move if move is legal
    if (!Move::make(board, moveList.list[i], Move::allMoves)) {
      // Decrement move and move onto next move
      ply--;
      continue;
    }

    // Increment legal moves
    legalMoves++;
    // Score current move
    int score = -negamax(board, -beta, -alpha, depth - 1);

    // Take move back and decrement ply
    *board = clone;
    ply--;

    // Fail-hard beta cutoff
    if (score >= beta)
      // Move that fails high
      return beta;

    // If current move is better, update move
    if (score > alpha) {
      // Principal Variation (PV) node
      alpha = score;
      // If root node
      if (ply == 0)
        // Associate best score with best move
        bestSoFar = moveList.list[i];
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

  if (oldAlpha != alpha)
    bestMove = bestSoFar;
  // Move that failed low
  return alpha;
}

int quiescence(Board *board, int alpha, int beta) {
  int positionEval = Evaluate::EvalPosition(*board);
  // Fail-hard beta cutoff
  if (positionEval >= beta)
    // Move that fails high
    return beta;

  // If current move is better, update move
  if (positionEval > alpha)
    // Principal Variation (PV) node
    alpha = positionEval;
  // If root node
  // Increment nodes
  nodes++;

  // Generate moves
  Move::MoveList moveList;
  Move::generate(moveList, *board);

  Board clone;
  // Loop over all the generated moves
  for (int i = 0; i < moveList.count; i++) {
    // Bookmark current state of board
    clone = *board;

    // Increment half move
    ply++;

    // Play move if move is legal
    if (!Move::make(board, moveList.list[i], Move::onlyCaptures)) {
      // Decrement move and move onto next move
      ply--;
      continue;
    }

    // Score current move
    int score = -quiescence(board, -beta, -alpha);

    // Take move back and decrement ply
    *board = clone;
    ply--;

    // Fail-hard beta cutoff
    if (score >= beta)
      // Move that fails high
      return beta;

    // If current move is better, update move
    if (score > alpha)
      // Principal Variation (PV) node
      alpha = score;
    // If root node
  }
  // Move that failed low
  return alpha;
}

int scoreMoves(Board &board, int move) {
  // Capture move scoring
  if (Move::isCapture(move)) {
    int victimPiece = P;
    for (int bbPiece = (!board.side ? p : P); bbPiece <= (!board.side ? k : K);
         bbPiece++) {
      if (getBit(board.pieces[bbPiece], Move::getTarget(move))) {
        victimPiece = bbPiece;
        break;
      }
    }
    printf("Move: %s\t%cx%c\t%d\n", Move::toString(move).c_str(), pieceStr[Move::getPiece(move)], pieceStr[victimPiece], mvvLva[Move::getPiece(move) % 6][victimPiece % 6]);
    return mvvLva[Move::getPiece(move) % 6][victimPiece % 6];
  }
  // Quiet move scoring
  else {
  }
}

} // namespace Search
