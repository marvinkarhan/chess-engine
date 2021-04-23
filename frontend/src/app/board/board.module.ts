import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { BoardComponent } from './board.component';
import { PieceComponent } from './piece/piece.component';

@NgModule({
  declarations: [BoardComponent, PieceComponent],
  imports: [BrowserModule],
  exports: [BoardComponent, PieceComponent],
})
export class BoardModule {}
