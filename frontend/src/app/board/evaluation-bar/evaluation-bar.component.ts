import { DecimalPipe } from '@angular/common';
import { Component } from '@angular/core';
import { combineLatest, Observable } from 'rxjs';
import { map } from 'rxjs/operators';
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

  constructor(public service: BoardService, private decimalPipe: DecimalPipe) {
    this.scorePercent$ = combineLatest([this.service.evaluation$, this.service.whitePOV$]).pipe(
      map(([evaluation, whitePOV]) => {
        if (!whitePOV) {
          evaluation.score = (evaluation.score || 0.5) * -1;
        }
        if (evaluation.mateIn) {
          return evaluation.mateIn > 0 ? 0 : 100;
        }
        return this.log(evaluation.score || 0.5);
      })
    );

    this.evalString$ = combineLatest([this.service.evaluation$, this.service.whitePOV$]).pipe(
      map(([evaluation, whitePOV]) => {
        if (evaluation.mateIn) {
          return 'M' + Math.abs(evaluation.mateIn);
        }
        if (whitePOV) {
          evaluation.score = (evaluation.score || 0.5) * -1;
        }
        return this.decimalPipe.transform(evaluation.score, '1.0-2') || '';
      })
    );
  }

  log(evaluation: number): number {
    //Math.log(this.evalValue! +1))
    // clamp the value
    return Math.min(Math.max(((10 + evaluation) / 20) * 100, 5), 95);
  }
}
