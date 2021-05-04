import { Injectable } from '@angular/core';
import { Side } from '../enums/Side';
import { Board, Piece, PieceTypes } from '../interfaces/Piece';

@Injectable({
  providedIn: 'root'
})
export class BoardService {

  fullMoves: number = 0;
  halfMoves: number = 0;
  sideToMove: Side = Side.white;
  pieces: Board = []
  constructor() { 
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
