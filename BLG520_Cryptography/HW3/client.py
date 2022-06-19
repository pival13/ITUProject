#! /usr/bin/env python3

from Crypto.Cipher import AES
from Crypto.Signature import PKCS1_v1_5
from Crypto.PublicKey import RSA
from Crypto.Hash import SHA256
from Crypto.Util import number
import socket

from wrapper import TLSWrapper
from modularExpo import modPow

class TLSClient(TLSWrapper):
    def __init__(self):
        super().__init__()
        self.__verifyer = None
        pass

    def start(self, port):
        try:
            self._socket = socket.create_connection(('0.0.0.0', int(port)))
            self._address = self._socket.getsockname()
            self._peer = self._socket.getpeername()
            print(f'Connected to {self._peer[0]}:{self._peer[1]} from {self._address[0]}:{self._address[1]}')

            sent = self._socket.send(b'Client Hello')
            if sent == 0: raise ConnectionError()

            msg = self._socket.recv(4096)
            if not msg: raise ConnectionError()
            keyBeg = msg.find(b'-----BEGIN PUBLIC KEY-----\n')
            keyEnd = msg.find(b'\n-----END PUBLIC KEY-----') + 25
            if keyBeg == -1 or keyEnd == 24: raise ValueError()
            self.__RSAPublic = RSA.importKey(msg[keyBeg:keyEnd])
            self.__verifyer = PKCS1_v1_5.new(self.__RSAPublic)
            self._unsign(msg)

            modulus = number.getPrime(128)
            generator = number.getRandomRange(2, modulus)
            privateKey = number.getRandomInteger(32)
            publicKey = modPow(generator, privateKey, modulus)
            sent = self._socket.send(number.long_to_bytes(modulus,16)+number.long_to_bytes(generator,16)+number.long_to_bytes(publicKey,16))
            if sent == 0: raise ConnectionError()

            msg = self._unsign(self._socket.recv(256))
            if not msg: raise ConnectionError()
            key = number.long_to_bytes(modPow(number.bytes_to_long(msg[:16]), privateKey, modulus), 16)
            ivs = SHA256.new(key).digest()
            self._cipher = AES.new(key, AES.MODE_CBC, ivs[:16])
            self._decipher = AES.new(key, AES.MODE_CBC, ivs[16:])
            
            if self._decrypt(msg[16:]) != b'Server encryption': raise ConnectionError()

            sent = self._socket.send(self._encrypt(b'Client encryption'))
            if sent == 0: raise ConnectionError()
        
        except (ConnectionError,ValueError):
            self.stop()

    def _unsign(self, msg):
        if not self.__verifyer or not msg: return None
        signSize = self.__RSAPublic.size_in_bytes()
        body,signature = msg[:-signSize],msg[-signSize:]
        self.__verifyer.verify(SHA256.new(body), signature)
        return body

    def recv(self):
        msg = self._unsign(self._recv())
        return msg.decode('utf8') if msg else None


from sys import argv, stdin
if __name__ == '__main__':
    if len(argv) != 2: exit(0)
    client = TLSClient()
    client.start(argv[1])

    while client:
        s = stdin.readline()
        if not s or s == '': break
        client.send(s[:-1])
        s = client.recv()
        print(f'Received "{s}"')

    client.stop()