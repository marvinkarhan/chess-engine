import { Injectable } from '@angular/core';
import { Socket } from 'ngx-socket-io';
import { Observable } from 'rxjs';
import { map, mapTo, reduce, switchMap } from 'rxjs/operators';
import { ALGEBRAIC_TO_INDEX } from '../constants/BoardConstants';
import { ChessApiEmits, ChessApiEvents } from '../enums/ChessApiEvents';
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

  createNewBoard(): Observable<string> {
    this.socket.emit(ChessApiEmits.NEW_BOARD);
    return this.socket.fromEvent<string>(ChessApiEvents.BOARD_INITIALIZED);
  }

  getBoardMoves(): Observable<string[]> {
    this.socket.emit(ChessApiEmits.GET_BOARD_MOVES);
    return this.socket.fromEvent<string[]>(ChessApiEvents.NEW_BOARD_MOVES);
  }
}
