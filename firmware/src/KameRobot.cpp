#include "KameRobot.h"

#include <math.h>
#include <string.h>

namespace {
constexpr uint8_t SERVO_COUNT = 8;
constexpr uint16_t FRAME_MS = 16;
constexpr uint16_t SERVO_MIN_US = 544;
constexpr uint16_t SERVO_MAX_US = 2400;

const uint8_t SERVO_PINS[SERVO_COUNT] = {
    D1, D4, D8, D6, D7, D5, D2, D3
};

const float HOME_POSE[SERVO_COUNT] = {
    108, 72, 56, 124, 72, 108, 124, 56
};

const float TRIM[SERVO_COUNT] = {
    0, 0, 0, 0, 0, 0, 0, 0
};

const bool REVERSED[SERVO_COUNT] = {
    false, false, false, false, false, false, false, false
};

float clampAngle(float value) {
    if (value < 0) return 0;
    if (value > 180) return 180;
    return value;
}

float clampSpeed(uint8_t speed) {
    if (speed < 20) return 20;
    if (speed > 100) return 100;
    return speed;
}

float phaseToRad(float phaseDeg) {
    return phaseDeg * PI / 180.0f;
}

float positive(float value) {
    return value > 0.0f ? value : 0.0f;
}
}

void KameRobot::begin() {
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        _servo[i].attach(SERVO_PINS[i]);
        _current[i] = HOME_POSE[i];
        writeServo(i, HOME_POSE[i]);
    }
}

void KameRobot::tick() {
    const uint32_t now = millis();
    if (now - _lastFrameAt < FRAME_MS) {
        return;
    }
    _lastFrameAt = now;

    float target[SERVO_COUNT];
    if (_actionMode != ActionMode::None) {
        buildActionPose(target, now);
    } else if (_driveMode != DriveMode::Stop) {
        buildDrivePose(target, now);
    } else {
        memcpy(target, HOME_POSE, sizeof(target));
    }

    smoothToward(target);
}

void KameRobot::drive(DriveMode mode, uint8_t speed) {
    if (_driveMode != mode || _actionMode != ActionMode::None) {
        _modeStartedAt = millis();
    }
    _actionMode = ActionMode::None;
    _driveMode = mode;
    _speed = constrain(speed, 20, 100);
}

void KameRobot::stop() {
    _driveMode = DriveMode::Stop;
    _actionMode = ActionMode::None;
    _modeStartedAt = millis();
}

void KameRobot::home() {
    stop();
    writePose(HOME_POSE);
}

void KameRobot::action(ActionMode action) {
    _driveMode = DriveMode::Stop;
    _actionMode = action;
    _actionStartedAt = millis();
}

DriveMode KameRobot::driveMode() const {
    return _driveMode;
}

ActionMode KameRobot::actionMode() const {
    return _actionMode;
}

uint8_t KameRobot::speed() const {
    return _speed;
}

const char* KameRobot::modeName() const {
    if (_actionMode != ActionMode::None) {
        switch (_actionMode) {
            case ActionMode::Hello: return "hello";
            case ActionMode::Dance: return "dance";
            case ActionMode::PushUp: return "pushup";
            case ActionMode::Moonwalk: return "moonwalk";
            default: break;
        }
    }

    switch (_driveMode) {
        case DriveMode::Forward: return "forward";
        case DriveMode::Backward: return "backward";
        case DriveMode::Left: return "left";
        case DriveMode::Right: return "right";
        case DriveMode::Stop:
        default:
            return "stop";
    }
}

void KameRobot::writePose(const float pose[8]) {
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        writeServo(i, pose[i]);
        _current[i] = pose[i];
    }
}

void KameRobot::writeServo(uint8_t id, float angle) {
    angle = clampAngle(angle + TRIM[id]);
    if (REVERSED[id]) {
        angle = 180.0f - angle;
    }
    _servo[id].writeMicroseconds(angleToMicros(angle));
}

