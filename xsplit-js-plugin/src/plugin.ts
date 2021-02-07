/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

// the typescript isn't currently compileable :'(
import { getPositionOfLineAndCharacter } from "typescript";
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
  window[`OnDll${plugin_prefix}${name}`] = (...args) => impl(...args);
}

function dll_function(name: string, impl): void {
  dll_callback(
    name,
    async (call_id: string, ...args: Array<string>) => {
      const result = await impl(...args);
      await StreamRemote.DllCall.returnValue(call_id, result);
    }
  );
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

dll_function('activateScene', async function(id: string) {
  const scene = await XJS.Scene.getBySceneUid(id);
  if (!scene) {
    return false;
  }
  if (scene == await XJS.Scene.getActiveScene()) {
    return false;
  }
  await XJS.Scene.setActiveScene(scene);
  return true;
});

async function get_scenes(): Promise<Array<StreamRemote.Scene>> {
  const [count, active] = await Promise.all([
    XJS.Scene.getSceneCount(),
    XJS.Scene.getActiveScene(),
  ]);
  const scenes = await Promise.all([...Array(count).keys()].map(
    (i) => XJS.Scene.getBySceneIndex(i)
  ));
  const active_id = await active.getSceneUid();
  return await Promise.all(scenes.map(
    async (scene) => { return {
      id: await scene.getSceneUid(),
      name: await scene.getName(),
      active: active_id == await scene.getSceneUid(),
  }; }));
}

dll_function('getScenes', get_scenes);

async function get_outputs(): Promise<Array<StreamRemote.Output>> {
  const [active_names, outputs] = await Promise.all([
    (async () => {
      const infos = await XJS.StreamInfo.getActiveStreamChannels();
      return Promise.all(infos.map(info => info.getName()));
    })(),
    XJS.Output.getOutputList(),
  ]);

  return await Promise.all(outputs.map(async output => {
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
    let state = StreamRemote.OutputState.STOPPED;
    active_names.forEach(active => {
      if (active == id) {
        state = StreamRemote.OutputState.ACTIVE;
      }
    });
    return {id, name, type, state} as StreamRemote.Output;
  }));
}

dll_function('getOutputs', get_outputs);

dll_function('getSceneThumbnailAsBase64Png', async (uid: string) => {
  const scene = await XJS.Scene.getBySceneUid(uid);
  return await XJS.Thumbnail.getSceneThumbnail(scene);
});

dll_callback('init', async function (dll_proto: string) {
  console.log("init", {dll_proto, XSplitPluginDllApiVersion});
  if (dll_proto != XSplitPluginDllApiVersion) {
    readyState.reject([dll_proto, XSplitPluginDllApiVersion]);
    return;
  }
  await XJS.ready();

  await StreamRemote.DllCall.setConfiguration(await getConfiguration());
  readyState.resolve(null);
});

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
  console.log('waiting for handle');
  await handle;
  console.log('calling init');
  try {
    await StreamRemote.DllCall.init(XSplitPluginDllApiVersion);
    console.log('init succeeded');
  } catch (e) {
    console.log(e);
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
  XJS.ExtensionWindow.on('scene-load', async function(number) {
    const scene = await XJS.Scene.getBySceneIndex(number);
    const id = await scene.getSceneUid();
    await StreamRemote.DllCall.currentSceneChanged(id);
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
