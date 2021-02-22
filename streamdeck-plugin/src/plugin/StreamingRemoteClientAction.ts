/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import { EventData, StreamDeckAction } from "./StreamDeckAction";
import * as Client from "StreamingRemoteClient";
import * as ESD from "./ESDTypes";
import {PluginEvents} from "../EventIDs"

function create_streamingremote_client(uri: string, password: string): Promise<[Client.RPC, WebSocket]> {
  const ws = new WebSocket(uri);
  ws.binaryType = 'arraybuffer';
  return new Promise((resolve, reject) => {
    ws.addEventListener('open', async () => {
      const handshakeState = await Client.handshake(ws, password);
      const rpc = new Client.RPC(ws, handshakeState);
      rpc.onHelloNotification(() => resolve([rpc, ws]));
    });
    ws.addEventListener('close', (e: CloseEvent) => {
      reject(`Websocket connection closed: ${e.code} ("${e.reason}")`);
    });
  });
}

export interface StreamingRemoteClientActionSettings {
  password: string;
  uri: string;
};

type Message = { event: string, context: string, payload: any };

export abstract class StreamingRemoteClientAction<TSettings extends StreamingRemoteClientActionSettings > extends StreamDeckAction<TSettings> {
  public static readonly UUID: string;

  protected rpc: Client.RPC;
  private streamingRemoteWebSocket: WebSocket;

  constructor(context: ESD.Context, ws: WebSocket) {
    super(context, ws);
    setInterval(() => this.displayAndRetryBadConnection(), 1000);
  }

  protected abstract onConnect(): Promise<void>;
  protected abstract onWebSocketClose(): Promise<void>;
  protected abstract onWebSocketError(): Promise<void>;

  public async willAppear(_data: EventData): Promise<void> {
    if (this.rpc) {
      return;
    }

    this.websocket.send(JSON.stringify({
      event: 'showAlert',
      context: this.context,
    }));
    await this.connectRemote();
  }

  protected async connectRemote(): Promise<void> {
    const { uri, password } = this.getSettings();
    if (this.streamingRemoteWebSocket) {
      this.streamingRemoteWebSocket.close();
      this.streamingRemoteWebSocket = null;
    }

    if (!(uri && password)) {
      return;
    }
    const [rpc, ws] = await create_streamingremote_client(uri, password);
    this.streamingRemoteWebSocket = ws;
    this.rpc = rpc;
    ws.addEventListener('close', () => {
      this.websocket.send(JSON.stringify({
        event: 'sendToPropertyInspector',
        context: this.context,
        payload: { event: PluginEvents.Disconnected },
      }));
      console.log('sent disconnected event');

      this.websocket.send(JSON.stringify({
        event: 'showAlert',
        context: this.context,
      }));
      this.rpc = undefined;
      this.onWebSocketClose();
    });
    ws.addEventListener('error', () => {
      this.websocket.send(JSON.stringify({
        event: 'showAlert',
        context: this.context,
      }));
      this.rpc = undefined;
      this.onWebSocketError();
    });
    await this.onConnect();
  }

  private async displayAndRetryBadConnection(): Promise<void> {
    if (this.rpc) {
      return;
    }
    this.websocket.send(JSON.stringify({
      event: 'showAlert',
      context: this.context,
    }));
    if (this.rpc) {
      return;
    }
    try {
      await this.connectRemote();
    } catch (e) {
    }
  }

  public async settingsDidChange(old: TSettings, settings: TSettings) {
    if ((!old) || old.uri != settings.uri || old.password != settings.password) {
      this.rpc = null;
      await this.connectRemote();
    }
  }
}

export default StreamingRemoteClientAction;
