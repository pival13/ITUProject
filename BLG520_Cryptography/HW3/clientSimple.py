#! /usr/bin/env python3

import socket

from sys import argv, stdin
if __name__ == '__main__':
    if len(argv) != 2: exit(0)
    client = socket.create_connection(('0.0.0.0', int(argv[1])))
    address = client.getsockname()
    peer = client.getpeername()
    print(f'Connected to {peer[0]}:{peer[1]} from {address[0]}:{address[1]}')

    while True:
        s = stdin.readline()
        if not s or s == '': break
        sent = client.send(s[:-1].encode('utf8'))
        if sent == 0: break
        msg = client.recv(4096)
        if not msg: break
        print(f'Received "{msg.decode("utf8")}"')