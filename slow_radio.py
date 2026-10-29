#!/usr/bin/env python3
"""Receive ten 24 KB sends from a XIAO board and print how long each write took.

Produced by Godswind Consulting.

Join the open Wi-Fi network XIAO-RADIO first. It has no password.
Then run this file with no options:

    python3 slow_radio.py

The board is at 192.168.4.1. Each printed time is how long that board took
to write one 24 KB block out its antenna. The bytes were already in internal
memory, and the camera module is not used.

A good board averages 50 ms or less, about 30 ms on the boards measured here.
A bad board averages 80 ms or more.
"""

import socket
import statistics

HOST = "192.168.4.1"
PORT = 80
BYTES = 24 * 1024
SENDS = 10


def read_exact(sock, size):
    got = b""
    while len(got) < size:
        chunk = sock.recv(size - len(got))
        if not chunk:
            raise SystemExit("The board closed the connection during a send.")
        got += chunk
    return got


def read_line(sock):
    line = b""
    while not line.endswith(b"\n"):
        byte = sock.recv(1)
        if not byte:
            raise SystemExit("The board closed the connection before the send time.")
        line += byte
    return line.strip()


def main():
    print(f"Reading {SENDS} sends of {BYTES // 1024} KB from {HOST}")
    print("Each time is the board's Wi-Fi write.")
    try:
        sock = socket.create_connection((HOST, PORT), timeout=10)
    except OSError:
        raise SystemExit("Could not connect. Join the Wi-Fi network XIAO-RADIO, then run this again.")
    times = []
    try:
        sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        sock.settimeout(30)
        for n in range(1, SENDS + 1):
            read_exact(sock, BYTES)
            ms = int(read_line(sock))
            times.append(ms)
            print(f"send {n:2d}: {ms:4d} ms")
    finally:
        sock.close()

    average = statistics.mean(times)
    print()
    print(f"fastest {min(times)} ms, average {average:.0f} ms, slowest {max(times)} ms")
    if average >= 80:
        print("BAD. This board is slow sending 24 KB out the antenna.")
    elif average <= 50:
        print("GOOD. This board sends 24 KB in 50 ms or less.")
    else:
        print("UNCLEAR. The average is between 50 ms and 80 ms.")


if __name__ == "__main__":
    main()
