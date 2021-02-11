/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import { PIEvents, PluginEvents } from "../EventIDs"
import PropertyInspector from "./PropertyInspector"
import * as Client from "StreamingRemoteClient";

export default class SetCurrentScenePI extends PropertyInspector {
  protected readonly sceneSelect: HTMLSelectElement;
  private scene: { id: string, name: string };
  private scenes: { [id: string]: Client.Scene };

  constructor(uuid: string, action: string, context: string) {
    super(uuid, action, context);
    this.mainWrapper.classList.add('scene-action');

    this.sceneSelect = document.getElementById('scene') as HTMLSelectElement;

    this.sendToPlugin({event: PIEvents.GetData});
  }

  protected receivedPluginMessage(message: any): void {
    if (message.payload.event != PluginEvents.SetData) {
      return;
    }

    while (this.sceneSelect.firstChild) {
      this.sceneSelect.removeChild(this.sceneSelect.firstChild);
    }

    const { scenes, settings } = message.payload;
    this.scene = settings.scene;
    this.scenes = scenes;

    Object.keys(scenes).map(id => {
      const scene = scenes[id];
      const option = document.createElement('option');
      option.setAttribute('value', scene.id);
      option.setAttribute('label', scene.name);
      if (this.scene && scene.id == this.scene.id) {
        option.setAttribute('selected', 'selected');
      }
      this.sceneSelect.appendChild(option);
    });

    this.sceneSelect.disabled = false;
    this.uriInput.value = typeof settings.uri == 'string' ? settings.uri : '';
    this.passwordInput.value = typeof settings.password == 'string' ? settings.password : '';

    if (settings.output == "" && this.sceneSelect.value != settings.scene_id) {
      this.saveSettings();
    }

    this.mainWrapper.classList.remove('hidden');
  }

  protected saveSettings(): void {
    if (this.sceneSelect.value) {
      this.scene = this.scenes[this.sceneSelect.value];
    }
    const settings = {
      uri: this.uriInput.value,
      password: this.passwordInput.value,
      scene: this.scene,
    };
    super.setSettings(settings);
  }
}
