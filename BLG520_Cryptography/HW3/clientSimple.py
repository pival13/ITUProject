#! /usr/bin/env python3

import socket
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

from sys import argv, stdin
if __name__ == '__main__':
    if len(argv) != 2: exit(0)
    
    tracemalloc.start()
    
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

    snapshot = tracemalloc.take_snapshot()
    display_top(snapshot)
