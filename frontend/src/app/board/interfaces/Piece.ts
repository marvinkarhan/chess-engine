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

export type Piece = { type: PieceTypes, possibleTargetSquares: number[]};

export type Board = (Piece|undefined)[];

export type Position = {x: number, y: number}

export type Move = [oldPosition: Position, newPosition: Position];

export type BoardInitializedFENStringEvent = Observable<string>
