#!/usr/bin/env python3
import socket

CLIENT = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
CLIENT.settimeout(2)
SERVER = ('127.0.0.1', 8000)

results = []
with open('domain.txt', 'r') as f:
    for line in f:
        domain = line.strip()
        if not domain:
            continue
        CLIENT.sendto(domain.encode(), SERVER)
        try:
            ip = CLIENT.recv(512).decode()
        except socket.timeout:
            ip = 'Not Found'
        results.append(f"{domain}\t{ip}")

with open('result.txt', 'w') as f:
    f.write("\n".join(results))
