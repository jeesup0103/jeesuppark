#!/usr/bin/env python3
import socket, pathlib
SERVER_ADDR = ("127.0.0.1", 8000)
TIMEOUT_SEC  = 3
def main():
    udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp.settimeout(TIMEOUT_SEC)
    results = []
    for line in pathlib.Path("domain.txt").read_text().splitlines():
        domain = line.strip()
        if not domain:
            continue
        udp.sendto(domain.encode(), SERVER_ADDR)
        try:
            data, _ = udp.recvfrom(512)
            reply = data.decode().strip()
        except socket.timeout:
            reply = "Not Found"
        results.append(f"{domain}\t{reply}")
    pathlib.Path("result.txt").write_text("\n".join(results) + "\n")
if __name__ == "__main__":
    main()
