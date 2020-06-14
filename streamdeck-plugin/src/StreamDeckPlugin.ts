/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import * as ESD from "./ESDTypes";
import { EventData, StreamDeckAction } from "./StreamDeckAction";

export abstract class StreamDeckPlugin {
  protected abstract createAction(context: ESD.Context, action: string, ws: WebSocket): StreamDeckAction<any>;

  constructor(
    port: string,
    uuid: string,
    event: string,
    _streamDeckInfo: string
  ) {
    const ws = new WebSocket(`ws://localhost:${port}`);
    this.websocket = ws;
    ws.addEventListener('open', () => this.websocket.send(JSON.stringify({ uuid, event })));
    ws.addEventListener('message', this.onMessage.bind(this));
  }

  protected deviceDidConnect(device: ESD.Device, info: ESD.DeviceInfo): void | Promise<void> {
  }

  protected deviceDidDisconnect(device: ESD.Device): void | Promise<void> {
  }

  private actions: { [context: string]: StreamDeckAction<any>; } = {}

  private getAction(context: string, uuid: string): StreamDeckAction<any> {
    if (!this.actions[context]) {
      this.actions[context] = this.createAction(context, uuid, this.websocket);
    }
    return this.actions[context];
  }

  private readonly websocket: WebSocket;

  private onMessage(evt: any): void {
    const data = JSON.parse(evt.data) as {
      event: string,
      action: string,
      context: string,
      device: string,
      deviceInfo?: ESD.DeviceInfo,
      payload: any,
    };
    const event = data.event;
    const payload = data.payload || {};

    const extra: EventData = {
      device: data.device as string,
      coordinates: payload.coordinates as ESD.Coordinates,
      state: payload.state as number,
      isInMultiAction: payload.isInMultiAction as boolean,
      userDesiredState: payload.userDesiredState as number,
    };

    const action = data.action ? this.getAction(data.context, data.action) : null;
    if (payload.settings) {
      action.setSettings(payload.settings);
    }

    if (event == "keyDown") {
      action.keyDown(extra);
      return;
    }

    if (event == "keyUp") {
      action.keyUp(extra);
      return;
    }

    if (event == "willAppear") {
      action.willAppear(extra);
      return;
    }

    if (event == "sendToPlugin") {
      action.sendToPlugin(payload);
      return;
    }

    if (event == "titleParametersDidChange") {
      action.titleParametersDidChange(payload.title, payload.titleParameters, extra);
      return;
    }

    if (event == "deviceDidConnect") {
      this.deviceDidConnect(data.device, data.deviceInfo);
      return;
    }

    if (event == "deviceDidDisconnect") {
      this.deviceDidDisconnect(data.device);
      return;
    }

    if (event == "didReceiveSettings") {
      action.setSettings(payload.settings);
      return;
    }

    if (event == "propertyInspectorDidAppear") {
      action.propertyInspectorDidAppear(data.device);
      return;
    }

    if (event == "propertyInspectorDidDisappear") {
      action.propertyInspectorDidDisappear(data.device);
      return;
    }

    console.log(`Unhandled plugin event '${event}'`, data);
  }
}

export default StreamDeckPlugin;
