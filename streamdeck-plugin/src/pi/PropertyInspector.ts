/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

export default abstract class PropertyInspector {
  private readonly uuid: string;
  private readonly action: string;
  protected readonly mainWrapper: HTMLElement;
  protected readonly uriInput: HTMLInputElement;
  protected readonly passwordInput: HTMLInputElement;

  constructor(uuid: string, action: string, context: string, settings: any) {
    this.uuid = uuid;
    this.action = action;

    this.uriInput = document.getElementById('uri') as HTMLInputElement;
    this.passwordInput = document.getElementById('password') as HTMLInputElement;
    if (settings.uri) {
      this.uriInput.value = settings.uri;
    }
    if (settings.password) {
      this.passwordInput.value = settings.password;
    }

    this.mainWrapper = document.getElementById('mainWrapper');
    this.mainWrapper.classList.remove('unknown-action');

    $SD.on('sendToPropertyInspector', this.receivedPluginMessage.bind(this));
    window['saveSettings'] = this.saveSettings.bind(this);
  }

  protected sendToPlugin(message: any) {
    $SD.api.sendToPlugin(this.uuid, this.action, message);
  }

  protected setSettings(settings: any) {
    $SD.api.setSettings(this.uuid, settings);
  }

  protected abstract receivedPluginMessage(message: any): void|Promise<void>;
  protected abstract saveSettings(): void|Promise<void>;
}
