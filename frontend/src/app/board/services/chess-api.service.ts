import { Injectable } from '@angular/core';
import { Socket } from 'ngx-socket-io';
import { Observable } from 'rxjs';
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
  constructor(private socket: Socket) {}

  onNewBoardInformation(): Observable<BoardInformation> {
    return this.socket.fromEvent<BoardInformation>(ChessApiEvents.NEW_BOARD_INFORMATION);
  }

  requestNewBoard() : void {
    this.socket.emit(ChessApiEmits.NEW_BOARD);
  }

  requestNewMove(uciMove: string): void {
    this.socket.emit(ChessApiEmits.MAKE_MOVE, uciMove);
  } 

}
