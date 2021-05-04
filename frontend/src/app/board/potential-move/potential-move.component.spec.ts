import { ComponentFixture, TestBed } from '@angular/core/testing';

import { PotentialMoveComponent } from './potential-move.component';

describe('PotentialMoveComponent', () => {
  let component: PotentialMoveComponent;
  let fixture: ComponentFixture<PotentialMoveComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ PotentialMoveComponent ]
    })
    .compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(PotentialMoveComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
