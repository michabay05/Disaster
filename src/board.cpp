#include "board.hpp"

#include <iostream>

#include "attack.hpp"
#include "bitboard.hpp"
#include "eval_constants.hpp"
#include "magics.hpp"
#include "zobrist.hpp"

const std::string pieceStr = "PNBRQKpnbrqk ";

const std::array<std::string, 65> strCoords = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "a7", "b7", "c7", "d7", "e7",
    "f7", "g7", "h7", "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "a5", "b5",
    "c5", "d5", "e5", "f5", "g5", "h5", "a4", "b4", "c4", "d4", "e4", "f4", "g4",
    "h4", "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "a2", "b2", "c2", "d2",
    "e2", "f2", "g2", "h2", "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", " "};

const std::array<int, 64> castlingRights = {
    7,  15, 15, 15, 3,  15, 15, 11, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 13, 15, 15, 15, 12, 15, 15, 14};

const std::array<std::string, 8> Board::position = {
    "8/8/8/8/8/8/8/8 w - - 0 1",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
    "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1",
};

Position::Position()
{
    pieces.fill(0);
    units.fill(0);
}

void Position::updateUnits()
{
    units.fill(0);
    for (int piece = (int)PieceTypes::PAWN; piece <= (int)PieceTypes::KING; piece++) {
        units[(int)Color::WHITE] |= pieces[piece];
        units[(int)Color::BLACK] |= pieces[piece + 6];
    }
    units[(int)Color::BOTH] = units[(int)Color::WHITE] | units[(int)Color::BLACK];
}

void EvalState::addPieceScores(const Piece piece, const Sq target)
{
    Color pieceClr = (int)piece < 6 ? Color::WHITE : Color::BLACK;
    Sq pieceTarget = pieceClr == Color::WHITE ? target : (Sq)FLIP(target);

    mgScores[(int)pieceClr] += Eval::PIECE_VALUES[(int)Eval::Phase::MG][COLORLESS(piece)] +
                               Eval::PSQT_MG[COLORLESS(piece)][(int)pieceTarget];
    egScores[(int)pieceClr] += Eval::PIECE_VALUES[(int)Eval::Phase::EG][COLORLESS(piece)] +
                               Eval::PSQT_EG[COLORLESS(piece)][(int)pieceTarget];
    phase -= Eval::PHASE_VALUES[COLORLESS(piece)];
}

Board::Board() { parseFen(position[1], *this); }

Board::Board(const std::string& fen) { parseFen(fen, *this); }

void Board::display() const
{
    std::cout << "\n    +---+---+---+---+---+---+---+---+\n";
    for (int r = 0; r < 8; r++) {
        std::cout << "  " << 8 - r << " |";
        for (int f = 0; f < 8; f++)
            std::cout << " " << pieceStr[pos.getPieceOnSquare(SQ(r, f))] << " |";
        std::cout << "\n    +---+---+---+---+---+---+---+---+\n";
    }
    std::cout << "      a   b   c   d   e   f   g   h\n\n";
    std::cout << "      Side to move: " << (state.side == Color::WHITE ? "white" : "black") << "\n";
    printCastling();
    std::cout << "         Enpassant: "
              << (state.enpassant != Sq::noSq ? strCoords[(int)state.enpassant] : "noSq") << "\n";
    std::cout << "        Full moves: " << state.fullMoves << "\n";
}

void Board::printCastling() const
{
    using enum CastlingRights;

    std::string castlingLtrs = "----";
    if (state.castling & (1 << (int)wk))
        castlingLtrs[0] = 'K';
    if (state.castling & (1 << (int)wq))
        castlingLtrs[1] = 'Q';
    if (state.castling & (1 << (int)bk))
        castlingLtrs[2] = 'k';
    if (state.castling & (1 << (int)bq))
        castlingLtrs[3] = 'q';
    std::cout << "        Full moves: " << castlingLtrs << "\n";
}

int Position::getPieceOnSquare(const int sq) const
{
    using enum Piece;
    for (int i = (int)P; i <= (int)k; i++) {
        if (getBit(pieces[i], sq))
            return i;
    }
    return (int)E;
}

