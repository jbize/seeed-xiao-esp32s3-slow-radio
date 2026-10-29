#!/usr/bin/env python3
"""Receive 24 KB sends from a XIAO board for 10 seconds and print the average.

Produced by Godswind Consulting.

When the sketch joins a router, pass the address it prints on the serial port
after it starts. The address in this example is only an example:

    python3 slow_radio.py http://192.168.1.96

When the sketch opens the XIAO-RADIO hotspot, run it with no address.
The board is then at 192.168.4.1.

Each printed time is how long that board took
to write one 24 KB block out its antenna. The bytes were already in internal
memory, and the camera module is not used.

A good board sends each block in about 20 to 30 ms. One or two sends in a
10 second run may take around 170 ms. That is still a good board.
A bad board has three or more sends of 80 ms or more in those 10 seconds.
Most of its other sends are still fast, so the average can stay under 80 ms.
"""

import socket
import statistics
import sys
import time

HOST = sys.argv[1] if len(sys.argv) > 1 else "192.168.4.1"
if "://" in HOST:
    HOST = HOST.split("://", 1)[1]
HOST = HOST.split("/", 1)[0]
if ":" in HOST:
    HOST = HOST.rsplit(":", 1)[0]
PORT = 80
BYTES = 24 * 1024
SECONDS = 10


def read_exact(sock, size):
    got = b""
    while len(got) < size:
        chunk = sock.recv(size - len(got))
        if not chunk:
            return None
        got += chunk
    return got


def read_line(sock):
    line = b""
    while not line.endswith(b"\n"):
        byte = sock.recv(1)
        if not byte:
            return None
        line += byte
    return line.strip()


def main():
    print(f"Reading {BYTES // 1024} KB sends from {HOST} for {SECONDS} seconds")
    print("Each time is the board's Wi-Fi write. The result is how many sends took 80 ms or more.")
    try:
        sock = socket.create_connection((HOST, PORT), timeout=10)
    except OSError:
        raise SystemExit("Could not connect. Pass the address the sketch prints on the serial port.")
    times = []
    try:
        sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        deadline = time.monotonic() + SECONDS
        n = 0
        while time.monotonic() < deadline:
            sock.settimeout(deadline - time.monotonic())
            try:
                payload = read_exact(sock, BYTES)
                line = read_line(sock) if payload is not None else None
            except (TimeoutError, socket.timeout):
                break
            if payload is None or line is None:
                break
            n += 1
            ms = int(line)
            times.append(ms)
            print(f"send {n:2d}: {ms:4d} ms")
    finally:
        sock.close()

    if not times:
        raise SystemExit("No send finished.")
    average = statistics.mean(times)
    long_sends = sum(ms >= 80 for ms in times)
    print()
    print(f"fastest {min(times)} ms, average {average:.0f} ms, slowest {max(times)} ms")
    print(f"{long_sends} sends took 80 ms or more")
    if long_sends >= 3:
        print("BAD. Three or more sends took 80 ms or more.")
    elif average <= 50:
        print("GOOD. One or two sends around 170 ms are normal.")
    else:
        print("UNCLEAR. Fewer than three long sends, and the average is between 50 ms and 80 ms.")


if __name__ == "__main__":
    main()
