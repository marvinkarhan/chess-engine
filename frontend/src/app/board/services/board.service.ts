import { Injectable, OnDestroy } from '@angular/core';
import {
  BehaviorSubject,
  forkJoin,
  merge,
  Observable,
  Subscription,
} from 'rxjs';
import { map, startWith, switchMap, tap } from 'rxjs/operators';
import { ALGEBRAIC_TO_INDEX } from '../constants/BoardConstants';
import { Side } from '../enums/Side';
import { BoardInformation } from '../interfaces/BoardInformation';
import { Board, Move, Piece, PieceTypes } from '../interfaces/Piece';
import { BoardAudioService } from './board-audio.service';
import { ChessApiService } from './chess-api.service';

@Injectable({
  providedIn: 'root',
})
export class BoardService implements OnDestroy {
  fullMoves: number = 0;
  halfMoves: number = 0;
  private skipOne = false;
  private _sideToMove$ = new BehaviorSubject<Side>(Side.white);
  public sideToMove$ = this._sideToMove$.asObservable();
  private _pieces$ = new BehaviorSubject<Board>([]);
  public pieces$ = this._pieces$.asObservable();
  private _evaluation$ = new BehaviorSubject<number>(0.5);
  public evaluation$ = this._evaluation$.asObservable();
  private _aiMoves$ = new BehaviorSubject<string[]>([]);
  public aiMoves$ = this._aiMoves$.asObservable();
  private _engineThinking$ = new BehaviorSubject<boolean>(false);
  public engineThinking$ = this._engineThinking$.asObservable();
  private _fen$ = new BehaviorSubject<string>('');
  public fen$ = this._fen$.asObservable();
  private _subs$ = new Subscription();

  private START_POS_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1';

  constructor(
    private chessApi: ChessApiService,
    private _boardAudio: BoardAudioService
  ) {
    this._setupBoardInformationListener();
  }

  ngOnDestroy(): void {
    this._subs$.unsubscribe();
  }

  makeMove(move: Move) {
    const [oldPosition, newPosition, promotion] = move;
    const oldPositionIndex = oldPosition.x + 8 * oldPosition.y;
    const newPositionIndex = newPosition.x + 8 * newPosition.y;
    const positions = Object.keys(ALGEBRAIC_TO_INDEX);
    const uciMove = `${positions[oldPositionIndex]}${positions[newPositionIndex]}${promotion}`;
    this.skipOne = true;
    this.chessApi.requestMakeMove(uciMove);
    this.chessApi.requestNewEngineMove();
    this._engineThinking$.next(true);
    this._clearMoves();
    // let pieces = this._pieces$.value;
    // pieces[newPositionIndex] = pieces[oldPositionIndex];
    // pieces[oldPositionIndex] = undefined;
  }

  unmakeMove() {
    this.chessApi.requestUnmakeMove();
  }

  swapSide() {
    this._engineThinking$.next(true);
    this.chessApi.requestNewEngineMove();
  }

  newBoard(fen = this.START_POS_FEN) {
    this.chessApi.requestNewBoard(fen);
  }

  changeTime(time: number) {
    this.chessApi.changeTime(time);
  }

  private _clearMoves() {
    let currentPieces = this._pieces$.value;
    currentPieces.forEach((piece) => {
      if (piece) {
        piece.possibleTargetSquares = [];
      }
    });
    this._pieces$.next(currentPieces);
  }

  private _setupBoardInformationListener(): void {
    this._subs$.add(
      this.chessApi
        .onNewBoardInformation()!
        .subscribe((boardInformation) => {
          let pieces = this._loadFENString(boardInformation.fen);
          this._uciToBoard(pieces, boardInformation.moves);
          this._pieces$.next(pieces);
          if (!this.skipOne) {
            this._engineThinking$.next(false);
            this._evaluation$.next(boardInformation.evaluation);
            this._aiMoves$.next(boardInformation.aiMoves);
            this._fen$.next(boardInformation.fen);
            this._boardAudio.playMoveSound();
          }
          this.skipOne = false;
          console.log('DEBUG BOARDINFO', boardInformation);
        })
    );
  }

  requestNewBoard() {
    this.chessApi.connect().subscribe(()=>this.chessApi.requestNewBoard(this.START_POS_FEN));
  }

  private _uciToBoard(currentPieces: Board, uciMoves: string[]) {
    uciMoves.forEach((uciMove: string) => {
      let startSquare: string = uciMove[0] + uciMove[1];
      let targetSquare = uciMove[2] + uciMove[3];
      let promotion = uciMove[4] ? true : false || false;
      const length: number = Object.keys(ALGEBRAIC_TO_INDEX).length;
      let startSquareIndex: number =
        length - ALGEBRAIC_TO_INDEX[startSquare] - 1;
      let targetSquareIndex: number =
        length - ALGEBRAIC_TO_INDEX[targetSquare] - 1;
      currentPieces[startSquareIndex]?.possibleTargetSquares.push(
        targetSquareIndex
      );
      if (currentPieces[startSquareIndex]) {
        currentPieces[startSquareIndex]!.promotion = promotion;
      }
    });
    return currentPieces;
  }

  private _loadFENString(fenString: string): Board {
    let [
      placement,
      sideToMove,
      castlingAbility,
      epSquare,
      halfMoves,
      fullMoves,
    ] = fenString.split(' ');
    this.fullMoves = +fullMoves;
    this.halfMoves = +halfMoves;
    this._sideToMove$.next(sideToMove as Side);

    const rows = placement.split('/');
    const pieces: Board = [];
    rows.forEach((row, i) => {
      row.split('').forEach((char, j) => {
        // make use of ascii char order
        if (char.charCodeAt(0) < '9'.charCodeAt(0)) {
          const skips = +char;
          for (let skip = 0; skip < skips; skip++) {
            pieces.push(undefined);
          }
        } else {
          let color =
            char.charCodeAt(0) < 'Z'.charCodeAt(0) ? Side.white : Side.black;
          pieces.push({
            type: (color + char.toLowerCase()) as PieceTypes,
            possibleTargetSquares: [],
            promotion: false,
          });
        }
      });
    });
    return pieces;
  }
}
