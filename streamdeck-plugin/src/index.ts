import * as Client from 'StreamingRemoteClient';

async function getOutputs(uri: string, password: string): Promise<{ [id: string]: Client.Output }> {
  const [rpc, ws] = await createClient(uri, password);
  const outputs = await rpc.getOutputs();
  ws.close();
  return outputs;
}

function createClient(uri: string, password: string): Promise<[Client.RPC, WebSocket]> {
  const ws = new WebSocket(uri);
  ws.binaryType = 'arraybuffer';
  return new Promise((resolve, reject) => {
    ws.addEventListener('open', async () => {
      const handshakeState = await Client.handshake(ws, password);
      const rpc = new Client.RPC(ws, handshakeState);
      rpc.onHelloNotification(() => resolve([rpc, ws]));
    });
    ws.addEventListener('close', () => {
      reject();
    });
  });
}

let websocket = null;
let pluginUUID = null;
let images = null;

type Settings = {
  output: Client.Output;
  password: string;
  uri: string;
};

const rpcStore: { [context: string]: Client.RPC } = {};
const outputStore: { [context: string]: Client.Output } = {};
const settingsStore: { [context: string]: Settings } = {};

const myPlugin = {

  type: "com.fredemmott.streamingremote.action",

  onKeyDown: function (_context, _settings, _coordinates, _userDesiredState): void {
  },

  onKeyUp: function (context: string, settings, _coordinates, _userDesiredState): void {
    const rpc = rpcStore[context];
    const state = outputStore[context].state;
    if (state == 'stopped') {
      rpc.startOutput(settings.output);
    } else {
      rpc.stopOutput(settings.output);
    }
  },

  updateImage: async function (context: string, settings): Promise<void> {
    const rpc = rpcStore[context];
    const outputs = await rpc.getOutputs();
    outputStore[context] = outputs[settings.output];
    const { type, state } = outputs[settings.output];
    if (!type) {
      return;
    }
    this.setImage(context, type, state);
  },

  setImage: function (context: string, type, state) {
    var suffix;
    switch (state) {
      case 'stopped':
        suffix = 'Stopped';
        break;
      case 'starting':
      case 'stopping':
        suffix = 'Changing';
        break;
      case 'active':
        suffix = 'Active';
        break;
    }
    const key = (type == 'local_recording' ? 'recording' : 'streaming') + suffix;
    const image = images[key];
    websocket.send(JSON.stringify({
      event: 'setImage',
      context: context,
      payload: { image },
    }));
  },

  onWillAppear: async function (context: string, settings, _coordinates) {
    const oldSettings = settingsStore[context];
    if (oldSettings && settings.output == oldSettings.output && settings.password == oldSettings.password && settings.uri == oldSettings.uri) {
      const output = outputStore[context];
      if (output && output.type && output.state) {
        this.setImage(context, output.type, output.state);
      }
      return;
    }
    settingsStore[context] = settings;
    websocket.send(JSON.stringify({
      event: 'showAlert',
      context: context,
    }));
    if (settings.output) {
      await this.connectRemote(context, settings);
      await this.updateImage(context, settings);
    }
  },

  connectRemote: async function (context: string, settings) {
    const [rpc, ws] = await createClient(settings.uri, settings.password);
    rpcStore[context] = rpc;
    outputStore[context] = undefined;
    ws.addEventListener('close', () => {
      websocket.send(JSON.stringify({
        event: 'showAlert',
        context: context,
      }));
      rpcStore[context] = undefined;
      outputStore[context] = undefined;
    });
    ws.addEventListener('error', () => {
      websocket.send(JSON.stringify({
        event: 'showAlert',
        context: context,
      }));
      rpcStore[context] = undefined;
      outputStore[context] = undefined;
    });
    rpc.onOutputStateChanged((id, state) => {
      outputStore[context].state = state;
      if (id == settings.output) {
        this.setImage(context, outputStore[context].type, state);
      }
    });
  },

  displayAndRetryBadConnections: async function () {
    Object.keys(settingsStore).map(async context => {
      if (rpcStore[context] && outputStore[context]) {
        return;
      }
      websocket.send(JSON.stringify({
        event: 'showAlert',
        context: context,
      }));
      if (rpcStore[context]) {
        return;
      }
      try {
        const settings = settingsStore[context];
        await this.connectRemote(context, settings);
        await this.updateImage(context, settings);
      } catch (e) {
      }
    });
  },

  onSendToPlugin: async function (context, jsonPayload) {
    const event = jsonPayload.event;
    if (event == "getData") {
      const settings = settingsStore[context];
      var outputs;
      try {
        outputs = await getOutputs(settings.uri, settings.password);
      } catch (e) {
        outputs = {};
      }

      const json = {
        event: "sendToPropertyInspector",
        context: context,
        payload: { event: "data", outputs, settings }
      };
      websocket.send(JSON.stringify(json));
      return;
    }
    if (event == 'saveSettings') {
      const settings = jsonPayload.settings;
      if (settingsStore[context] != settings) {
        settingsStore[context] = jsonPayload.settings;
        await this.connectRemote(context, settings);
      }
      websocket.send(JSON.stringify({
        event: 'setSettings',
        context,
        payload: jsonPayload.settings
      }));
      this.updateImage(context, settings);
      var outputs;
      try {
        outputs = await getOutputs(settings.uri, settings.password);
      } catch (e) {
        outputs = {};
      }
      websocket.send(JSON.stringify({
        event: 'sendToPropertyInspector',
        context,
        payload: { event: 'data', outputs, settings }
      }));
    }
  }
};

