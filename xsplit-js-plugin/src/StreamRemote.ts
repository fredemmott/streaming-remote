/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import * as XJS from "xjs-framework/dist/xjs-es2015"

function cpp_fun(name: string): string {
  return `com.fredemmott.streaming-remote/cpp/${name}`;
}

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

export interface Scene {
  id: string;
  name: string;
  active: boolean;
}

export interface Config {
  password: string;
  tcpPort: Number;
  webSocketPort: Number;
}

export namespace DllCall {
  export async function init(proto_version: string): Promise<void> {
    await XJS.Dll.callEx(cpp_fun('init'), proto_version);
  }
  export async function setConfig(
    config: Config,
    outputs: Array<Output>,
    scenes: Array<Scene>,
  ): Promise<void> {
    await XJS.Dll.callEx(
      cpp_fun('setConfig'),
      JSON.stringify(config),
      JSON.stringify(outputs),
      JSON.stringify(scenes),
    );
  }

  export async function getDefaultConfiguration(): Promise<Config> {
    const json = await XJS.Dll.callEx(
      cpp_fun('getDefaultConfiguration'),
    );
    const config = JSON.parse(json) as Config;
    if (config.password == '') {
      config.password = null;
    }
    return config;
  }

  export async function outputStateChanged(id: string, state: OutputState): Promise<void> {
    await XJS.Dll.callEx(cpp_fun('outputStateChanged'), id, state);
  }

  export async function setConfiguration(config: Config): Promise<void> {
    const json = JSON.stringify(config);
    console.log('setting config', config);
    await XJS.Dll.callEx(cpp_fun('setConfiguration'), json);
  }

  export async function currentSceneChanged(id: string): Promise<void> {
    await XJS.Dll.callEx(cpp_fun('currentSceneChanged'), id);
  }

  export async function returnValue(call_id: string, value: Object): Promise<void> {
    await XJS.Dll.callEx(cpp_fun('returnValue'), JSON.stringify({call_id, value}));
  }
}
