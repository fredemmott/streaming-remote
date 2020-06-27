/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

// the typescript isn't currently compileable :'(
import * as XJS from "xjs-framework/dist/xjs-es2015"
import * as StreamRemote from './StreamRemote'
import { XSplitPluginDllApiVersion } from './Version';

interface DeferredPromise<T> {
  resolve?: (T) => void;
  reject?: (T) => void;
}

const readyState: DeferredPromise<void> = {};
export const ready: Promise<void> = new Promise((resolve, reject) => {
  readyState.resolve = resolve;
  readyState.reject = reject;
});

function dll_callback(name: string, impl): void {
  // com.fredemmott.js.streaming-remote/js/, but without invalid chars for JS
  // function names
  const plugin_prefix = "com_fredemmott_streamingremote__js__";
  window[`OnDll${plugin_prefix}${name}`] = impl;
}

dll_callback('debugLog', async function(what: string) {
  console.log(`DLL: ${what}`);
});

dll_callback('startOutput', async function (id: string) {
  const outputs = await XJS.Output.getOutputList();
  await Promise.all(
    outputs.map(async output => {
      const name = await output.getName();
      if (name == id) {
        // ordering is important :)
        await StreamRemote.DllCall.outputStateChanged(id, StreamRemote.OutputState.STARTING);
        await output.startBroadcast();
        return;
      }
    })
  );
});

dll_callback('stopOutput', async function (id: string) {
  const outputs = await XJS.Output.getOutputList();
  await Promise.all(
    outputs.map(async output => {
      const name = await output.getName();
      if (name == id) {
        await StreamRemote.DllCall.outputStateChanged(id, StreamRemote.OutputState.STOPPING);
        await output.stopBroadcast();
        return;
      }
    })
  );
});

dll_callback('init', async function (dll_proto: string) {
  console.log("init", {dll_proto, XSplitPluginDllApiVersion});
  if (dll_proto != XSplitPluginDllApiVersion) {
    readyState.reject([dll_proto, XSplitPluginDllApiVersion]);
    return;
  }
  await sendOutputListToDll();
  readyState.resolve(null);
});

async function sendOutputListToDll() {
  await XJS.ready();
  const [allOutputs, activeOutputs, config] = await Promise.all([
    (async () => {
      const outputs = await XJS.Output.getOutputList() as Array<any>;
      return Promise.all(
        outputs.map(async output => {
          const id = await output.getName();
          const name = await (async () => {
            if (id == 'Local Recording') {
              // getDisplayName() raises a typeerror
              return 'Recording';
            }
            if (id == 'XBC_NDIStream') {
              // getDisplayName() returns the ID. Not useful.
              return 'NDI';
            }
            return await output.getDisplayName();
          })();
          let type = StreamRemote.OutputType.REMOTE_STREAM;
          if (id == "Local Recording") {
            type = StreamRemote.OutputType.LOCAL_RECORDING;
          }
          if (id == "XBC_NDIStream") {
            type = StreamRemote.OutputType.LOCAL_STREAM;
          }
          return {
            id,
            name,
            type: type,
            state: StreamRemote.OutputState.STOPPED,
          } as StreamRemote.Output;
        })
      );
    })(),
    XJS.StreamInfo.getActiveStreamChannels(),
    getConfiguration(),
  ]);
  let retOutputs = allOutputs;
  activeOutputs.forEach(active => {
    for (const key in allOutputs) {
      if (allOutputs[key].id == active.name) {
        retOutputs[key].state = StreamRemote.OutputState.ACTIVE;
        break;
      }
    }
  });
  await StreamRemote.DllCall.setConfig(config, retOutputs);
}

async function loadDll(): Promise<void> {
  console.log('Waiting for XJS');
  await XJS.ready();
  console.log('XJS ready');
  const handle = XJS.Dll.load(["ScriptDlls\\Local\\xsplit-streaming-remote.dll"]);
  const haveDll = await XJS.Dll.isAccessGranted();
  if (!haveDll) {
    await new Promise((resolve, _) => {
      XJS.Dll.on('access-granted', resolve);
    });
  }

  try {
    await StreamRemote.DllCall.init(XSplitPluginDllApiVersion);
  } catch (e) {
    document.getElementById('dllNotFound').classList.remove("uninit");
  }
}

export async function start() {
  await XJS.ready();
  await loadDll();
  XJS.ChannelManager.on('stream-start', async function (data) {
    const {error, channel} = data;
    if (error) {
      return;
    }
    const id = await channel.getName();
    await StreamRemote.DllCall.outputStateChanged(id, StreamRemote.OutputState.ACTIVE);
  });
  XJS.ChannelManager.on('stream-end', async function (data) {
    const {error, channel} = data;
    if (error) {
      return;
    }
    const id = await channel.getName();
    await StreamRemote.DllCall.outputStateChanged(id, StreamRemote.OutputState.STOPPED);
  });
}

export async function getConfiguration(): Promise<StreamRemote.Config> {
  const storageKey = await (new XJS.App()).getUserIdHash();
  const storedJson = localStorage.getItem(storageKey);
  if (storedJson === null || storedJson === '') {
    const config = await StreamRemote.DllCall.getDefaultConfiguration();
    // Keep the same password each time
    localStorage.setItem(storageKey, JSON.stringify(config));
    return config;
  }

  return JSON.parse(storedJson) as StreamRemote.Config;
}

export async function setConfiguration(config: StreamRemote.Config): Promise<void> {
  await ready;
  const storageKey = await (new XJS.App()).getUserIdHash();
  localStorage.setItem(storageKey, JSON.stringify(config));
  await StreamRemote.DllCall.setConfiguration(config);
}