setInterval(() => myPlugin.displayAndRetryBadConnections(), 1000);

function connectSocket(inPort, inPluginUUID, inRegisterEvent, inInfo) {
  pluginUUID = inPluginUUID

  // Open the web socket
  websocket = new WebSocket("ws://localhost:" + inPort);

  function registerPlugin(inPluginUUID) {
    var json = {
      "event": inRegisterEvent,
      "uuid": inPluginUUID
    };

    websocket.send(JSON.stringify(json));
  };

  websocket.onopen = function () {
    // WebSocket is connected, send message
    registerPlugin(pluginUUID);
  };

  websocket.onmessage = async function (evt) {
    // Received message from Stream Deck
    var jsonObj = JSON.parse(evt.data);
    var event = jsonObj['event'];
    var action = jsonObj['action'];
    var context = jsonObj['context'];
    var jsonPayload = jsonObj['payload'] || {};

    if (event == "keyDown") {
      var settings = jsonPayload['settings'];
      var coordinates = jsonPayload['coordinates'];
      var userDesiredState = jsonPayload['userDesiredState'];
      myPlugin.onKeyDown(context, settings, coordinates, userDesiredState);
    }
    else if (event == "keyUp") {
      var settings = jsonPayload['settings'];
      var coordinates = jsonPayload['coordinates'];
      var userDesiredState = jsonPayload['userDesiredState'];
      myPlugin.onKeyUp(context, settings, coordinates, userDesiredState);
    }
    else if (event == "willAppear") {
      var settings = jsonPayload['settings'];
      var coordinates = jsonPayload['coordinates'];
      myPlugin.onWillAppear(context, settings, coordinates);
    }
    else if (event == "sendToPlugin") {
      myPlugin.onSendToPlugin(context, jsonPayload);

    }
  };

  websocket.onclose = function () {
    // Websocket is closed
  };
};


function loadImageAsDataUri(url: string): Promise<string> {
  return new Promise((resolve, _) => {
    var image = new Image();

    image.onload = function () {
      var canvas = document.createElement("canvas");

      canvas.width = this.naturalWidth;
      canvas.height = this.naturalHeight;

      var ctx = canvas.getContext("2d");
      ctx.drawImage(this, 0, 0);
      resolve(canvas.toDataURL("image/png"));
    }.bind(image);

    image.src = url;
  });
};

async function loadImages() {
  const [
    streamingStopped,
    streamingChanging,
    streamingActive,
    recordingStopped,
    recordingChanging,
    recordingActive
  ] = await Promise.all([
    loadImageAsDataUri('keys/streaming-stopped.png'),
    loadImageAsDataUri('keys/streaming-changing.png'),
    loadImageAsDataUri('keys/streaming-active.png'),
    loadImageAsDataUri('keys/recording-stopped.png'),
    loadImageAsDataUri('keys/recording-changing.png'),
    loadImageAsDataUri('keys/recording-active.png'),
  ]);
  images = {
    streamingStopped,
    streamingChanging,
    streamingActive,
    recordingStopped,
    recordingChanging,
    recordingActive,
  };
}

window['connectSocket'] = connectSocket;
window.addEventListener('load', () => loadImages());
