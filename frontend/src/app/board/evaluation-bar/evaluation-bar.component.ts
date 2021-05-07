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

}
