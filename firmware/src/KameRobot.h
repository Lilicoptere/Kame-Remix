#pragma once

#include <Arduino.h>
#include <Servo.h>

enum class DriveMode : uint8_t {
    Stop,
    Forward,
    Backward,
    Left,
    Right,
};

enum class ActionMode : uint8_t {
    None,
    Hello,
    Dance,
    PushUp,
    Moonwalk,
};

class KameRobot {
public:
    void begin();
    void tick();

    void drive(DriveMode mode, uint8_t speed);
    void stop();
    void home();
    void action(ActionMode action);

    DriveMode driveMode() const;
    ActionMode actionMode() const;
    uint8_t speed() const;
    const char* modeName() const;

private:
    void writePose(const float pose[8]);
    void writeServo(uint8_t id, float angle);
    void buildDrivePose(float pose[8], uint32_t now);
    void buildActionPose(float pose[8], uint32_t now);
    void smoothToward(const float target[8]);

    float wave(uint32_t now, float periodMs, float phaseDeg) const;
    int angleToMicros(float angle) const;

    Servo _servo[8];
    float _current[8];
    DriveMode _driveMode = DriveMode::Stop;
    ActionMode _actionMode = ActionMode::None;
    uint8_t _speed = 55;
    uint32_t _modeStartedAt = 0;
    uint32_t _actionStartedAt = 0;
    uint32_t _lastFrameAt = 0;
};
