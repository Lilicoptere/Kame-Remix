# Kame Remix

Kame Remix is an ESP8266/NodeMCU firmware for driving an 8DOF Kame-style quadruped from a real web remote.

The robot stays in WiFi access point mode. Connect your phone or laptop to the robot, open the control page, then drive it with press-and-hold controls like a small remote-controlled car.

## What This Remix Adds

- WiFi AP mode, no router required.
- Embedded web page served by the robot at `http://192.168.4.1`.
- Touch-friendly directional controls: forward, backward, left, right, stop.
- Speed slider.
- Safety timeout: if the browser stops sending commands, the robot stops.
- Extra action buttons for home, hello, dance, push-up, and moonwalk.
- Non-blocking firmware loop so the web server keeps listening while the robot moves.

## Hardware Target

The firmware targets an ESP8266 NodeMCU board with 8 hobby servos.

Default servo pin map:

```cpp
S0 -> D1
S1 -> D4
S2 -> D8
S3 -> D6
S4 -> D7
S5 -> D5
S6 -> D2
S7 -> D3
```

You can change pins, servo trims, and reversed servos in `firmware/src/KameRobot.cpp`.

## WiFi

Default AP settings:

- SSID: `Kame-Remix`
- Password: `a1b2c3d4`
- URL: `http://192.168.4.1`

Edit these values in `firmware/src/main.cpp` if you want to keep the original `kame` SSID.

## Build And Upload

Install PlatformIO, then run:

```sh
cd firmware
pio run
pio run -t upload
```

After upload:

1. Power the robot.
2. Connect to the `Kame-Remix` WiFi network.
3. Open `http://192.168.4.1`.
4. Hold a direction button to move; release to stop.

## Attribution

This remix was created after studying the public Kame project by `p123ad`:

https://github.com/p123ad/kame

The 3D models are not copied here. Use the upstream repository for the original printable parts and mechanical references.
