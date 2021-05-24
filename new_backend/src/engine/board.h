#pragma once
#include "constants.h"
#include <string>
#include "move.h"

using namespace std;


struct Evaluation {
    int evaluation;
    string moves[];
};

struct StoredBoard {
    int pieces[12][1];
    bool castleWhiteKingSide, castleWhiteQueenSide, castleBlackKingSide, castleBlackQueenSide, activeSide;
    BB friendlies, enemies, epSquare;
    int fullMoves, halfMoves;
};

class board
{
private:
    int pieces[12][1];
    /** current_opening_table **/
    bool castleWhiteKingSide, castleWhiteQueenSide, castleBlackKingSide, castleBlackQueenSide, activeSide, openingFinished;
    u64 friendlieBB, enemiesBB, epSquareBB, hashValue;
    int fullMoves, halfMoves, openingMoves;

public:
    board(string fen = "");
    ~board();
    u64 getHash();
    void resetBoard();
    void printBitboard(BB bb);
    Evaluation evaluateMoves(int depth, string lastMove);
    FenString toFenString();
    bool getActiveSide();
    void printEveryPiece();
    BB whitePiecesBB();
    BB blackPiecesBB();
    BB allPiecesBB();
    int* getActivePieces(bool activeSide);
    BB getPieceOnSquare(BB bb);
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