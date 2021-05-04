import { TestBed } from '@angular/core/testing';

import { ChessApiService } from './chess-api.service';

describe('ChessApiService', () => {
  let service: ChessApiService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(ChessApiService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