bool Board::isSquareAttacked(const int sq) const
{
    // Attacked by white pawns
    if ((state.side == Color::WHITE) &&
        (Attack::pawnAttacks[(int)Color::BLACK][sq] & pos.pieces[(int)Piece::P]))
        return true;
    // Attacked by black pawns
    if ((state.side == Color::BLACK) &&
        (Attack::pawnAttacks[(int)Color::WHITE][sq] & pos.pieces[(int)Piece::p]))
        return true;
    // Attacked by knights
    if (Attack::knightAttacks[sq] &
        pos.pieces[state.side == Color::WHITE ? (int)Piece::N : (int)Piece::n])
        return true;
    // Attacked by bishops
    if (Magics::getBishopAttack(sq, pos.units[(int)Color::BOTH]) &
        pos.pieces[state.side == Color::WHITE ? (int)Piece::B : (int)Piece::b])
        return true;
    // Attacked by rooks
    if (Magics::getRookAttack(sq, pos.units[(int)Color::BOTH]) &
        pos.pieces[state.side == Color::WHITE ? (int)Piece::R : (int)Piece::r])
        return true;
    // Attacked by queens
    if (Magics::getQueenAttack(sq, pos.units[(int)Color::BOTH]) &
        pos.pieces[state.side == Color::WHITE ? (int)Piece::Q : (int)Piece::q])
        return true;
    // Attacked by kings
    if (Attack::kingAttacks[sq] &
        pos.pieces[state.side == Color::WHITE ? (int)Piece::K : (int)Piece::k])
        return true;

    // If all of the above cases fail, return false
    return false;
}

bool Board::isInCheck() const
{
    uint8_t piece = state.side == Color::WHITE ? (int)Piece::k : (int)Piece::K;
    return isSquareAttacked(Bitboard::lsbIndex(pos.pieces[piece]));
}

void Board::parseFen(const std::string& fen, Board& board)
{
    int currIndex = 0;

    // Parse the piece portion of the fen and place them on the board
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            if (fen[currIndex] == ' ')
                break;
            if (fen[currIndex] == '/')
                currIndex++;
            if (fen[currIndex] >= '0' && fen[currIndex] <= '8') {
                file += (fen[currIndex] - '0');
                currIndex++;
            }
            if ((fen[currIndex] >= 'A' && fen[currIndex] <= 'Z') ||
                (fen[currIndex] >= 'a' && fen[currIndex] <= 'z')) {
                size_t piece;
                if ((piece = pieceStr.find(fen[currIndex])) != (int)Piece::E) {
                    setBit(board.pos.pieces[piece], SQ(rank, file));
                    board.evalState.addPieceScores((Piece)piece, (Sq)SQ(rank, file));
                }
                currIndex++;
            }
        }
    }
    currIndex++;
    // Parse side to move
    if (fen[currIndex] == 'w') {
        board.state.side = Color::WHITE;
        board.state.xside = Color::BLACK;
    } else if (fen[currIndex] == 'b') {
        board.state.side = Color::BLACK;
        board.state.xside = Color::WHITE;
    }
    currIndex += 2;

    // Check if king is in check in the initial position
    // If the king is in check, the king has to have a chance to escape it.
    if (board.isInCheck()) {
        std::cout << "Failed to initialized board!\n";
        std::cout << "     [Problem] - Initial position can't have a king being in check and it's "
                     "not the checked side to move.\n";
        std::cout
            << "  [Solution 1] - Modify the position to remove the checked king from check.\n";
        std::cout << "  [Solution 2] - Change side to move to the side of the checked king.\n\n";
        std::exit(1);
    }

    // Parse castling rights
    while (fen[currIndex] != ' ') {
        switch (fen[currIndex]) {
        case 'K':
            setBit(board.state.castling, (int)CastlingRights::wk);
            break;
        case 'Q':
            setBit(board.state.castling, (int)CastlingRights::wq);
            break;
        case 'k':
            setBit(board.state.castling, (int)CastlingRights::bk);
            break;
        case 'q':
            setBit(board.state.castling, (int)CastlingRights::bq);
            break;
        }
        currIndex++;
    }
    currIndex++;

    // Parse enpassant square
    if (fen[currIndex] != '-') {
        int f = fen[currIndex] - 'a';
        currIndex++;
        int r = 8 - (fen[currIndex] - '0');
        board.state.enpassant = (Sq)SQ(r, f);
        currIndex++;
    } else {
        board.state.enpassant = Sq::noSq;
        currIndex++;
    }
    currIndex++;

    size_t count;
    // Parse the number of half moves
    std::string spaceInd = fen.substr(currIndex);
    count = spaceInd.find(" ");
    board.state.halfMoves = atoi(fen.substr(currIndex, count - 1).c_str());
    currIndex += (int)count;

    // Parse the number of full moves
    spaceInd = fen.substr(currIndex);
    count = spaceInd.find(" ");
    board.state.fullMoves = atoi(fen.substr(currIndex, count - 1).c_str());

    // Update occupancy bitboards
    board.pos.updateUnits();

    board.state.posKey = Zobrist::genKey(board);
    board.state.posLock = Zobrist::genLock(board);
}
