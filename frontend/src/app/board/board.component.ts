import { Component, Input, OnInit } from '@angular/core';
import { Side } from './enums/Side';
import { Board, Piece, PieceTypes } from './interfaces/Piece';
import { ChessApiService } from './services/chess-api.service';

const START_POS_FEN: string =
  'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1';

@Component({
  selector: 'board',
  templateUrl: './board.component.html',
  styleUrls: ['./board.component.scss'],
})
export class BoardComponent implements OnInit {
  @Input() boardWidth = 800;

  pieces: Board = [];

  constructor(private chessApi: ChessApiService) {
    this.chessApi.createNewBoard().pipe().subscribe(
      (pieces: Board) => {
        this.pieces = pieces;
      }
    )
  }

  ngOnInit(): void {}
}
