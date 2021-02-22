/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import { EventData } from "./StreamDeckAction";
import * as Client from "StreamingRemoteClient";
import images from "./Images";
import {StreamingRemoteClientActionSettings as BaseSettings, StreamingRemoteClientAction} from "./StreamingRemoteClientAction";
import { ActionIDs } from "../ActionIDs";
import { PIEvents, PluginEvents } from "../EventIDs";

interface StartStopSettings extends BaseSettings {
  output: string;
  outputName: string;
  outputType: string;
};

export class StartStopOutputAction extends StreamingRemoteClientAction<StartStopSettings> {
  public static readonly UUID = ActionIDs.StartStopOutput;

  private output: Client.Output;

  public async willAppear(data: EventData): Promise<void> {
    if (!this.output) {
      await this.initOutputFromSettings(data.settings);
    }
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
    await Promise.all([
      this.updateStateAndImage(),
      this.sendData(),
    ]);
  }

  protected async onWebSocketClose(): Promise<void> {
    this.output = null;
  }

  protected async onWebSocketError(): Promise<void> {
    this.onWebSocketClose();
  }

  private async updateStateAndImage(): Promise<void> {
    const key = this.getSettings().output;
    if (!(this.rpc && key)) {
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


  public async sendToPlugin(untypedPayload: any): Promise<void> {
    const payload = untypedPayload as {
      event: string;
    };
    const event = payload.event;

    if (event == PIEvents.GetData) {
      await this.sendData();
      return;
    }

    console.log(`Received unhandled event: ${event}`);
  }

  public async settingsDidChange(old: StartStopSettings, settings: StartStopSettings) {
    try {
      await super.settingsDidChange(old, settings);
    } catch (ignored) {
    }

    if (settings.outputName) {
      await this.initOutputFromSettings(settings);
    }

    await this.sendData();
  }

  private async initOutputFromSettings(settings: StartStopSettings) {
    const {output, outputName, outputType} = settings;
    this.output = { id: output, name: outputName, type: outputType as Client.OutputType, state: Client.OutputState.STOPPED };
    await this.updateImage();
  }

  private async sendData(): Promise<void> {
    if (!this.rpc) {
      return;
    }
    let outputs: { [id: string]: Client.Output } = {};
    try {
      outputs = await this.rpc.getOutputs();
    } catch (e) {
    }
    this.websocket.send(JSON.stringify({
      event: 'sendToPropertyInspector',
      context: this.context,
      payload: { event: PluginEvents.SetData, outputs, settings: this.getSettings() }
    }));
  }
}

export default StartStopOutputAction;
