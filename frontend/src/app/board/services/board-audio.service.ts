import { Injectable } from '@angular/core';

@Injectable({
  providedIn: 'root'
})
export class BoardAudioService {

  audio = new Audio("../../../assets/sounds/Move.ogg");

  constructor() {
    this.audio.load();
  }

  playMoveSound() {
    this.audio.play();
  }
}
