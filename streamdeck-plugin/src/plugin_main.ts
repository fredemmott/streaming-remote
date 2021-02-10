/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import StreamingRemotePlugin from './StreamingRemotePlugin';

import images from "./Images";

// Prime cache
window.addEventListener('load', () => images());
window['connectElgatoStreamDeckSocket'] = (port, uuid, event, info) => { new StreamingRemotePlugin(port, uuid, event, info) };