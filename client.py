#!/usr/bin/env python3
import socket

CLIENT = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
CLIENT.settimeout(2)
SERVER = ('127.0.0.1', 8000)

out = []
for line in open('domain.txt'):
    a = line.split()
    scenario = f"{a[0]} {a[1]}"
    domain   = a[2]
    CLIENT.sendto(domain.encode(), SERVER)
    try:
        ip = CLIENT.recv(512).decode()
    except socket.timeout:
        ip = 'Not Found'
    out.append(f"{scenario}\t{ip}")

open('result.txt','w').write('\n'.join(out))
