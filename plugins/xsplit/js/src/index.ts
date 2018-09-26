/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import * as Plugin from './plugin';
import * as ConfigPage from './configpage';

window.addEventListener('load', () => {
  Plugin.start();
  ConfigPage.start();
});
