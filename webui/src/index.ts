/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import {Output, OutputState, OutputType} from "StreamingRemoteClient";
import * as Client from "StreamingRemoteClient";
import * as ConfigStorage from "./ConfigStorage";

function handleOutputDrop(e) {
  e.preventDefault();
}

function handleOutputDrag(e) {
  const dragging = e.target;
  const container = document.querySelector('#outputContainer');
  for (let i = 0; i < container.children.length; ++i) {
    const child = container.children[i];
    if (child == dragging) {
      continue;
    }
    if (!(child instanceof HTMLElement)) {
      continue;
    }
    if (child.offsetLeft > e.clientX) {
      container.insertBefore(dragging, child);
      return;
    }
  }
  container.appendChild(dragging);
}

function handleOutputDragOver(e) {
  e.preventDefault();
}

class WebUIClient {
  static allHosts: { [host: string]: boolean } = {};
  private ws: WebSocket;
  private rpc: Client.RPC;
  private config: Client.Config;
  private outputs: { [id: string]: HTMLDivElement } = {};

  public async start(ws: WebSocket, config: Client.Config): Promise<void> {
    this.config = config;
    this.ws = ws;

    let cryptoState : Client.CryptoState = null;
    try {
      cryptoState = await Client.handshake(this.ws, config.password);
    } catch (e) {
      const error = document.querySelector('#error');
      error.classList.remove('uninit');
      if (typeof e == 'string') {
        error.textContent = e;
      } else if (e instanceof Error) {
        error.textContent = e.message;
      }
      document.querySelector('#connectionParameters').classList.remove('hide');
      return;
    }
    this.rpc = new Client.RPC(this.ws, cryptoState);
    this.rpc.onHelloNotification(() => {
      document.querySelector('#connecting').classList.add('hide');
      this.updateOutputs();
    });
    this.rpc.onOutputStateChanged((id: string, state: OutputState) => {
      const element = this.outputs[id];
      this.setOutputElementState(element, state);
    });
  }

  private async updateOutputs(): Promise<void> {
    const outputs = await this.rpc.getOutputs();
    WebUIClient.allHosts[this.config.host] = true;
    if (Object.keys(WebUIClient.allHosts).length > 1) {
      document.querySelector('body').classList.add('multihost');
    }

    const container = document.querySelector('#outputContainer') as HTMLDivElement;
    for (const key in this.outputs) {
      container.removeChild(this.outputs[key]);
    }
    this.outputs = {};

    for (const id in outputs) {
      this.addOutputElement(container, outputs[id]);
    }
    document.querySelector('#outputContainer').classList.remove('uninit');
  }

  private addOutputElement(container: HTMLDivElement, output: Output): void {
    const recordingTemplate = document.querySelector('#recordingTemplate');
    const streamingTemplate = document.querySelector('#streamingTemplate');
    const { id, name, type, state, delaySeconds } = output;
    const template = (type === OutputType.LOCAL_RECORDING)
      ? recordingTemplate : streamingTemplate;
    const elem = template.cloneNode(/* deep = */ true) as HTMLDivElement;
    this.outputs[id] = elem;
    elem.removeAttribute('id');
    elem.dataset.outputId = id;
    elem.dataset.host = this.config.host;
    if (delaySeconds != undefined) {
      const delayMsg = elem.querySelector('.delay') as HTMLSpanElement;
      if (delaySeconds == 0) {
        delayMsg.innerText = 'no delay';
      } else {
        delayMsg.innerText = delaySeconds.toString() + 's delay';
      }
      delayMsg.addEventListener(
        'click',
        async e => {
          e.preventDefault();
          const input = window.prompt(
            "Enter the number of seconds to delay the stream, or '0' to " +
            "disable delay",
            '0'
          );
          const seconds = parseInt(input);
          if (isNaN(seconds) || seconds < 0) {
            return;
          }
          await this.rpc.setDelay(output.id, seconds);
          if (seconds == 0) {
            delayMsg.innerText = 'no delay';
          } else {
            delayMsg.innerText = input + 's delay';
          }
        }
      );
    }
    this.setOutputElementState(elem, state);
    elem.querySelector('.button').addEventListener('click', e => {
      e.preventDefault();
      this.toggleState(id, elem);
    });
    elem.querySelector('h1').textContent = name;
    elem.querySelector('h2').textContent = this.config.host;
    elem.addEventListener('drag', e => handleOutputDrag(e));
    elem.addEventListener('drop', e => handleOutputDrop(e));
    elem.querySelector('.remove').addEventListener('click', e => {
      e.preventDefault();
      elem.classList.add('removed');
    });
    container.appendChild(elem);
  }

