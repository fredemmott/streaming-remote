/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import { PIEvents, PluginEvents } from "../EventIDs"
import PropertyInspector from "./PropertyInspector"

export default class StartStopActionPI extends PropertyInspector {
  protected readonly outputSelect: HTMLSelectElement;

  constructor(uuid: string, action: string, context: string) {
    super(uuid, action, context);
    this.mainWrapper.classList.add('output-action');

    this.outputSelect = document.getElementById('output') as HTMLSelectElement;

    this.sendToPlugin({event: PIEvents.GetData});
  }

  protected receivedPluginMessage(message: any): void {
    if (message.payload.event != PluginEvents.SetData) {
      return;
    }

    while (this.outputSelect.firstChild) {
      this.outputSelect.removeChild(this.outputSelect.firstChild);
    }

    const { outputs, settings } = message.payload;

    Object.keys(outputs).map(id => {
      const { name, state, type } = outputs[id];
      const option = document.createElement('option');
      option.setAttribute('value', id);
      option.setAttribute('label', name);
      if (id == settings.output) {
        option.setAttribute('selected', 'selected');
      }
      this.outputSelect.appendChild(option);
    });
    this.outputSelect.disabled = false;
    this.uriInput.value = typeof settings.uri == 'string' ? settings.uri : '';
    this.passwordInput.value = typeof settings.password == 'string' ? settings.password : '';

    if (settings.output == "" && this.outputSelect.value != settings.output) {
      this.saveSettings();
    }

    this.mainWrapper.classList.remove('hidden');
  }

  protected saveSettings(): void {
    const settings = {
      uri: this.uriInput.value,
      password: this.passwordInput.value,
      output: this.outputSelect.value,
    };
    super.setSettings(settings);
  }
}
