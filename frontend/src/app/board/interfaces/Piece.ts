import { Observable } from "rxjs";

export type PieceTypes =
  | 'br'
  | 'bn'
  | 'bb'
  | 'bq'
  | 'bk'
  | 'bp'
  | 'wr'
  | 'wn'
  | 'wb'
  | 'wq'
  | 'wk'
  | 'wp';

export type Piece = { type: PieceTypes, possibleTargetSquares: string[]};

export type Board = (Piece|undefined)[];

export type Position = {x: number, y: number}

export type Moves = {[startSquare: string]: string[]}

export type BoardInitializedFENStringEvent = Observable<string>
