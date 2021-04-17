import { Component, Input, OnInit } from '@angular/core';
import { Side } from './enums/Side';
import { Piece, PieceTypes } from './interfaces/Piece';

const START_POS_FEN: string =
  'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1';

@Component({
  selector: 'board',
  templateUrl: './board.component.html',
  styleUrls: ['./board.component.scss'],
})
export class BoardComponent implements OnInit {
  @Input() boardWidth = 800;
  @Input() set fenString(fenString: string) {
    if (fenString) {
      this.loadFENString(fenString);
    }
  }

  pieces: Piece[] = [];
  sideToMove: Side = Side.white;
  halfMoves = 0;
  fullMoves = 0;

  constructor() {
    this.loadFENString(START_POS_FEN);
  }

  ngOnInit(): void {}

  loadFENString(fenString: string) {
    console.log('load fen');
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
          j += +char;
        } else {
          let color =
            char.charCodeAt(0) < 'Z'.charCodeAt(0) ? Side.white : Side.black;
          this.pieces.push({
            type: (color + char.toLowerCase()) as PieceTypes,
            xPos: +i,
            yPos: +j,
          });
        }
      });
    });
  }
}
