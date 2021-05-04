import { Component, Input, OnInit } from '@angular/core';
import { Observable } from 'rxjs';
import { Side } from './enums/Side';
import { Board, Piece, PieceTypes } from './interfaces/Piece';
import { BoardService } from './services/board.service';
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
  potentialMoves: number[] = [];

  constructor(private boardService: BoardService) {
    this.boardService.getNewBoard().subscribe(
      (obs: Observable<Board>) => obs.subscribe((board: Board) => {this.pieces = board})
    )
  }
  
  showMoves(moves: number[]) {
    console.log("Hallo", moves)
    this.potentialMoves = moves;
  }
  clearMoves() {
    this.potentialMoves = [];
  }
  ngOnInit(): void {}
}
