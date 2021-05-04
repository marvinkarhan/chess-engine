import { Injectable } from '@angular/core';
import { Socket } from 'ngx-socket-io';
import { Observable } from 'rxjs';
import { map } from 'rxjs/operators';
import { ChessApiEmits, ChessApiEvents } from '../enums/ChessApiEvents';
import { Board, BoardInitializedFENStringEvent, Piece } from '../interfaces/Piece';
import { BoardService } from './board.service';

@Injectable({
  providedIn: 'root'
})
export class ChessApiService {

  constructor(private socket: Socket, private boardService: BoardService) {

   }

   createNewBoard() : Observable<Board> {
     this.socket.emit(ChessApiEmits.NEW_BOARD);
     let event = this.socket.fromEvent<string>(ChessApiEvents.BOARD_INITIALIZED);
     return event.pipe<Board>(map((fen: string) => this.boardService.loadFENString(fen)));
   }
}
