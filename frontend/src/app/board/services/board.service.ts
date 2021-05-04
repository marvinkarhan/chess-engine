import { Injectable } from '@angular/core';
import { forkJoin, merge, Observable } from 'rxjs';
import { map } from 'rxjs/operators';
import { ALGEBRAIC_TO_INDEX } from '../constants/BoardConstants';
import { Side } from '../enums/Side';
import { Board, Piece, PieceTypes } from '../interfaces/Piece';
import { ChessApiService } from './chess-api.service';

@Injectable({
  providedIn: 'root'
})
export class BoardService {

  fullMoves: number = 0;
  halfMoves: number = 0;
  sideToMove: Side = Side.white;
  pieces: Board = []
  constructor(private chessApi: ChessApiService) { 
  }

  getNewBoard() : Observable<Observable<Board>> {
    return this.chessApi.createNewBoard().pipe(map((fen: string) =>{
      this.loadFENString(fen);
      return this.getCurrentBoard()
    }))
  }
  getCurrentBoard() : Observable<Board> {
    // Delete all the previous target squares
    this.pieces.forEach((piece: Piece|undefined) => {
        if(piece) {
          piece.possibleTargetSquares = []
        }
    })
    // Load the new target squares
    return this.chessApi.getBoardMoves().pipe<Board>(map((uciMoves: string[]) => {
      uciMoves.forEach((uciMove: string) => {
        let startSquare : string = uciMove[0] + uciMove[1];
        let targetSquare = uciMove[2] + uciMove[3];
        let startSquareIndex : number = ALGEBRAIC_TO_INDEX[startSquare];
        let targetSquareIndex : number = ALGEBRAIC_TO_INDEX[targetSquare];
        this.pieces[startSquareIndex]?.possibleTargetSquares.push(targetSquareIndex);
      })
      return this.pieces;
    }))
  }
  loadFENString(fenString: string) : Board {
    let [
      placement,
      sideToMove,
      castlingAbility,
      epSquare,
      halfMoves,
      fullMoves,
    ] = fenString.split(' ');
    this.fullMoves = +fullMoves;
    this.halfMoves = +halfMoves;
    this.sideToMove = sideToMove as Side;

    const rows = placement.split('/');
    rows.forEach((row, i) => {
      row.split('').forEach((char, j) => {
        // make use of ascii char order
        if (char.charCodeAt(0) < '9'.charCodeAt(0)) {
          const  skips = +char;
          for(let skip = 0; skip < skips; skip++) {
            this.pieces.push(undefined);
          }
        } else {
          let color =
            char.charCodeAt(0) < 'Z'.charCodeAt(0) ? Side.white : Side.black;
          this.pieces.push({
            type: (color + char.toLowerCase()) as PieceTypes,
            possibleTargetSquares: [],
          });
        }
      });
    });
    return this.pieces
  }


}
