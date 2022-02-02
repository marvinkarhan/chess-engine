import { Injectable } from '@angular/core';

@Injectable({
  providedIn: 'root',
})
export class WasmEngineService {
  private module: any;

  private readonly moduleArgs = {
    print: (value: string) => {
      console.log(value);
    },
  };
  constructor() {
    this.loadWasm().then(() => {
      // this.module.cwrap('processCommand', null, ['string'])('fen')
    });
  }

  async loadWasm() {
    this.module = await (window as any)['Module'](this.moduleArgs);
  }
}