void KameRobot::buildDrivePose(float pose[8], uint32_t now) {
    memcpy(pose, HOME_POSE, sizeof(float) * SERVO_COUNT);

    const float speedScale = clampSpeed(_speed) / 100.0f;
    const float period = 900.0f - (speedScale * 430.0f);
    const float stride = 7.0f + (speedScale * 15.0f);
    const float lift = 7.0f + (speedScale * 19.0f);
    const float t = (now - _modeStartedAt) * 360.0f / period;

    float hipSign = 1.0f;
    float turnSign = 0.0f;

    if (_driveMode == DriveMode::Backward) {
        hipSign = -1.0f;
    } else if (_driveMode == DriveMode::Left) {
        turnSign = -1.0f;
    } else if (_driveMode == DriveMode::Right) {
        turnSign = 1.0f;
    }

    const float a = wave(now - _modeStartedAt, period, 0);
    const float b = wave(now - _modeStartedAt, period, 180);

    if (_driveMode == DriveMode::Left || _driveMode == DriveMode::Right) {
        pose[0] += turnSign * stride * a;
        pose[1] += turnSign * stride * b;
        pose[4] += turnSign * stride * b;
        pose[5] += turnSign * stride * a;
    } else {
        pose[0] += hipSign * stride * wave(now - _modeStartedAt, period, 90);
        pose[1] += hipSign * stride * wave(now - _modeStartedAt, period, 90);
        pose[4] += hipSign * stride * wave(now - _modeStartedAt, period, 270);
        pose[5] += hipSign * stride * wave(now - _modeStartedAt, period, 270);
    }

    pose[2] -= lift * positive(sin(phaseToRad(t + 270)));
    pose[3] += lift * positive(sin(phaseToRad(t + 90)));
    pose[6] += lift * positive(sin(phaseToRad(t + 90)));
    pose[7] -= lift * positive(sin(phaseToRad(t + 270)));
}

void KameRobot::buildActionPose(float pose[8], uint32_t now) {
    memcpy(pose, HOME_POSE, sizeof(float) * SERVO_COUNT);

    const uint32_t elapsed = now - _actionStartedAt;

    switch (_actionMode) {
        case ActionMode::Hello:
            if (elapsed > 2300) {
                _actionMode = ActionMode::None;
                return;
            }
            pose[0] = 155;
            pose[1] = 26 + 22 * wave(elapsed, 360, 0);
            pose[2] = 90;
            pose[3] = 92 + 24 * wave(elapsed, 360, 90);
            break;

        case ActionMode::Dance:
            if (elapsed > 3600) {
                _actionMode = ActionMode::None;
                return;
            }
            pose[0] += 16 * wave(elapsed, 520, 0);
            pose[1] += 16 * wave(elapsed, 520, 180);
            pose[4] += 16 * wave(elapsed, 520, 180);
            pose[5] += 16 * wave(elapsed, 520, 0);
            pose[2] -= 18 * wave(elapsed, 520, 90);
            pose[3] += 18 * wave(elapsed, 520, 270);
            pose[6] += 18 * wave(elapsed, 520, 270);
            pose[7] -= 18 * wave(elapsed, 520, 90);
            break;

        case ActionMode::PushUp:
            if (elapsed > 3200) {
                _actionMode = ActionMode::None;
                return;
            }
            pose[0] = 90;
            pose[1] = 90;
            pose[4] = 38;
            pose[5] = 142;
            pose[2] = 56 - 28 * positive(wave(elapsed, 750, 0));
            pose[3] = 124 + 28 * positive(wave(elapsed, 750, 180));
            break;

        case ActionMode::Moonwalk:
            if (elapsed > 4200) {
                _actionMode = ActionMode::None;
                return;
            }
            pose[2] = 90 - 34 * wave(elapsed, 700, 0);
            pose[3] = 90 + 34 * wave(elapsed, 700, 120);
            pose[6] = 90 + 34 * wave(elapsed, 700, 180);
            pose[7] = 90 - 34 * wave(elapsed, 700, 290);
            break;

        case ActionMode::None:
        default:
            break;
    }
}

void KameRobot::smoothToward(const float target[8]) {
    const float maxStep = 5.0f;

    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        const float delta = target[i] - _current[i];
        if (delta > maxStep) {
            _current[i] += maxStep;
        } else if (delta < -maxStep) {
            _current[i] -= maxStep;
        } else {
            _current[i] = target[i];
        }
        writeServo(i, _current[i]);
    }
}

float KameRobot::wave(uint32_t now, float periodMs, float phaseDeg) const {
    const float angle = ((float)(now % (uint32_t)periodMs) / periodMs) * 360.0f;
    return sin(phaseToRad(angle + phaseDeg));
}

int KameRobot::angleToMicros(float angle) const {
    angle = clampAngle(angle);
    return SERVO_MIN_US + (int)((angle / 180.0f) * (SERVO_MAX_US - SERVO_MIN_US));
}
