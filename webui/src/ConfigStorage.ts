/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import { Config } from "StreamingRemoteClient";

export function getSavedConfigurationNames(): Array<string> {
  const names = [];
  for (let i = 0; i < localStorage.length; ++i) {
    const key = localStorage.key(i);
    if (key.startsWith('config/')) {
      names.push(key.slice(7));
    }
  }

  return names;
}

export function getSavedConfiguration(name: string): Config {
  const json = localStorage.getItem('config/' + name);
  if (json === null) {
    return null;
  }
  return JSON.parse(json);
}

export function saveConfiguration(name: string, config: Config): void {
  try {
    localStorage.setItem('config/' + name, JSON.stringify(config));
  } catch (e) {
    // ignore: incognito mode in mobile safari throws here as there's
    // a 0-byte quote
  }
}
