import { Component, Input, OnInit } from '@angular/core';
import { Position } from '../interfaces/Piece';

@Component({
  selector: 'potential-move',
  templateUrl: './potential-move.component.html',
  styleUrls: ['./potential-move.component.scss']
})
export class PotentialMoveComponent implements OnInit {

  @Input() position!: Position;
  constructor() { 

  }

  ngOnInit(): void {
        //TODO FIXME
    //Correct the y position
    this.position = {x: this.position.x, y: Math.trunc(this.position.y)};
    console.log(this.position)
  }

}
