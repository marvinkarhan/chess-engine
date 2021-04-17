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

export type Piece = { xPos: number; yPos: number; type: PieceTypes };
