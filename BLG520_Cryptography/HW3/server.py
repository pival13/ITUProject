#! /usr/bin/env python3

from Crypto.Cipher import AES
from Crypto.Signature import PKCS1_v1_5
from Crypto.PublicKey import RSA
from Crypto.Hash import SHA256
from Crypto.Util import number
import socket

from wrapper import TLSWrapper
from modularExpo import modPow

import linecache
import os
import tracemalloc

def display_top(snapshot, key_type='filename', limit=3):
    snapshot = snapshot.filter_traces((
        tracemalloc.Filter(False, "<frozen importlib._bootstrap>"),
        tracemalloc.Filter(False, "<frozen importlib._bootstrap_external>"),
        tracemalloc.Filter(False, "<unknown>"),
    ))
    top_stats = snapshot.statistics(key_type)

    print("Top %s lines" % limit)
    for index, stat in enumerate(top_stats[:limit], 1):
        frame = stat.traceback[0]
        # replace "/path/to/module/file.py" with "module/file.py"
        filename = os.sep.join(frame.filename.split(os.sep)[-2:])
        print("#%s: %s:%s: %.1f KiB"
              % (index, filename, frame.lineno, stat.size / 1024))
        line = linecache.getline(frame.filename, frame.lineno).strip()
        if line:
            print('    %s' % line)

    other = top_stats[limit:]
    if other:
        size = sum(stat.size for stat in other)
        print("%s other: %.1f KiB" % (len(other), size / 1024))
    total = sum(stat.size for stat in top_stats)
    print("Total allocated size: %.1f KiB" % (total / 1024))

class TLSServer(TLSWrapper):
    def __init__(self) -> None:
        super().__init__()
        self._server = socket.create_server(('0.0.0.0', 0))
        self._address = self._server.getsockname()
        key = RSA.generate(1024)
        self.__RSAPublic = key.publickey().exportKey()
        self.__signer = PKCS1_v1_5.new(key)

    def start(self):
        try:
            print(f'Listening on {self._address[0]}:{self._address[1]}')
            self._socket, self._peer = self._server.accept()
            print(f'Connected to {self._peer[0]}:{self._peer[1]}')

            if self._socket.recv(16) != b'Client Hello': raise ConnectionError()

            sent = self._socket.send(self._sign(self.__RSAPublic))
            if sent == 0: raise ConnectionError()

            msg = self._socket.recv(64)
            if not msg: raise ConnectionError()
            modulus = number.bytes_to_long(msg[:16])
            generator = number.bytes_to_long(msg[16:32])
            privateKey = number.getRandomInteger(32)
            publicKey = modPow(generator, privateKey, modulus)
            
            key = number.long_to_bytes(modPow(number.bytes_to_long(msg[32:]), privateKey, modulus), 16)
            ivs = SHA256.new(key).digest()
            self._decipher = AES.new(key, AES.MODE_CBC, ivs[:16])
            self._cipher = AES.new(key, AES.MODE_CBC, ivs[16:])

            sent = self._socket.send(self._sign(number.long_to_bytes(publicKey,16) + self._encrypt(b'Server encryption')))
            if sent == 0: raise ConnectionError()

            msg = self._socket.recv(32)
            if not msg or self._decrypt(msg) != b'Client encryption': raise ConnectionError()

        except (ConnectionError,ValueError):
            self.stop()

    def _sign(self, data):
        return data + self.__signer.sign(SHA256.new(data))

    def send(self, msg):
        if msg: self._send(self._sign(msg.encode('utf8')))


if __name__ == '__main__':
    tracemalloc.start()

    server = TLSServer()
    server.start()

    while server:
        s = server.recv()
        if s: print(f'Received "{s}", replying with "{s[::-1]}"')
        server.send(s[::-1] if s else None)
    
    snapshot = tracemalloc.take_snapshot()
    display_top(snapshot)
