import { TestBed } from '@angular/core/testing';

import { BoardAudioService } from './board-audio.service';

describe('BoardAudioService', () => {
  let service: BoardAudioService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(BoardAudioService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
