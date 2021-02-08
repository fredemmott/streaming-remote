# Binary Handshake Protocol

This protocol provides authentication and initializes stream encryption for later protocols.

1. server confirms the client knows the password (don't allow random people to start broadcasting your webcam to the internet)
2. client confirms the server knows the password (probably not neccessary for this case, but why not)
3. establish a secure client-to-server stream
4. establish a secure server-to-client stream
5. be replay-resistent, both for servers and clients (don't allow someone who sniffed your wifi to start broadcasting your webcam to the internet)

We build this using libsodium secret boxes and secret streams.

All random nonces and keys *must* be re-generated for each connection.

## 1. Client Hello

This is the first message sent.

### SecretBox Key

The pre-shared key is used to send "secret boxes", containing the data that we actually need to share

1. A random `unit8[crypto_pwhash_SALTBYTES]` salt is created
1. A key is created using `crypto_pwhash()`, using the password, salt, default algorithm, and interactive memory and opslimits. If you are working in Javascript, the default
  memory limit requires the 'sumo' variant of libsodium.js
1. The key must be retained to complete the handshake

### Server-To-Client Stream

1. A random `uint8[crypto_secretstream_xchacha20poly1305_KEYBYTES]` key is created using `crypto_secretstream_xchacha20poly1305_keygen()`
1. The key must be retained to complete the handshake

### SecretBox Construction

1. A random `uint8[crypto_secretbox_NONCEBYTES]` nonce is created
1. A secretbox is created using `crypto_secretbox_easy`
   - the plaintext (box contents) is the Server-To-Client stream key
   - the key is the secretbox key created above using `crypto_pwhash`

### Message Construction

The message is a packed binary struct, containing:

```
struct {
  uint8 pwhash_salt[crypto_pwhash_SALTBYTES];
  uint8 secretbox_nonce[crypto_secretbox_NONCEBYTES];
  uint8 secretbox[crypto_secretstream_xchacha20poly1305_KEYBYTES + crypto_secretbox_MACBYTES];
}
```

## 2. Server Hello

This is sent by the server in response to a valid client hello

## SecretBox Key

The key is created in the same manner as the client, except that the client-provided `pwhash_nonce` is used,
instead of a new random value.

## Server-To-Client Stream

1. The key is extracted from the secretbox using the key derived above, and the client-provided nonce
1. A state object and header blob are created using `crypto_secretstream_xchacha20poly1305_init_push` and the key
1. The state object must be retained for use with the RPC protocol

## Client-To-Server Stream

1. A random `uint8[crypto_secretstream_xchacha20poly1305_KEYBYTES]` key is created using `crypto_secretstream_xchacha20poly1305_keygen()`
1. The key must be retained to complete the handshake

### Client Authentication

1. A random `uint8[crypto_auth_KEYBYTES]` key is created using `crypto_auth_keygen()`
1. The key must be retained to complete the handshake

### SecretBox Construction

1. A random `uint8[crypto_secretbox_NONCEBYTES]` nonce is created
1. A secretbox is created using `crypto_secretbox_easy`
    - the plaintext (box contents) is a packed binary struct:
      ```
      struct {
        uint8 client_to_server_key[crypto_secretstream_chacha20poly1305_KEYBYTES];
        uint8 authentication_key[crypto_auth_KEYBYTES];
      }
      ```
    - the key is the secretbox key created above using `crypto_pwhash`

## Message Construction

The message is a packed binary struct, containing:

```
struct {
  uint8 secretbox_nonce[crypto_secretbox_NONCEBYTES];
  uint8 secretbox[
    crypto_secretstream_xchacha20poly1305_KEYBYTES
    + crypto_auth_KEYBYTES
    + crypto_secretbox_MACBYTES
  ];
  uint8 server_to_client_header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
}
```

## 3. Client Ready 

This is sent by the client in response to a valid server hello.

### Client-To-Server Stream

1. The key is extracted from the secretbox using the key derived above, and the server-provided nonce
1. A state object and header blob are created using `crypto_secretstream_xchacha20poly1305_init_push` and the key
1. The state object must be retained for use with the RPC protocol

### Server-To-Client Stream

1. A state object is created from the header blob using the previously established key
   and `crypto_secretstream_xchacha20poly1305_init_pull`
1. The state object must be retained for use with the RPC protocol

### Client Authentication

1. the authentication key is extracted from the secretbox
1. a mac is created using `crypto_auth()` using the provided key, and using the client-to-server header as the message

This is to confirm to the server that the client is actually decrypting the boxes - e.g. that this isn't
a replay attack.

### Message Construction

The message is a packed binary struct, containing:

```
struct {
  uint8 client_to_server_header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
  uint8 authentication_mac[crypto_auth_BYTES];
}
```

### Switching To RPC

- It is now only neccessary to retain the stream state objects, not the keys. Keys should not be retained longer.
- Future client/server messages should follow the RPC protocol.

## Server Handling of Client Hello

1. The authentication mac *must* be verified
1. A state object for the client-to-server stream should be initialized using
   `crypto_secretstream_xchacha20poly1305_secretstream_init_pull()` and the client-provided header
1. The state object must be retained for the RPC protocol
1. It is now only neccessary to retain the stream state objects, not the keys. Keys should not be retained longer.
