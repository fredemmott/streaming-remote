/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

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
  id: string,
  name: string,
  type: OutputType,
  state: OutputState,
  delaySeconds?: number,
}
