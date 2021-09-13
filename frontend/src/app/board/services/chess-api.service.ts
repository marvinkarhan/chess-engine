import { Injectable } from '@angular/core';
import { BehaviorSubject, fromEvent, Observable, Subject } from 'rxjs';
import { map } from 'rxjs/operators';
import { io, Socket } from 'socket.io-client';
import { DefaultEventsMap } from 'socket.io-client/build/typed-events';
import { environment } from 'src/environments/environment';
import { ChessApiEmits } from '../enums/ChessApiEvents';
import { BoardInformation } from '../interfaces/BoardInformation';

@Injectable({
  providedIn: 'root',
})
export class ChessApiService {
  private socket: Socket<DefaultEventsMap, DefaultEventsMap>;
  private boardInfo$ = new Subject<BoardInformation>();

  constructor() {
    this.socket = io(
      `${environment.socketProtocol}://${environment.socketServerURI}`,
      { transports: ['websocket', 'polling'] }
    );
    this.socket.on('board_info', (boardInfo) => {
      console.log('boardInfo :', boardInfo);
      this.boardInfo$.next(boardInfo);
    });
  }

  onNewBoardInformation$(): Observable<BoardInformation> {
    return this.boardInfo$.asObservable();
  }

  changeTime(time: number): void {
    this.socket.emit(ChessApiEmits.CHANGE_TIME, time * 100);
  }

  requestNewBoard(fen: string): void {
    this.socket.emit(ChessApiEmits.NEW_BOARD, fen);
  }

  requestMakeMove(uciMove: string, callback?: () => void): void {
    this.socket.emit(ChessApiEmits.MAKE_MOVE, uciMove, callback);
  }

  requestNewEngineMove(): void {
    this.socket.emit(ChessApiEmits.NEW_ENGINE_MOVE);
  }
}
