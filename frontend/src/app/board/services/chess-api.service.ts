import { Injectable } from '@angular/core';
import { fromEvent, Observable } from 'rxjs';
import { map } from 'rxjs/operators';
import { environment } from 'src/environments/environment';
import { ChessApiEmits } from '../enums/ChessApiEvents';
import { BoardInformation } from '../interfaces/BoardInformation';

@Injectable({
  providedIn: 'root',
})
export class ChessApiService {

  newSocket: WebSocket;

  constructor() {
    this.newSocket = new WebSocket(`wss://${environment.socketServerURI}/ws`);
  }

  connect() {
    return fromEvent(this.newSocket, 'open');
  }


  onNewBoardInformation() : Observable<BoardInformation> {
    return fromEvent<MessageEvent>(this.newSocket, 'message').pipe(
      map((event: MessageEvent) => JSON.parse(event.data))
    );
  }

  changeTime(time: number) {
    let request = {
      emitMessage: ChessApiEmits.CHANGE_TIME,
      time
    };
    this.newSocket?.send(JSON.stringify(request));
  }

  requestNewBoard(fen: string) : void {
    let request = {
      emitMessage: ChessApiEmits.NEW_BOARD,
      fen
    };
    this.newSocket?.send(JSON.stringify(request));
  }

  requestMakeMove(uciMove: string): void {
    let request = {
      emitMessage: ChessApiEmits.MAKE_MOVE,
      move: uciMove
    };
    this.newSocket?.send(JSON.stringify(request));
  }

  requestUnmakeMove(): void {
    let request = {
      emitMessage: ChessApiEmits.UNMAKE_MOVE,
    };
    this.newSocket?.send(JSON.stringify(request));
  }

  requestNewEngineMove(): void {
    let request = {
      emitMessage: ChessApiEmits.NEW_ENGINE_MOVE,
    };
    this.newSocket?.send(JSON.stringify(request));
  }

}
