/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import { ActionIDs } from "./ActionIDs"
import { Version } from "StreamingRemoteClient"
import StartStopActionPI from "./pi/StartStopActionPI"
import SetCurrentSceneActionPI from "./pi/SetCurrentSceneActionPI"

$SD.on('connected', function (jsonObj) {
  const { uuid , actionInfo } = jsonObj;
  const { action, context } = actionInfo;

  if (action == ActionIDs.StartStopOutput) {
    new StartStopActionPI(uuid, action, context);
    return;
  }

  if (action == ActionIDs.SetCurrentScene) {
    new SetCurrentSceneActionPI(uuid, action, context);
    return;
  }

  console.log('PI connected with unknown action type: ' + action);
});

window.addEventListener(
  'load',
  () => {
    const link = document.getElementById('plugin-download') as HTMLAnchorElement;
    link.href = `https://github.com/fredemmott/streaming-remote/releases/v${Version}`;
  }
);
