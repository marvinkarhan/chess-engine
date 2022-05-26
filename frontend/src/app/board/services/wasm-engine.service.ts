import { Injectable } from '@angular/core';
import { BehaviorSubject, Subject, zip } from 'rxjs';
import { map } from 'rxjs/operators';
import { BoardInformation } from '../interfaces/BoardInformation';
import { Evaluation } from '../interfaces/Evaluation';

interface BoardState {
  engineTime: number;
}

@Injectable({
  providedIn: 'root',
})
export class WasmEngineService {
  private engineWorker: Worker;
  private boardState: BoardState = {
    engineTime: 20,
  };

  private _evaluation$ = new BehaviorSubject<Evaluation>({
    score: 0.5,
  });
  public evaluation$ = this._evaluation$.asObservable();
  private _aiMoves$ = new Subject<string[]>();
  public aiMoves$ = this._aiMoves$.asObservable();
  private _fen$ = new Subject<string>();
  private _moves$ = new Subject<string[]>();
  public boardInfo$ = zip(this._fen$, this._moves$).pipe(
    map(
      ([fen, moves]): BoardInformation => ({
        fen,
        moves,
      })
    )
  );

  constructor() {
    this.engineWorker = new Worker('./wasm-engine.worker.ts', {
      type: 'module',
    });
    this.engineWorker.onmessage = ({ data }) =>
      this.processEngineResponse(data);
    this.engineWorker.postMessage('init');
  }

  processEngineResponse(res: string) {
    console.log(res);
    if (res.match(/.*bestmove.*/)) {
      // get top engine move
      const move = res.replace('bestmove ', '');
      this.makeMove(move);
    } else if (res.match(/fen.*/)) {
      // get fen
      this._fen$.next(res.replace('fen ', ''));
    } else if (res.match(/info.*/)) {
      // get evaluation
      let evaluationMatch = res.match(/cp -?\d*/);
      if (evaluationMatch && evaluationMatch[0])
        this._evaluation$.next({
          score: +evaluationMatch[0].replace('cp ', '') / 100,
        });
      // get evaluation on mate
      evaluationMatch = res.match(/mate -?\d*/);
      if (evaluationMatch && evaluationMatch[0])
        this._evaluation$.next({
          mateIn: +evaluationMatch[0].replace('mate ', ''),
        });
      // get aiMoves
      const aiMovesMatch = res.match(/pv (\S{4}(\s|$))+/);
      if (aiMovesMatch && aiMovesMatch[0])
        this._aiMoves$.next(aiMovesMatch[0].replace('pv ', '').split(' '));
    } else if (res.match(/legalmoves.*/)) {
      this._moves$.next(res.replace('legalmoves ', '').split(' '));
    }
  }

  callEngine(command: string) {
    this.engineWorker.postMessage(command);
  }

  changeTime(time: number): void {
    this.boardState.engineTime = time * 100;
  }

  newBoard(fen: string) {
    this.callEngine(`position fen ${fen}`);
    this.getBoardInfo();
  }

  makeMove(uciMove: string) {
    this.callEngine(`move ${uciMove}`);
    this.getBoardInfo();
  }

  newEngineMove() {
    this.callEngine(`go movetime ${this.boardState.engineTime}`);
  }

  private getBoardInfo() {
    this.callEngine('fen');
    this.callEngine('legalmoves');
  }
}
