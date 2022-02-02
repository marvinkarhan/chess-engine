import { TestBed } from '@angular/core/testing';

import { WasmEngineService } from './wasm-engine.service';

describe('WasmEngineService', () => {
  let service: WasmEngineService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(WasmEngineService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
