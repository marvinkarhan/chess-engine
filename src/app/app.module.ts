import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { BoardComponent } from './board/board.component';
import { PieceComponent } from './board/piece/piece.component';
import { DirectivesModule } from './directives/directives.module';

@NgModule({
  declarations: [AppComponent, BoardComponent, PieceComponent],
  imports: [BrowserModule, AppRoutingModule, DirectivesModule],
  providers: [],
  bootstrap: [AppComponent],
})
export class AppModule {}
