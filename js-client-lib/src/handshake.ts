/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

import * as sodium from 'libsodium-wrappers-sumo';
import CryptoState from './CryptoState';

interface ClientHelloState {
  psk: Uint8Array;
  pullKey: Uint8Array;
}

function sendClientHello(ws: WebSocket, password: string): ClientHelloState {
  const pwhashSalt = sodium.randombytes_buf(sodium.crypto_pwhash_SALTBYTES);
  const boxNonce = sodium.randombytes_buf(sodium.crypto_secretbox_NONCEBYTES);

  const psk = sodium.crypto_pwhash(
    sodium.crypto_secretbox_KEYBYTES,
    password,
    pwhashSalt,
    sodium.crypto_pwhash_OPSLIMIT_INTERACTIVE,
    sodium.crypto_pwhash_MEMLIMIT_INTERACTIVE,
    sodium.crypto_pwhash_ALG_DEFAULT
  );

  const pullKey = sodium.crypto_secretstream_xchacha20poly1305_keygen();
  const box = pullKey;

  const secretBox = sodium.crypto_secretbox_easy(
    box,
    boxNonce,
    psk,
  );

  const msg = new Uint8Array(
    pwhashSalt.length + boxNonce.length + secretBox.length
  );
  msg.set(pwhashSalt, 0);
  msg.set(boxNonce, pwhashSalt.length);
  msg.set(secretBox, pwhashSalt.length + boxNonce.length);
  ws.send(msg);

  return { psk, pullKey };
}

async function sendClientReady(
  ws: WebSocket,
  state: ClientHelloState,
  e: MessageEvent
): Promise<CryptoState> {
  // Message unpacking
  const message = e.data;
  const secretBoxNonce = new Uint8Array(message, 0, sodium.crypto_secretbox_NONCEBYTES);
  const secretBox = new Uint8Array(
    message,
    secretBoxNonce.length,
    sodium.crypto_secretstream_xchacha20poly1305_KEYBYTES
    + sodium.crypto_auth_KEYBYTES
    + sodium.crypto_secretbox_MACBYTES,
  );
  const pullHeader = new Uint8Array(
    message,
    secretBoxNonce.length + secretBox.length,
  );

  const box = sodium.crypto_secretbox_open_easy(
    secretBox,
    secretBoxNonce,
    state.psk
  );

  // Client-To-Server ('push') stream
  const pushKey = box.slice(0, sodium.crypto_secretstream_xchacha20poly1305_KEYBYTES);
  const authenticationKey = box.slice(pushKey.length);

  const pushResult = sodium.crypto_secretstream_xchacha20poly1305_init_push(
    pushKey,
  );
  const pushState = pushResult.state;
  const pushHeader = pushResult.header;

  // Server-To-Client ('pull') stream
  const pullState =
    sodium.crypto_secretstream_xchacha20poly1305_init_pull(
      pullHeader,
      state.pullKey,
    );

  // Client Authentication
  const mac = sodium.crypto_auth(pushHeader, authenticationKey);

  const response = new Uint8Array(pushHeader.length + mac.length);
  response.set(pushHeader, 0);
  response.set(mac, pushHeader.length);
  ws.send(response);
  return { pushState, pullState };
}

export default async function handshake(
  ws: WebSocket,
  password: string,
): Promise<CryptoState> {
  await sodium.ready;
  let failServerHello: () => void = null;
  const serverHelloHandle = new Promise<MessageEvent>((resolve, reject) => {
    failServerHello = () => reject("Handshake failed - bad password?");
    ws.addEventListener('message', function _tmp(e) {
      ws.removeEventListener('message', _tmp);
      resolve(e);
    });
  });

  const clientHelloState = sendClientHello(ws, password);
  ws.addEventListener('close', failServerHello);
  const serverHello = await serverHelloHandle;
  ws.removeEventListener('close', failServerHello);
  const result = await sendClientReady(ws, clientHelloState, serverHello);
  return result;
}
