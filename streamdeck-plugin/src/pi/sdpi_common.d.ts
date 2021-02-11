/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

declare var $SD : {
	on(method: string, handler: (data: any) => void|Promise<void>): void
	readonly api: {
		sendToPlugin(uuid: string, action: string, message: any);
		setSettings(uuid: string, settings: any);
	};
}
