import { Component, Input, OnInit } from '@angular/core';

@Component({
  selector: 'evaluation-bar',
  templateUrl: './evaluation-bar.component.html',
  styleUrls: ['./evaluation-bar.component.scss']
})
export class EvaluationBarComponent implements OnInit {

  @Input() evalValue!: number | null;
  constructor() { }

  ngOnInit(): void {
  }

  log() {
    //Math.log(this.evalValue! +1))
    return ((10 +  this.evalValue!) / 20) * 100
  }
}
