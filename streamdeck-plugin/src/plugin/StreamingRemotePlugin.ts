/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import * as ESD from "./ESDTypes";
import StreamDeckAction from "./StreamDeckAction";
import StreamDeckPlugin from "./StreamDeckPlugin";

import StartStopOutputAction from "./StartStopOutputAction";

export default class StreamingRemotePlugin extends StreamDeckPlugin {
  public createAction(context: ESD.Context, action: string, websocket: WebSocket): StreamDeckAction<any> {
    if (action != StartStopOutputAction.UUID) {
      console.log(`Unknown action type ${action}`);
      return;
    }
    return new StartStopOutputAction(context, websocket);
  }
}
