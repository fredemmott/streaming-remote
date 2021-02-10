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
import {StreamingRemoteClientActionSettings as BaseSettings, StreamingRemoteClientAction} from "./StreamingRemoteClientAction";
import { ActionIDs } from "./ActionIDs";

interface StartStopSettings extends BaseSettings {
  output: string;
};

export class StartStopOutputAction extends StreamingRemoteClientAction<StartStopSettings> {
  public static readonly UUID = ActionIDs.StartStopOutput;

  private output: Client.Output;

  public async keyUp(_data: EventData): Promise<void> {
    const state = this.output.state;
    if (state == 'stopped') {
      await this.rpc.startOutput(this.output.id);
    }
    else {
      await this.rpc.stopOutput(this.output.id);
    }
  }

  protected async onConnect(): Promise<void> {
    this.rpc.onOutputStateChanged(
      (id: string, state: Client.OutputState) => {
        if (id != this.output.id) {
          return;
        }
        this.output.state = state;
        this.updateImage();
      }
    )
    await this.updateStateAndImage();
  }

  protected async onWebSocketClose(): Promise<void> {
    this.output = null;
  }

  protected async onWebSocketError(): Promise<void> {
    this.onWebSocketClose();
  }

  private async updateStateAndImage(): Promise<void> {
    const key = this.getSettings().output;
    if (!key) {
      return;
    }
    const outputs = await this.rpc.getOutputs();
    this.output = outputs[key];
    if (!(this.output && this.output.type)) {
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


  public async willAppear(data: EventData): Promise<void> {
    await super.willAppear(data);
    await this.updateStateAndImage();
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
        payload: { event: "startStopOutputData", outputs, settings }
      };
      this.websocket.send(JSON.stringify(json));
      return;
    }

    console.log(`Received unhandled event: ${event}`);
  }

  public async settingsDidChange(old: StartStopSettings, settings: StartStopSettings) {
    await super.settingsDidChange(old, settings);

    let outputs: { [id: string]: Client.Output } = {};
    try {
      outputs = await this.rpc.getOutputs();
    } catch (e) {
    }
    this.websocket.send(JSON.stringify({
      event: 'sendToPropertyInspector',
      context: this.context,
      payload: { event: 'startStopOutputData', outputs, settings }
    }));
  }
}

export default StartStopOutputAction;
