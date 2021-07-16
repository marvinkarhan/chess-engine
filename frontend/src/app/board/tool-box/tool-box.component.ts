import { Component, OnInit } from '@angular/core';
import { SizeProp } from '@fortawesome/fontawesome-svg-core';
import { faRetweet, faSync } from '@fortawesome/free-solid-svg-icons';
import { BoardService } from '../services/board.service';

@Component({
  selector: 'tool-box',
  templateUrl: './tool-box.component.html',
  styleUrls: ['./tool-box.component.scss']
})
export class ToolBoxComponent {
  faSwapBoard = faRetweet;
  faNewGame = faSync;

  constructor(public service: BoardService) { }

  swapSide() {
    this.service.swapSide();
  }

}
