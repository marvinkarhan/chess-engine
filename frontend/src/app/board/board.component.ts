import { AfterViewChecked, AfterViewInit, Component, ElementRef, HostListener, Input, OnChanges, OnInit, ViewChild } from '@angular/core';
import { fromEvent, Observable } from 'rxjs';
import { debounceTime, take } from 'rxjs/operators';
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
export class BoardComponent implements OnInit, AfterViewInit {
  boardWidth = 800;
  fontSize = 12;

  pieces: Board = [];
  potentialMoves: number[] = [];

  @HostListener('document:keyup', ['$event'])
  handleKeyboardEvent(event: KeyboardEvent) {
    if (event.key === "ArrowLeft") {
      this.boardService.unmakeMove();
    }
  }

  constructor(public boardService: BoardService) {}

  ngAfterViewInit() {
    this.calcBoardDim(window);
  }

  ngOnInit() {
    fromEvent(window, 'resize').subscribe((e) => {
      const window = e.target as Window;
      this.calcBoardDim(window);
    });
    this.boardService.requestNewBoard();
  }

  private calcBoardDim(window: Window): void {
    const windowDim = Math.min(window.innerWidth, window.innerHeight);
    this.boardWidth = windowDim * 0.8;
    // normalize the value to get the font size
    this.fontSize = (this.boardWidth - 200) / (800 - 200) + 0.2;
    console.log('window resized: ', window.innerWidth, this.boardWidth)
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
