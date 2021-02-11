/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import { ActionIDs } from "./ActionIDs";
import { Version } from "StreamingRemoteClient";

declare var $SD : {
  on(method: string, handler: (data: any) => void|Promise<void>): void
  readonly api: {
    sendToPlugin(uuid: string, action: string, message: Message);
    setSettings(uuid: string, settings: any);
  };
}

interface Message {
  event: string;
  payload?: any;
}

abstract class PropertyInspector {
  private readonly uuid: string;
  private readonly action: string;
  protected readonly mainWrapper: HTMLElement;
  protected readonly uriInput: HTMLInputElement;
  protected readonly passwordInput: HTMLInputElement;

  constructor(uuid: string, action: string, context: string) {
    this.uuid = uuid;
    this.action = action;

    this.uriInput = document.getElementById('uri') as HTMLInputElement;
    this.passwordInput = document.getElementById('password') as HTMLInputElement;

    this.mainWrapper = document.getElementById('mainWrapper');
    this.mainWrapper.classList.remove('unknown-action');

    $SD.on('sendToPropertyInspector', this.receivedPluginMessage.bind(this));
    window['saveSettings'] = this.saveSettings.bind(this);
  }

  protected sendToPlugin(message: Message) {
    $SD.api.sendToPlugin(this.uuid, this.action, message);
  }

  protected setSettings(settings: any) {
    $SD.api.setSettings(this.uuid, settings);
  }

  protected abstract receivedPluginMessage(message: Message): void|Promise<void>;
  protected abstract saveSettings(): void|Promise<void>;
}

class StartStopActionPI extends PropertyInspector {
  protected readonly outputSelect: HTMLSelectElement;

  constructor(uuid: string, action: string, context: string) {
    super(uuid, action, context);
    this.mainWrapper.classList.add('output-action');

    this.outputSelect = document.getElementById('output') as HTMLSelectElement;

    this.sendToPlugin({event: 'getData'});
  }

  protected receivedPluginMessage(message: Message): void {
    const {event, payload} = message;
    if (event != 'startStopOutputData') {
      return;
    }

    while (this.outputSelect.firstChild) {
      this.outputSelect.removeChild(this.outputSelect.firstChild);
    }

    const { outputs, settings } = payload;

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

$SD.on('connected', function (jsonObj) {
  const { uuid , actionInfo } = jsonObj;
  const { action, context } = actionInfo;
  if (action == ActionIDs.StartStopOutput) {
    new StartStopActionPI(uuid, action, context);
    return;
  }

  console.log('Unknown action type: ' + action);
});

window.addEventListener(
  'load',
  () => {
    const link = document.getElementById('plugin-download') as HTMLAnchorElement;
    link.href = 'https://github.com/fredemmott/streaming-remote/releases/v'+Version;
  }
);
