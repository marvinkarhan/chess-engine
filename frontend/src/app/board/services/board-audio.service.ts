import { Injectable } from '@angular/core';

@Injectable({
  providedIn: 'root'
})
export class BoardAudioService {

  isMuted = false;

  audio = new Audio("../../../assets/sounds/Move.ogg");

  constructor() {
    this.audio.load();
  }

  playMoveSound() {
    if (!this.isMuted)
      this.audio.play();
  }
}
