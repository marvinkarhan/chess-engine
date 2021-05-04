import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { BoardComponent } from './board.component';
import { PieceComponent } from './piece/piece.component';
import { PotentialMoveComponent } from './potential-move/potential-move.component';

@NgModule({
  declarations: [BoardComponent, PieceComponent, PotentialMoveComponent],
  imports: [BrowserModule],
  exports: [BoardComponent, PieceComponent],
})
export class BoardModule {}
