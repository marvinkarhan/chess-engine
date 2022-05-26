import { Component } from '@angular/core';
import { faChessBoard, faChessPawn, faRetweet, faSync, faVolumeMute, faVolumeUp } from '@fortawesome/free-solid-svg-icons';
import { faHourglass } from '@fortawesome/free-regular-svg-icons';
import { combineLatest } from 'rxjs';
import { map } from 'rxjs/operators';
import { BoardService } from '../services/board.service';
import { BoardAudioService } from '../services/board-audio.service';

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
  faMute = faVolumeMute;
  faSound = faVolumeUp;
  private MATE_EVALUATION = 2000;
  private fen: string | undefined = undefined;

  showEvalBar$ = this.service.aiMoves$.pipe(
    map((aiMoves) => aiMoves?.length > 0 && aiMoves[0] != '')
  );

  isMate$ = combineLatest([
    this.service.evaluation$,
    this.service.aiMoves$,
  ]).pipe(map(([evaluation, aiMoves]) => evaluation.mateIn && aiMoves.length <= 1));

  constructor(public service: BoardService, public audioService: BoardAudioService) {}

  swapSide() {
    this.service.swapSide();
  }

  newBoard() {
    this.service.newBoard();
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
      this.service.engineTime = time;
      this.service.changeTime(time);
    }
  }
}
