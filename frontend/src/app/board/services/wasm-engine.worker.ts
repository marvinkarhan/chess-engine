/// <reference lib="webworker" />

importScripts('assets/wasm/wasm-engine.js');

const loadWasm = () =>
  new Promise<any>(async (resolve) => {
    const wasm = await fetch('assets/wasm/wasm-engine.wasm');
    const buffer = await wasm.arrayBuffer();
    const engine = await (self as any).Module({
      wasmBinary: buffer,
      print: (value: string) => {
        postMessage(value);
      },
    });
    engine.processCommand = engine.cwrap('processCommand', null, ['string']);
    resolve(engine);
  });

let engine: any = undefined;
// queue for capturing requests that come in before init
const taskQueue: string[] = [];

addEventListener('message', async ({ data: command }) => {
  if (!engine && command != 'init') {
    taskQueue.push(command);
    postMessage('task queued... waiting for init to be done');
    return;
  }
  if (command === 'init') {
    loadWasm().then((_engine) => {
      engine = _engine;
      postMessage('init done');
      while (taskQueue.length) {
        engine.processCommand(taskQueue.shift());
      }
    });
  } else {
    engine.processCommand(command);
  }
});
