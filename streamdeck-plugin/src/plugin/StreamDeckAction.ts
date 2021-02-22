/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import * as ESD from "./ESDTypes";

export type EventData = {
  device: ESD.Device;
  coordinates: ESD.Coordinates;
  state: number;
  isInMultiAction: boolean;
  userDesiredState?: number;
  settings?: any;
};

export abstract class StreamDeckAction<TSettings> {
  public static readonly UUID: string;
  protected readonly context: ESD.Context;
  protected readonly websocket: WebSocket;

  constructor(context: ESD.Context, websocket: WebSocket) {
    this.context = context;
    this.websocket = websocket;
  }

  public keyDown(data: EventData): void | Promise<void> { }
  public keyUp(data: EventData): void | Promise<void> { }
  public willAppear(data: EventData): void | Promise<void> { }
  public willDisappear(data: EventData): void | Promise<void> { }
  public sendToPlugin(payload: any): void | Promise<void> { }
  public settingsDidChange(oldSettings: TSettings | null, newSettings: TSettings): void | Promise<void> { }
  public titleParametersDidChange(
    title: string,
    parameters: ESD.TitleParameters,
    extra: EventData,
  ): void | Promise<void> { }
  public propertyInspectorDidAppear(device: ESD.Device): void | Promise<void> {}
  public propertyInspectorDidDisappear(device: ESD.Device): void | Promise<void> {}

  private settings: TSettings;

  protected getSettings(): TSettings {
    return this.settings;
  }

  public async setSettings(settings: TSettings): Promise<void> {
    if (JSON.stringify(this.settings) == JSON.stringify(settings)) {
      return;
    }
    const old = this.settings;
    this.settings = settings;
    await this.settingsDidChange(old, settings);
  }
}

export default StreamDeckAction;
