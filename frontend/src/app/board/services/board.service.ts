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
import { ChessApiService } from './chess-api.service';

@Injectable({
  providedIn: 'root',
})
export class BoardService implements OnDestroy {
  fullMoves: number = 0;
  halfMoves: number = 0;
  sideToMove: Side = Side.white;
  private _pieces$ = new BehaviorSubject<Board>([]);
  public pieces$ = this._pieces$.asObservable();
  private _evaluation$ = new BehaviorSubject<number>(0.5);
  public evaluation$ = this._evaluation$.asObservable();
  private _subs$ = new Subscription();

  constructor(private chessApi: ChessApiService) {
    this._setupBoardInformationListener();
  }

  ngOnDestroy(): void {
    this._subs$.unsubscribe();
  }

  makeMove(move: Move) {
    const [oldPosition, newPosition] = move;
    const oldPositionIndex = oldPosition.x + 8 * oldPosition.y;
    const newPositionIndex = newPosition.x + 8 * newPosition.y;
    const positions = Object.keys(ALGEBRAIC_TO_INDEX);
    const uciMove = `${positions[oldPositionIndex]}${positions[newPositionIndex]}`;
    this.chessApi.requestNewMove(uciMove);
    // let pieces = this._pieces$.value;
    // pieces[newPositionIndex] = pieces[oldPositionIndex];
    // pieces[oldPositionIndex] = undefined; 
  }

  private _setupBoardInformationListener() : void {
    this._subs$.add(
      this.chessApi.onNewBoardInformation().subscribe((boardInformation: BoardInformation) => {
        console.log("Hallo")
        let pieces = this._loadFENString(boardInformation.fen);
        this._uciToBoard(pieces, boardInformation.moves);
        this._pieces$.next(pieces);
        this._evaluation$.next(boardInformation.evaluation);
        console.log("update")
      })
    )
  }

  requestNewBoard() {
    this.chessApi.requestNewBoard();
  }

  private _uciToBoard(currentPieces: Board, uciMoves: string[]) {
    uciMoves.forEach((uciMove: string) => {
      let startSquare: string = uciMove[0] + uciMove[1];
      let targetSquare = uciMove[2] + uciMove[3];
      const length: number = Object.keys(ALGEBRAIC_TO_INDEX).length
      let startSquareIndex: number = length - ALGEBRAIC_TO_INDEX[startSquare] - 1;
      let targetSquareIndex: number = length - ALGEBRAIC_TO_INDEX[targetSquare] - 1;
      currentPieces[startSquareIndex]?.possibleTargetSquares.push(
        targetSquareIndex
      );
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
    this.sideToMove = sideToMove as Side;

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
          });
        }
      });
    });
    return pieces;
  }
}
