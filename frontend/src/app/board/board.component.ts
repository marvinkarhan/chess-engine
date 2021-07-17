import { Component, HostListener, Input, OnInit } from '@angular/core';
import { Observable } from 'rxjs';
import { take } from 'rxjs/operators';
import { Side } from './enums/Side';
import { Board, Move, Piece, PieceTypes } from './interfaces/Piece';
import { BoardService } from './services/board.service';
import { ChessApiService } from './services/chess-api.service';

const START_POS_FEN: string =
  'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1';

@Component({
  selector: 'board',
  templateUrl: './board.component.html',
  styleUrls: ['./board.component.scss'],
})
export class BoardComponent {
  @Input() boardWidth = 800;

  pieces: Board = [];
  potentialMoves: number[] = [];

  @HostListener('document:keyup', ['$event'])
  handleKeyboardEvent(event: KeyboardEvent) {
    if (event.key === "ArrowLeft") {
      this.boardService.unmakeMove();
    }
  }

  constructor(public boardService: BoardService) {
    this.boardService.requestNewBoard();
  }

  showMoves(moves: number[]) {
    this.potentialMoves = moves;
  }
  clearMoves() {
    this.potentialMoves = [];
  }
  onMove(move: Move) {
    this.boardService.makeMove(move);
  }

  toInt(float: number) {
    return float | 0;
  }
}
