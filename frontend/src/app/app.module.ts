import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { SocketIoConfig, SocketIoModule } from 'ngx-socket-io';
import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { webSocket, WebSocketSubject } from 'rxjs/webSocket';
import { BoardModule } from './board/board.module';
import makeWebSocketObservable from 'rxjs-websockets';

const config: SocketIoConfig = { url: 'http://jjkkkkj:8000', options: {} };

const socket = new WebSocket("ws://localhost:8000/ws")
@NgModule({
  declarations: [AppComponent],
  imports: [BrowserModule, AppRoutingModule, BoardModule, SocketIoModule.forRoot(config)],
  providers: [],
  bootstrap: [AppComponent],
})
export class AppModule {}
