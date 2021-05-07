import { ComponentFixture, TestBed } from '@angular/core/testing';

import { EvaluationBarComponent } from './evaluation-bar.component';

describe('EvaluationBarComponent', () => {
  let component: EvaluationBarComponent;
  let fixture: ComponentFixture<EvaluationBarComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ EvaluationBarComponent ]
    })
    .compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(EvaluationBarComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
