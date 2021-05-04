import { Injectable } from '@angular/core';
import { Socket } from 'ngx-socket-io';
import { Observable } from 'rxjs';
import { map, mapTo, reduce, switchMap } from 'rxjs/operators';
import { ChessApiEmits, ChessApiEvents } from '../enums/ChessApiEvents';
import { Board, BoardInitializedFENStringEvent, Moves, Piece } from '../interfaces/Piece';
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

   getBoardMoves(): Observable<Moves> {
     this.socket.emit(ChessApiEmits.GET_BOARD_MOVES);
     let event = this.socket.fromEvent<string[]>(ChessApiEvents.NEW_BOARD_MOVES);
     return event.pipe<Moves>(map((uciMoves: string[]) => {
       let moves: Moves = {};
       uciMoves.forEach((uciMove: string) => {
         let startSquare = uciMove[0] + uciMove[1];
         let targetSquare = uciMove[2] + uciMove[3];
         moves[startSquare] = moves[startSquare] ?? []
         moves[startSquare].push(targetSquare);
       })
       return moves;
     }))
   }
}
