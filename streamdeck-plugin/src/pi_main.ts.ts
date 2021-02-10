/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import { ActionIDs } from "./ActionIDs";
import { Version } from "StreamingRemoteClient";

window['ActionIDs'] = ActionIDs;

window.addEventListener(
  'load',
  () => {
    const link = document.getElementById('plugin-download') as HTMLAnchorElement;
    link.href = 'https://github.com/fredemmott/streaming-remote/releases/v'+Version;
    console.log(link.href);
  }
);
