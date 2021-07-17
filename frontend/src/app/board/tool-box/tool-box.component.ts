import { Component, OnInit } from '@angular/core';
import { faChessBoard, faChessPawn, faRetweet, faSync } from '@fortawesome/free-solid-svg-icons';
import { faHourglass } from '@fortawesome/free-regular-svg-icons';
import { combineLatest } from 'rxjs';
import { map } from 'rxjs/operators';
import { BoardService } from '../services/board.service';

@Component({
  selector: 'tool-box',
  templateUrl: './tool-box.component.html',
  styleUrls: ['./tool-box.component.scss'],
})
export class ToolBoxComponent {
  faSwapBoard = faRetweet;
  faNewGame = faSync;
  faTime = faHourglass;
  faBoard = faChessBoard;
  faPawn = faChessPawn;
  engineTime: number | undefined = 5;
  private MATE_EVALUATION = 2000;
  private fen: string | undefined = undefined;

  showEvalBar$ = this.service.aiMoves$.pipe(
    map((aiMoves) => aiMoves?.length > 0 && aiMoves[0] != '')
  );

  isMate$ = combineLatest([
    this.service.evaluation$,
    this.service.aiMoves$,
  ]).pipe(map(([evaluation, aiMoves]) => this.isMateScore(evaluation) && aiMoves.length <= 1));

  constructor(public service: BoardService) {}

  swapSide() {
    this.service.swapSide();
  }

  newBoard() {
    this.service.newBoard();
  }

  isMateScore(evaluation: number): boolean {
    return (
      evaluation < -this.MATE_EVALUATION || evaluation > this.MATE_EVALUATION
    );
  }

  onKeydown(event: KeyboardEvent) {
    if (event.key === 'Enter') {
      (event.target as HTMLElement).blur();
    }
  }

  fenEntered(fen: string) {
    this.fen = fen;
  }

  emitFen() {
    if (this.fen)
      this.service.newBoard(this.fen);
  }

  changeTime(time: number) {
    console.log(time);
    if (time) {
      this.engineTime = time;
      this.service.changeTime(time);
    }
  }
}
