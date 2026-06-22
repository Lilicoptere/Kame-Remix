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
    Bow,
    Wiggle,
    Stretch,
    Patrol,
    ShowOff,
};

enum class GaitStyle : uint8_t {
    Normal,
    Sneak,
    Bounce,
};

class KameRobot {
public:
    static constexpr uint8_t ServoCount = 8;

    void begin();
    void tick();

    void drive(DriveMode mode, uint8_t speed);
    void stop();
    void home();
    void action(ActionMode action);
    void setGaitStyle(GaitStyle style);

    void enterCalibration();
    void exitCalibration();
    void calibrationNeutral();
    void setCalibrationServo(uint8_t id, float angle);
    void setTrim(uint8_t id, int8_t trim);
    void setReversed(uint8_t id, bool reversed);
    void resetCalibration();
    bool saveCalibration();

    DriveMode driveMode() const;
    ActionMode actionMode() const;
    GaitStyle gaitStyle() const;
    bool calibrationMode() const;
    bool calibrationLoaded() const;
    int8_t trim(uint8_t id) const;
    bool reversed(uint8_t id) const;
    uint8_t speed() const;
    const char* modeName() const;
    const char* gaitName() const;

private:
    void writePose(const float pose[8]);
    void writeServo(uint8_t id, float angle);
    void buildDrivePose(float pose[8], uint32_t now);
    void buildGaitPose(float pose[8], uint32_t elapsed, DriveMode mode, uint8_t speed) const;
    void buildActionPose(float pose[8], uint32_t now);
    void smoothToward(const float target[8]);

    float wave(uint32_t now, float periodMs, float phaseDeg) const;
    int angleToMicros(float angle) const;
    void loadCalibration();
    void applyDefaultCalibration();

    Servo _servo[ServoCount];
    float _current[ServoCount];
    float _calibrationPose[ServoCount];
    int8_t _trim[ServoCount];
    bool _reversed[ServoCount];
    DriveMode _driveMode = DriveMode::Stop;
    ActionMode _actionMode = ActionMode::None;
    GaitStyle _gaitStyle = GaitStyle::Normal;
    bool _calibrationMode = false;
    bool _calibrationLoaded = false;
    uint8_t _speed = 55;
    uint32_t _modeStartedAt = 0;
    uint32_t _actionStartedAt = 0;
    uint32_t _lastFrameAt = 0;
};
