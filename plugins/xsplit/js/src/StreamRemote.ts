/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import * as XJS from "xjs-framework/dist/xjs-es2015"

export enum OutputType {
  UNKNOWN = 'unknown',
  LOCAL_RECORDING = 'local_recording',
  LOCAL_STREAM = 'local_stream',
  REMOTE_STREAM = 'remote_stream',
}

export enum OutputState {
  UNKNOWN = 'unknown',
  STARTING = 'starting',
  ACTIVE = 'active',
  STOPPING = 'stopping',
  STOPPED = 'stopped',
}

export interface Output {
  id: string;
  name: string;
  type: OutputType;
  state: OutputState;
}

export interface Config {
  password: string;
  localSocket: string;
  tcpPort: Number;
  webSocketPort: Number;
}

export namespace DllCall {
  export async function init(
    config: Config,
    outputs: Array<Output>,
  ): Promise<void> {
    await XJS.Dll.callEx(
      'StreamingRemote.init',
      JSON.stringify(config),
      JSON.stringify(outputs),
    );
  }

  export async function getDefaultConfiguration(): Promise<Config> {
    const json = await XJS.Dll.callEx(
      'StreamingRemote.getDefaultConfiguration'
    );
    const config = JSON.parse(json) as Config;
    if (config.password == '') {
      config.password = null;
    }
    if (config.localSocket == '') {
      config.localSocket = null;
    }
    return config;
  }

  export async function outputStateChanged(id: string, state: OutputState): Promise<void> {
    await XJS.Dll.callEx('StreamingRemote.outputStateChanged', id, state);
  }

  export async function setConfiguration(config: Config): Promise<void> {
    const json = JSON.stringify(config);
    console.log('setting config', config);
    await XJS.Dll.callEx('StreamingRemote.setConfiguration', json);
  }
}
