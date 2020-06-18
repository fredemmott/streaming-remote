/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

function loadImageAsDataUri(url: string): Promise<string> {
  return new Promise((resolve, _) => {
    var image = new Image();

    image.onload = function () {
      var canvas = document.createElement("canvas");

      canvas.width = this.naturalWidth;
      canvas.height = this.naturalHeight;

      var ctx = canvas.getContext("2d");
      ctx.drawImage(this, 0, 0);
      resolve(canvas.toDataURL("image/svg+xml"));
    }.bind(image);

    image.src = url;
  });
};

type Images = { [key: string]: string };

async function loadImagesUncached(): Promise<Images> {
  const [
    streamingStopped,
    streamingChanging,
    streamingActive,
    recordingStopped,
    recordingChanging,
    recordingActive
  ] = await Promise.all([
    loadImageAsDataUri('keys/streaming-stopped.svg'),
    loadImageAsDataUri('keys/streaming-changing.svg'),
    loadImageAsDataUri('keys/streaming-active.svg'),
    loadImageAsDataUri('keys/recording-stopped.svg'),
    loadImageAsDataUri('keys/recording-changing.svg'),
    loadImageAsDataUri('keys/recording-active.svg'),
  ]);
  return {
    streamingStopped,
    streamingChanging,
    streamingActive,
    recordingStopped,
    recordingChanging,
    recordingActive,
  };
}

let cache: Promise<Images> = null;
async function images(): Promise<Images> {
  if (!cache) {
    cache = loadImagesUncached();
  }
  return await cache;
}

export default images;
