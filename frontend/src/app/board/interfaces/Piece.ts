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

export type PromotionTypes =
| 'br'
| 'bn'
| 'bb'
| 'bq'
| 'wr'
| 'wn'
| 'wb'
| 'wq';

export type Piece = { type: PieceTypes, possibleTargetSquares: number[], promotion: boolean};

export type Board = (Piece|undefined)[];

export type Position = {x: number, y: number}

export type Move = [oldPosition: Position, newPosition: Position, promotion: string];

export type BoardInitializedFENStringEvent = Observable<string>
