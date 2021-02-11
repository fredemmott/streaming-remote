/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import { EventData } from "./StreamDeckAction";
import * as Client from "StreamingRemoteClient";
import {StreamingRemoteClientActionSettings as BaseSettings, StreamingRemoteClientAction} from "./StreamingRemoteClientAction";
import { ActionIDs } from "../ActionIDs";
import { PIEvents, PluginEvents } from "../EventIDs";

interface SetCurrentSceneSettings extends BaseSettings {
  scene: { id: string, label: string },
};

export class SetCurrentSceneAction extends StreamingRemoteClientAction<SetCurrentSceneSettings> {
  public static readonly UUID = ActionIDs.SetCurrentScene;

  private sceneID: string;

  public async keyUp(_data: EventData): Promise<void> {
    await this.rpc.activateScene(this.sceneID);
  }

  protected async onConnect(): Promise<void> {
    this.rpc.onSceneChanged(
      (id: string) => {
        if (id != this.sceneID) {
          return;
        }
        this.sendData();
      }
    )
    this.sendData();
  }

  protected async onWebSocketClose(): Promise<void> {
  }

  protected async onWebSocketError(): Promise<void> {
    this.onWebSocketClose();
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

  public async settingsDidChange(old: SetCurrentSceneSettings, settings: SetCurrentSceneSettings) {
    await super.settingsDidChange(old, settings);
    this.sceneID = settings.scene ? settings.scene.id : null;
    await this.sendData();
  }

  private async sendData(): Promise<void> {
    const scenes = this.rpc ? await this.rpc.getScenes() : {};
    this.websocket.send(JSON.stringify({
      event: 'sendToPropertyInspector',
      context: this.context,
      payload: { event: PluginEvents.SetData, scenes, settings: this.getSettings() }
    }));

  }
}

export default SetCurrentSceneAction;
