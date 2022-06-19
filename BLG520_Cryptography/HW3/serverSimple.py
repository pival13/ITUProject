#! /usr/bin/env python3

import socket


if __name__ == '__main__':
    server = socket.create_server(('0.0.0.0', 0))
    address = server.getsockname()
    print(f'Listening on {address[0]}:{address[1]}')
    sock, peer = server.accept()
    print(f'Connected to {peer[0]}:{peer[1]}')

    while True:
        msg = sock.recv(4096)
        if not msg: break
        print(f'Received "{msg.decode("utf8")}", replying with "{msg[::-1].decode("utf8")}"')
        sent = sock.send(msg[::-1])
        if sent == 0: break