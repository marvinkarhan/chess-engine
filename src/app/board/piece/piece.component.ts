import { Component, Input, OnInit } from '@angular/core';
import { Piece } from '../interfaces/Piece';

@Component({
  selector: 'piece',
  templateUrl: './piece.component.html',
  styleUrls: ['./piece.component.scss'],
})
export class PieceComponent implements OnInit {
  @Input() pieceProperties!: Piece;
  @Input() boardWidth = 800;

  constructor() {}

  ngOnInit(): void {}
}
