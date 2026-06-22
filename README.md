# Kame Remix

Kame Remix is an ESP8266/NodeMCU firmware for driving an 8DOF Kame-style quadruped from a real web remote.

The robot stays in WiFi access point mode. Connect your phone or laptop to the robot, open the control page, then drive it with press-and-hold controls like a small remote-controlled car.

## What This Remix Adds

- WiFi AP mode, no router required.
- Embedded web page served by the robot at `http://192.168.4.1`.
- Touch-friendly directional controls: forward, backward, left, right, stop.
- Speed slider.
- Three gait styles: `Normal`, `Furtif`, and `Bond`.
- Temporary `Turbo` button while a direction is held.
- Safety timeout: if the browser stops sending commands, the robot stops.
- Extra action buttons for rest pose, hello, bow, wiggle, stretch, dance, push-up, moonwalk, patrol, and showtime.
- Servo calibration page with live offsets, reverse toggles, test positions, and EEPROM save.
- Non-blocking firmware loop so the web server keeps listening while the robot moves.

## Remote Functions

- `Normal`: balanced walking.
- `Furtif`: slower and lower gait for careful movement.
- `Bond`: higher, more playful gait.
- `Turbo`: temporarily raises speed to 100% while held.
- `Patrouille`: short autonomous routine that walks and turns.
- `Showtime`: a little combined performance using several poses.
- `Bip`: phone/browser-side beep and vibration only; the robot needs a buzzer if you want real onboard sound.

## Servo Calibration

Open `http://192.168.4.1/calibration` or tap `Calibration` from the remote.

Calibration mode stops the normal movement routines and holds the robot in a neutral pose while you tune each servo.

For every servo `S0` to `S7`, you can:

- adjust the trim offset from `-30` to `+30` degrees;
- reverse the servo direction;
- test positions at `60`, `90`, and `120` degrees;
- return all servos to the neutral pose.

Tap `Sauvegarder` to write the offsets and reverse flags to ESP8266 EEPROM. The saved values are loaded again after reboot.

`Reset live` resets the current calibration values on screen and on the robot, but you still need to tap `Sauvegarder` if you want that reset to persist.

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
5. Tap `Stop` to cancel any autonomous routine.

The `Repos` button only returns the servos to their neutral standing pose. It is not spatial localization; the robot has no sensors here to know where it is in the room.

## Attribution

This remix was created after studying the public Kame project by `p123ad`:

https://github.com/p123ad/kame

The 3D models are not copied here. Use the upstream repository for the original printable parts and mechanical references.
