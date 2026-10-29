# Slow radio check for the Seeed Studio XIAO ESP32S3 Sense

Produced by Godswind Consulting.

Some XIAO ESP32S3 Sense boards transmit very slowly. A good board sends one picture-sized block in about 20 to 30 ms, and it stays there on a normal router even when the signal is weak. A slow board still sends most blocks that fast, then stalls again and again. Those stalls last hundreds of milliseconds. The picture itself is fine. The same slowness shows up with the camera module unplugged, and it shows up in Seeed's own camera sketch. It also shows up when the signal is strong, so signal strength is not the fault. The slow step is the Wi-Fi send, so the fault is in the ESP32-S3 Wi-Fi block or in the antenna parts on the Sense board. This test cannot split those two.

Blink, a Wi-Fi scan, joining a network, BLE, and a serial example all succeed on a slow board. None of them writes a picture-sized block out the antenna.

## What the test does

`slow_radio.ino` leaves the camera module unplugged and never starts it. It places one 24 KB block in internal memory, the only memory the Wi-Fi block can transmit from. 24 KB is the size of one of the JPEGs that were slow. The board writes that same block for as long as the computer stays connected. The clock covers only that write. After each write the board sends one line with the number of milliseconds it took.

`kJoinRouter` at the top of the sketch chooses the radio.

`true` joins the network in `secrets.h`. Copy `secrets.h.example` to `secrets.h` and set the name and password. `secrets.h` is in `.gitignore`. This is the test that shows a slow board. The serial monitor prints the address.

`false` opens an open hotspot named `XIAO-RADIO` at `192.168.4.1`. That hotspot can look fast on a slow board, including when the signal is no stronger than the router link that stalls. It is not the test that shows the fault. `secrets.h` still has to be present so the sketch can compile. The password is not used.

Either way the sketch calls `WiFi.persistent(false)` before it starts, so it does not save the network into NVS. A normal upload also leaves the NVS partition in place. Leave "Erase All Flash Before Sketch Upload" off, so a board that is already provisioned keeps those settings.

`slow_radio.py` reads for 10 seconds and prints each time. The result is how many sends took 80 ms or more. It uses only the Python standard library. Ten writes are not enough: on a slow board many writes are still fast, and the stalls show up across the longer run.

## Run it

Copy the example and set the Wi-Fi name and password:

```
cp secrets.h.example secrets.h
```

Open `slow_radio.ino` in the Arduino IDE. Install the ESP32 board package, version 2.0.8 or newer, select the board **XIAO_ESP32S3** and the port, and upload. No extra library is required. Set **USB CDC On Boot** to Enabled if the serial monitor is blank. The monitor at 115200 prints the same times as the script. The script still works if the monitor is blank.

With `kJoinRouter` true, the monitor prints `ready http://…`. From this directory, pass that address:

```
python3 slow_radio.py http://192.168.1.96
```

With `kJoinRouter` false, join the open network `XIAO-RADIO` and run:

```
python3 slow_radio.py
```

## How to read the result

Join a router to judge the board. A good board stays near 20 to 30 ms. One or two sends in the 10 seconds may take around 170 ms. The script still prints `GOOD` for that when the average is 50 ms or less.

A slow board stalls again and again. Many sends take 80 ms or more, and those stalls run to hundreds of milliseconds. The average can stay under 80 ms, because most sends are still fast. The script prints `BAD` when three or more sends took 80 ms or more. With fewer than three long sends and an average between 50 ms and 80 ms, the script prints `UNCLEAR`.

The next beacon, or another anomaly, can make one send longer, around 170 ms. One or two of those are not a fault in the Sense board. A slow board stalls again and again across the run. The hotspot is the wrong place to look for that. A slow board can look fast there.