  private toggleState(id: string, node: HTMLElement): void {
    if (node.classList.contains('stopped')) {
      this.rpc.startOutput(id);
      return;
    }
    if (node.classList.contains('active')) {
      this.rpc.stopOutput(id);
      return;
    }
  }

  private setOutputElementState(node: HTMLElement, state: OutputState): void {
    const classes = node.classList;
    classes.remove('starting', 'active', 'stopping', 'stopped', 'uninit');
    classes.add(state);

    const label = state === OutputState.ACTIVE ? 'LIVE' : (state.charAt(0).toUpperCase() + state.slice(1));
    node.querySelector('.state').textContent = label;
  }
}

async function connect(config: Client.Config, reused_client: WebUIClient = null) {
  document.querySelector('#connectionParameters').classList.add('hide');
  document.querySelector('#connecting').classList.remove('hide');
  const uri = 'ws://' + config.host + ':' + config.port;
  const ws = new WebSocket(uri);
  ws.binaryType = 'arraybuffer';

  ws.addEventListener('open', async () => {
    const client = reused_client ? reused_client : new WebUIClient();
    await client.start(ws, config);
    ws.addEventListener('close', () => reset(ws, config, client));
  });
  ws.addEventListener('error', function () {
    reset(ws, config, reused_client);
  });
}

function reset(ws: WebSocket, config: Client.Config, reused_client: WebUIClient = null) {
  ws.close();
  setTimeout(function () { connect(config, reused_client); }, 500);
}

function initializeConnectionForm(): void {
  try {
    const saved = ConfigStorage.getSavedConfigurationNames();
    if (saved.length == 0) {
      document.querySelector('#savedConnections').classList.add('hide');
    } else {
      const list = document.querySelector('#savedConnectionsList') as HTMLElement;
      saved.forEach(name => {
        const label = document.createElement('span');
        label.textContent = name;
        const button = document.createElement('button');
        button.textContent = "Connect";
        list.appendChild(label);
        list.appendChild(button);

        button.addEventListener('click', e => {
          e.preventDefault();
          const config = ConfigStorage.getSavedConfiguration(name);
          document.title = name + ' [Stream Remote]';
          connect(config);
        });
      });
    }
  } catch (e) {
    // Safari forbids local storage on file:// URIs
    const toHide = document.querySelectorAll('.needsLocalStorage');
    Array.prototype.forEach.call(toHide, node => node.classList.add('hide'));
  }

  document.querySelector('#connectionParameters').classList.remove('uninit');
  document.querySelector('#connectButton').addEventListener('click', e => {
    e.preventDefault();
    const config = {
      host: (document.querySelector('#host') as HTMLInputElement).value,
      password: (document.querySelector('#password') as HTMLInputElement).value,
      port: parseInt((document.querySelector('#port') as HTMLInputElement).value),
    };
    const name = (document.querySelector('#name') as HTMLInputElement).value;
    if (name !== '') {
      document.title = name + ' [Stream Remote]';
      ConfigStorage.saveConfiguration(name, config);
    }
    connect(config);
  });
}

window.addEventListener('load', async function () {
  initializeConnectionForm();
  document.querySelector('#outputContainer').addEventListener('dragover', e => handleOutputDragOver(e));
  document.querySelector('#addConnection').addEventListener('click', e => {
    this.document.querySelector('#connectionParameters').classList.remove('hide');
  });
});
