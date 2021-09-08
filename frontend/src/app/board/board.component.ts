import { AfterViewInit, Component, HostListener, OnInit } from '@angular/core';
import { fromEvent } from 'rxjs';
import { Board, Move } from './interfaces/Piece';
import { BoardService } from './services/board.service';

const START_POS_FEN: string =
  'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1';

@Component({
  selector: 'board',
  templateUrl: './board.component.html',
  styleUrls: ['./board.component.scss'],
})
export class BoardComponent implements OnInit {
  boardWidth = 800;
  reservedSpace = 0;
  fontSize = 12;

  pieces: Board = [];
  potentialMoves: number[] = [];

  private toolboxSize = 340;

  @HostListener('document:keyup', ['$event'])
  handleKeyboardEvent(event: KeyboardEvent) {
    if (event.key === "ArrowLeft") {
      this.boardService.unmakeMove();
    }
  }

  constructor(public boardService: BoardService) {
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
    // the tool box is only shown on width > 1040 so include it in the calculation in that case
    const width = window.innerWidth > 800 ? window.innerWidth - 340 : window.innerWidth;
    const availableSpace = Math.min(width, window.innerHeight);
    const boardSize = window.innerWidth <= 800 ? 1 : 0.8;
    // reserve space for eval bar on width <= 800
    this.reservedSpace = (window.innerWidth <= 800 ? 50 : 0)
    this.boardWidth = availableSpace * boardSize;
    // normalize the value to get the font size
    this.fontSize = (this.boardWidth - 200) / (800 - 200) + 0.2;
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
