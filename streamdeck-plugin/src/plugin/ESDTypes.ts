/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

export type Coordinates = { column: number, row: number };
export type Context = any;
export type Device = any;
export enum DeviceType {
  StreamDeck = 0,
  StreamDeckMini,
  StreamDeckXL,
  StreamDeckMobile,
  CorsairGKeys,
}
export type DeviceInfo = {
  name: string,
  type: DeviceType,
  size: { columns: number, rows: number }
};

export type TitleParameters = {
  fontFamily: string,
  fontSize: number,
  fontStyle: string,
  fontUnderline: boolean,
  showTitle: true,
  titleAlignment: 'top' | 'bottom' | 'middle',
  titleColor: string,
};
