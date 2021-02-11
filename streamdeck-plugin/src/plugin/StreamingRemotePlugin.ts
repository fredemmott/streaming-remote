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
import SetCurrentSceneAction from "./SetCurrentSceneAction";

export default class StreamingRemotePlugin extends StreamDeckPlugin {
  public createAction(context: ESD.Context, action: string, websocket: WebSocket): StreamDeckAction<any> {
    switch(action) {
      case StartStopOutputAction.UUID:
        return new StartStopOutputAction(context, websocket);
      case SetCurrentSceneAction.UUID:
        return new SetCurrentSceneAction(context, websocket);
    }
    console.log(`Plugin created with unknown action type ${action}`);
  }
}
