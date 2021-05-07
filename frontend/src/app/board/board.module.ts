import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { BoardComponent } from './board.component';
import { PieceComponent } from './piece/piece.component';
import { PotentialMoveComponent } from './potential-move/potential-move.component';
import { EvaluationBarComponent } from './evaluation-bar/evaluation-bar.component';

@NgModule({
  declarations: [BoardComponent, PieceComponent, PotentialMoveComponent, EvaluationBarComponent],
  imports: [BrowserModule],
  exports: [BoardComponent, PieceComponent],
})
export class BoardModule {}
