import { Injectable } from '@angular/core';
import { Socket } from 'ngx-socket-io';
import { fromEvent, Observable } from 'rxjs';
import { map } from 'rxjs/operators';
import { ChessApiEmits, ChessApiEvents } from '../enums/ChessApiEvents';
import { BoardInformation } from '../interfaces/BoardInformation';
import {
  Board,
  BoardInitializedFENStringEvent,
  Piece,
} from '../interfaces/Piece';
import { BoardService } from './board.service';

@Injectable({
  providedIn: 'root',
})
export class ChessApiService {

  newSocket: WebSocket;
  constructor(private socket: Socket) {
    this.newSocket = new WebSocket("ws://localhost:8000/ws")
  } 
  
  connect() {
    return fromEvent(this.newSocket, 'open');
  }
  

  onNewBoardInformation() : Observable<BoardInformation> {
    return fromEvent<MessageEvent>(this.newSocket, 'message').pipe(
      map((event: MessageEvent) => JSON.parse(event.data))
    );
  }

  requestNewBoard() : void {
    this.newSocket?.send(ChessApiEmits.NEW_BOARD);
    this.socket.emit(ChessApiEmits.NEW_BOARD);
  }

  requestNewMove(uciMove: string): void {
    this.newSocket?.send(ChessApiEmits.MAKE_MOVE);
    this.socket.emit(ChessApiEmits.MAKE_MOVE, uciMove);
  } 

}
