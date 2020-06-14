/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import { EventData, StreamDeckAction } from "./StreamDeckAction";
import * as Client from "StreamingRemoteClient";
import images from "./Images";
import * as ESD from "./ESDTypes";

function create_streamingremote_client(uri: string, password: string): Promise<[Client.RPC, WebSocket]> {
  const ws = new WebSocket(uri);
  ws.binaryType = 'arraybuffer';
  return new Promise((resolve, reject) => {
    ws.addEventListener('open', async () => {
      const handshakeState = await Client.handshake(ws, password);
      const rpc = new Client.RPC(ws, handshakeState);
      rpc.onHelloNotification(() => resolve([rpc, ws]));
    });
    ws.addEventListener('close', () => {
      reject();
    });
  });
}

type StartStopSettings = {
  output: string;
  password: string;
  uri: string;
};

export class StartStopOutputAction extends StreamDeckAction<StartStopSettings> {
  public static readonly UUID = "com.fredemmott.streamingremote.action";

  private rpc: Client.RPC;
  private output: Client.Output;

  constructor(context: ESD.Context, ws: WebSocket) {
    super(context, ws);
    setInterval(() => this.displayAndRetryBadConnection(), 1000);
  }

  public async keyUp(_data: EventData): Promise<void> {
    const state = this.output.state;
    if (state == 'stopped') {
      await this.rpc.startOutput(this.output.id);
    }
    else {
      await this.rpc.stopOutput(this.output.id);
    }
  }


  private async updateStateAndImage(): Promise<void> {
    const outputs = await this.rpc.getOutputs();
    this.output = outputs[this.getSettings().output];
    if (!this.output.type) {
      return;
    }
    this.updateImage();
  }

  private async updateImage(): Promise<void> {
    var suffix;
    switch (this.output.state) {
      case 'stopped':
        suffix = 'Stopped';
        break;
      case 'starting':
      case 'stopping':
        suffix = 'Changing';
        break;
      case 'active':
        suffix = 'Active';
        break;
    }
    const key = (this.output.type == 'local_recording' ? 'recording' : 'streaming') + suffix;
    const image = (await images())[key];
    this.websocket.send(JSON.stringify({
      event: 'setImage',
      context: this.context,
      payload: { image }
    }));
  }


  public async willAppear(_data: EventData): Promise<void> {
    if (this.rpc) {
      await this.updateImage();
      return;
    }

    this.websocket.send(JSON.stringify({
      event: 'showAlert',
      context: this.context,
    }));
    await this.connectRemote();
    await this.updateStateAndImage();
  }

  private streamingRemoteWebSocket: WebSocket;

  private async connectRemote(): Promise<void> {
    const { uri, password, output } = this.getSettings();
    if (this.streamingRemoteWebSocket) {
      this.streamingRemoteWebSocket.close();
      this.streamingRemoteWebSocket = null;
    }
    const [rpc, ws] = await create_streamingremote_client(uri, password);
    this.streamingRemoteWebSocket = ws;
    this.rpc = rpc;
    this.output = undefined;
    ws.addEventListener('close', () => {
      this.websocket.send(JSON.stringify({
        event: 'showAlert',
        context: this.context,
      }));
      this.rpc = undefined;
      this.output = undefined;
    });
    ws.addEventListener('error', () => {
      this.websocket.send(JSON.stringify({
        event: 'showAlert',
        context: this.context,
      }));
      this.rpc = undefined;
      this.output = undefined;
    });
    rpc.onOutputStateChanged((id: string, state: Client.OutputState) => {
      if (id == output) {
        this.output.state = state;
        this.updateImage();
      }
    });
  }

  private async displayAndRetryBadConnection(): Promise<void> {
    if (this.rpc && this.output) {
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
      await this.updateStateAndImage();
    } catch (e) {
    }
  }

  public async sendToPlugin(untypedPayload: any): Promise<void> {
    const payload = untypedPayload as {
      event: string;
    };
    const event = payload.event;

    if (event == 'getData') {
      const settings = this.getSettings();
      var outputs;
      try {
        outputs = await this.rpc.getOutputs();
      }
      catch (e) {
        outputs = {};
      }

      const json = {
        event: "sendToPropertyInspector",
        context: this.context,
        payload: { event: "data", outputs, settings }
      };
      this.websocket.send(JSON.stringify(json));
      return;
    }

    console.log(`Received unhandled event: ${event}`);
  }

  public async settingsDidChange(old: StartStopSettings, settings: StartStopSettings) {
    if ((!old) || old.uri != settings.uri || old.password != settings.password) {
      this.rpc = null;
      await this.connectRemote();
    }
    await this.updateStateAndImage();

    let outputs: { [id: string]: Client.Output } = {};
    try {
      outputs = await this.rpc.getOutputs();
    } catch (e) {
    }
    this.websocket.send(JSON.stringify({
      event: 'sendToPropertyInspector',
      context: this.context,
      payload: { event: 'data', outputs, settings }
    }));
  }
}

export default StartStopOutputAction;
