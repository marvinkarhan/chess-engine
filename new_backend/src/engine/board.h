#pragma once
#include "constants.h"
#include <string>
#include <map>
#include "move.h"

using namespace std;

struct Evaluation
{
    int evaluation;
    string moves[];
};

struct StoredBoard
{
    std::map<Piece, BB> pieces;
    bool castleWhiteKingSide, castleWhiteQueenSide, castleBlackKingSide, castleBlackQueenSide, activeSide;
    BB friendlies, enemies, epSquare;
    int fullMoves, halfMoves;
};

class Board
{
private:
    std::map<Piece, BB> pieces;
    /** current_opening_table **/
    bool castleWhiteKingSide, castleWhiteQueenSide, castleBlackKingSide, castleBlackQueenSide, activeSide, openingFinished;
    u64 friendliesBB, enemiesBB, epSquareBB, hashValue;
    int fullMoves, halfMoves, openingMoves;

public:
    Board(FenString fen = START_POS_FEN);
    u64 getHash();
    void resetBoard();
    void printBitboard(BB bb);
    Evaluation evaluateMoves(int depth, string lastMove);
    FenString toFenString();
    void printEveryPiece();
    BB whitePiecesBB();
    BB blackPiecesBB();
    BB allPiecesBB();
    BB *getActivePieces(bool activeSide);
    Piece getPieceOnSquare(BB bb);
    void parseFenString(FenString fen);
    BB potentialAttackers(int square, bool activeSide, BB occupied, bool onlySliders = false, bool excludeSliders = false);
    BB atackers(int square, bool activeSide, BB occupied, bool onlySliders = false, bool excludeSliders = false);
    BB blockers(int square, bool activeSide, BB occupied);
    auto pseudoLegalMovesGenerator(bool activeSide, bool onlyEvasions = false);
    auto legalMovesGenerator(bool activeSide = 0);
    bool moveIsLegal(Move move, bool activeSide, u64 blockers, int kingSquare, u64 occupied);
    bool stalemate();
    bool checkmate();
    auto getMovesTree(int depth);
    u64 perft(int depth);
    StoredBoard store();
    void restore(StoredBoard board);
    void hash();
    bool makeMove(Move move);

private:
    Evaluation negaMax(int depth, int alpha, int beta);
    int evaluate();
};