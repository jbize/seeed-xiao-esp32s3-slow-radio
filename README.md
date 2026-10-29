# Slow radio check for the Seeed Studio XIAO ESP32S3 Sense

Produced by Godswind Consulting.

Some XIAO ESP32S3 Sense boards transmit very slowly. A good board sends one picture-sized block in about 30 ms. A slow board averages 80 ms or more, and some sends take hundreds of milliseconds. The picture itself is fine. The same slowness shows up with the camera module unplugged, and it shows up in Seeed's own camera sketch. The signal can be strong. The slow step is the Wi-Fi send, so the fault is in the ESP32-S3 Wi-Fi block or in the antenna parts on the Sense board. This test cannot split those two.

Blink, a Wi-Fi scan, joining a network, BLE, and a serial example all succeed on a slow board. None of them writes a picture-sized block out the antenna.

## What the test does

`slow_radio.ino` leaves the camera module unplugged and never starts it. It places one 24 KB block in internal memory, the only memory the Wi-Fi block can transmit from. 24 KB is the size of one of the JPEGs that were slow. The board opens an open Wi-Fi network named `XIAO-RADIO` and writes that same block ten times to whoever connects. The clock covers only that write. After each write the board sends one line with the number of milliseconds it took.

The access point is kept in RAM. The sketch calls `WiFi.persistent(false)` before it starts, so it does not save the network into NVS. A normal upload also leaves the NVS partition in place. Leave "Erase All Flash Before Sketch Upload" off.

`slow_radio.py` connects to `192.168.4.1`, reads the ten blocks, and prints each time. It takes no options and uses only the Python standard library.

## Run it

Open `slow_radio.ino` in the Arduino IDE. Install the ESP32 board package, version 2.0.8 or newer, select the board **XIAO_ESP32S3** and the port, and upload. No extra library is required. Set **USB CDC On Boot** to Enabled if the serial monitor is blank. The monitor at 115200 prints the same times as the script. The script still works if the monitor is blank.

Join the open network `XIAO-RADIO`. Then, from this directory:

```
python3 slow_radio.py
```

## How to read the result

A good board averages 50 ms or less. The boards measured here sit near 30 ms. A bad board averages 80 ms or more. Between 50 ms and 80 ms the script prints `UNCLEAR`.

The next beacon, or another anomaly, can make a single send longer, near 100 ms. That one result is not a fault in the Sense board. Judge the board by the average and by the other nine sends. A slow board is slow across the run.
