/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import * as XJS from "xjs-framework/dist/xjs-es2015"
import * as Plugin from './plugin';
import { Config } from './StreamRemote';

export async function start(): Promise<void> {
  await XJS.ready;
  initializeDllWarning();
  setupConfiguration();
  connectConfigurationButtons();
}

async function initializeDllWarning(): Promise<void> {
  const canDll = await XJS.Dll.isAccessGranted();
  const element = document.querySelector('#dllAccessWarning');
  if (canDll) {
    element.classList.add('hide');
  }
  element.classList.remove('uninit');
  XJS.Dll.on('access-granted', () => {
   element.classList.add('hide');
  });
  XJS.Dll.on('access-revoked', () => {
    element.classList.remove('hide');
  });
}

interface ElementsIf {
  password ?: HTMLInputElement;
  tcpPort ?: HTMLInputElement;
  webSocketPort ?: HTMLInputElement;
  namedPipe ?: HTMLInputElement;
  apply ?: HTMLButtonElement;
  reset ?: HTMLButtonElement;
}

const Elements : ElementsIf = { };

window.addEventListener('load', () => {
  const e = Elements;
  const qs = (s) => document.querySelector(s);
  e.password = qs('#password');
  e.tcpPort = qs('#tcpPort');
  e.webSocketPort = qs('#webSocketPort');
  e.namedPipe = qs('#localSocket');
  e.apply = qs('#apply');
  e.reset = qs('#reset');
});

async function setupConfiguration(): Promise<void> {
  const passwordToggle = document.querySelector('#passwordShowHide') as HTMLAnchorElement;
  passwordToggle.addEventListener('click', e => {
    e.preventDefault();
    if (Elements.password.type == "password") {
      Elements.password.type = "text";
      passwordToggle.textContent = '(hide)';
    } else {
      Elements.password.type = "password";
      passwordToggle.textContent = '(reveal)';
    }
  });

  const config = await Plugin.getConfiguration();
  Elements.password.value = config.password;
  Elements.namedPipe.value = config.localSocket;
  Elements.tcpPort.value = config.tcpPort.toString();
  Elements.webSocketPort.value = config.webSocketPort.toString();
  document.querySelector('#configTable').classList.remove('uninit');
}

async function updateButtonState(): Promise<void> {
  const a = await Plugin.getConfiguration();
  const b = getConfigurationFromElements();

  const haveChanges =
    (a.password !== b.password)
    || (a.localSocket !== b.localSocket)
    || (a.tcpPort !== b.tcpPort)
    || (a.webSocketPort !== b.webSocketPort);
  Elements.reset.disabled = !haveChanges;
  Elements.apply.disabled = !haveChanges;
}

function getConfigurationFromElements(): Config {
  const e = Elements;
  return {
    password: e.password.value,
    localSocket: e.namedPipe.value == '' ? null : e.namedPipe.value,
    tcpPort: e.tcpPort.value == null ? null : parseInt(e.tcpPort.value),
    webSocketPort: e.webSocketPort.value == null ? null : parseInt(e.webSocketPort.value),
  };
}

function connectConfigurationButtons(): void {
  const e = Elements;
  e.apply.addEventListener('click', () => {
    Plugin.setConfiguration(getConfigurationFromElements());
    updateButtonState();
  });
  e.reset.addEventListener('click', async () => {
    const config = await Plugin.getConfiguration();
    e.password.value = config.password;
    e.tcpPort.value = config.tcpPort.toString();
    e.webSocketPort.value = config.webSocketPort.toString();
    e.namedPipe.value = config.localSocket;
    updateButtonState();
  });

  e.password.addEventListener('change', () => updateButtonState());
  e.tcpPort.addEventListener('change', () => updateButtonState());
  e.webSocketPort.addEventListener('change', () => updateButtonState());
  e.namedPipe.addEventListener('change', () => updateButtonState());
}
