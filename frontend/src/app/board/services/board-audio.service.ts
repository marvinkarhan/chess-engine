import { Injectable } from '@angular/core';

@Injectable({
  providedIn: 'root'
})
export class BoardAudioService {

  constructor() {

   }

  playMoveSound() {
    let audio = new Audio("../../../assets/sounds/Move.ogg");
    audio.load();
    audio.play();
  }
}
