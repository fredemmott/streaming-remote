/*
 * Copyright (c) 2019-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import RPCClient from './RPCClient';
import handshake from './handshake';

window['StreamingRemote'] = {
  RPCClient,
  handshake,
};

