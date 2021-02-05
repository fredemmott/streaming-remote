/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import * as sodium from 'libsodium-wrappers-sumo';
import CryptoState from './CryptoState';
import { OutputState, Output, Scene } from './Types';

interface JSONRPCMessage {
  jsonrpc: "2.0",
}
interface JSONRPCRequest extends JSONRPCMessage {
  id: number | string | null,
  method: string,
  params?: any,
}
interface JSONRPCNotification extends JSONRPCMessage {
  method: string,
  params?: any,
}
interface JSONRPCSuccessResponse extends JSONRPCMessage {
  id: number | string | null,
  result: any,
}
interface JSONRPCErrorResponse extends JSONRPCMessage {
  id: number | string | null,
  error: any,
}
type JSONRPCResponse = JSONRPCSuccessResponse | JSONRPCErrorResponse;

export type HelloCallback = () => void | Promise<void>;
export type OutputStateChangedCallback =
  (id: string, state: OutputState) => void | Promise<void>;


export default class Client {
  private ws: WebSocket;
  private cryptoState: CryptoState;
  constructor(ws: WebSocket, cryptoState: CryptoState) {
    this.ws = ws;
    ws.addEventListener('message', e => this.handleMessage(e));
    this.cryptoState = cryptoState;
  }

  private helloCallbacks: Array<HelloCallback> = [];
  public onHelloNotification(cb: HelloCallback): void {
    this.helloCallbacks.push(cb);
  }

  private outputStateChangedCallbacks: Array<OutputStateChangedCallback> = [];
  public onOutputStateChanged(cb: OutputStateChangedCallback): void {
    this.outputStateChangedCallbacks.push(cb);
  }

  private sendMessage(message: JSONRPCMessage): void {
    const json = JSON.stringify(message);
    const utf8 = (new TextEncoder()).encode(json);
    const encrypted = sodium.crypto_secretstream_xchacha20poly1305_push(
      this.cryptoState.pushState,
      utf8,
      /* ad = */ null,
      /* tag = */ 0
    );
    this.ws.send(encrypted);
  }

  private handleMessage(e: MessageEvent): void {
    const encrypted = new Uint8Array(e.data);
    const utf8 = sodium.crypto_secretstream_xchacha20poly1305_pull(
      this.cryptoState.pullState,
      encrypted,
    ).message;

    const json = (new TextDecoder('utf-8')).decode(utf8);
    const payload = JSON.parse(json);
    if (payload.jsonrpc != "2.0") {
      throw "Expected JSONRPC";
    }
    if (payload.method) {
      this.handleNotification(payload as JSONRPCNotification);
      return;
    }
    if (payload.id !== undefined) {
      this.handleResponse(payload as JSONRPCResponse);
      return;
    }
  }

  private handleNotification(message: JSONRPCNotification): void {
    if (message.method == "hello") {
      this.helloCallbacks.forEach(cb => cb());
      return;
    }
    if (message.method == "outputs/stateChanged") {
      this.outputStateChangedCallbacks.forEach(cb => cb(
        message.params.id,
        message.params.state
      ));
      return;
    }
  }

  private responseHandlers: {
    [id: string]: {
      resolve: (any) => void | Promise<void>,
      reject: (any) => void | Promise<void>,
    }
  } = {};

  private handleResponse(message: JSONRPCResponse): void {
    if (!message['id']) {
      return;
    }
    const handler = this.responseHandlers[message.id];
    if (!handler) {
      return;
    }
    delete this.responseHandlers[message.id];

    if (message['result']) {
      handler.resolve((message as JSONRPCSuccessResponse).result);
      return;
    }

    handler.reject(message['error']);
  }

  private nextId: number = 1;
  private async sendRequest(method: string, params: any): Promise<any> {
    const id = this.nextId.toString();
    this.nextId++;

    const message: JSONRPCRequest = {
      jsonrpc: "2.0",
      id,
      method,
      params,
    };

    return await new Promise((resolve, reject) => {
      this.responseHandlers[id] = { resolve, reject };
      this.sendMessage(message);
    });
  }

  public async getOutputs(): Promise<{ [id: string]: Output }> {
    return await this.sendRequest('outputs/get', null);
  }

  public async getScenes(): Promise<{ [id: string]: Scene }> {
    return await this.sendRequest('scenes/get', null);
  }

  public async setDelay(id: string, seconds: number): Promise<void> {
    await this.sendRequest('outputs/setDelay', { id, seconds });
  }

  public async startOutput(id: string): Promise<void> {
    await this.sendRequest("outputs/start", { id });
  }

  public async stopOutput(id: string): Promise<void> {
    await this.sendRequest("outputs/stop", { id });
  }
}
