import { DecimalPipe } from '@angular/common';
import { Component } from '@angular/core';
import { combineLatest, Observable } from 'rxjs';
import { map } from 'rxjs/operators';
import { Side } from '../enums/Side';
import { BoardService } from '../services/board.service';

@Component({
  selector: 'evaluation-bar',
  templateUrl: './evaluation-bar.component.html',
  styleUrls: ['./evaluation-bar.component.scss'],
  providers: [DecimalPipe],
})
export class EvaluationBarComponent {
  public scorePercent$: Observable<number>;
  public evalString$: Observable<string>;
  private MATE_EVALUATION = 2000;

  constructor(public service: BoardService, private decimalPipe: DecimalPipe) {
    this.scorePercent$ = combineLatest([this.service.evaluation$, this.service.aiMoves$]).pipe(
      map(([evaluation, aiMoves]) => {
        console.log('evaluation: ', evaluation, this.mateIn(evaluation, aiMoves), evaluation < this.MATE_EVALUATION, Math.ceil((aiMoves.length - 1) / 2), this.service.sideToMove)
        if (this.isMate(evaluation, aiMoves)) {
          if (this.service.sideToMove == Side.white)
            return evaluation < 0 ? 0 : 100;
          return evaluation > 0 ? 0 : 100;
        }
        return this.log(evaluation);
      })
    );

    this.evalString$ = combineLatest([this.service.evaluation$, this.service.aiMoves$]).pipe(
      map(([evaluation, aiMoves]) => {
        console.log('evaluation: ', evaluation, this.mateIn(evaluation, aiMoves), evaluation < this.MATE_EVALUATION, Math.ceil((aiMoves.length - 1) / 2), this.service.sideToMove)
        if (this.isMate(evaluation, aiMoves)) {
          return 'M' + this.mateIn(evaluation, aiMoves);
        }
        return this.decimalPipe.transform(evaluation, '1.0-2') || '';
      })
    );
  }

  isMate(evaluation: number, aiMoves: string[]): boolean {
    return evaluation < -this.MATE_EVALUATION || evaluation > this.MATE_EVALUATION;
  }

  mateIn(evaluation: number, aiMoves: string[]): number {
    let mateIn = 0;
    // mate for enemy
    if (evaluation < -this.MATE_EVALUATION || evaluation > this.MATE_EVALUATION) {
      mateIn = Math.ceil((aiMoves.length - 1) / 2);
    }
    return mateIn;
  }

  log(evaluation: number): number {
    //Math.log(this.evalValue! +1))
    // clamp the value
    return Math.min(Math.max(((10 + evaluation) / 20) * 100, 5), 95);
  }
}
