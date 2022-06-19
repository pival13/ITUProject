#! /usr/bin/env python3

from Crypto.Cipher import AES
from Crypto.Util import Padding


class TLSWrapper:
    def __init__(self):
        self._socket = None
        self._peer = None
        pass

    def __bool__(self):
        return self._socket != None

    def stop(self):
        if self._socket:
            self._socket.close()
        self._socket = None
        if self._peer:
            print(f'Disconnected from {self._peer[0]}:{self._peer[1]}')
        self._peer = None
        self._cipher = None
        self._decipher = None

    def _encrypt(self, data):
        if not self._cipher: return None
        return self._cipher.encrypt(Padding.pad(data, AES.block_size))
    
    def _decrypt(self, data):
        if not self._decipher: return None
        return Padding.unpad(self._decipher.decrypt(data), AES.block_size)

    def _recv(self):
        if not self._socket: return None
        msg = self._socket.recv(4096)
        if not msg:
            self.stop()
            return None
        dec = self._decrypt(msg)
        if not dec:
            self.stop()
            return None
        return dec

    def recv(self):
        msg = self._recv()
        return msg.decode('utf8') if msg else None

    def _send(self, msg):
        if not self._socket or not msg: return
        enc = self._encrypt(msg)
        if not enc:
            self.stop()
            return
        sent = self._socket.send(enc)
        if sent == 0:
            self.stop()
            return

    def send(self, msg):
        if msg: self._send(msg.encode('utf8'))