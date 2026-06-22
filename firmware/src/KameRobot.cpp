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

void KameRobot::setGaitStyle(GaitStyle style) {
    _gaitStyle = style;
}

DriveMode KameRobot::driveMode() const {
    return _driveMode;
}

ActionMode KameRobot::actionMode() const {
    return _actionMode;
}

GaitStyle KameRobot::gaitStyle() const {
    return _gaitStyle;
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
            case ActionMode::Bow: return "bow";
            case ActionMode::Wiggle: return "wiggle";
            case ActionMode::Stretch: return "stretch";
            case ActionMode::Patrol: return "patrol";
            case ActionMode::ShowOff: return "showtime";
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

const char* KameRobot::gaitName() const {
    switch (_gaitStyle) {
        case GaitStyle::Sneak: return "sneak";
        case GaitStyle::Bounce: return "bounce";
        case GaitStyle::Normal:
        default:
            return "normal";
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
    buildGaitPose(pose, now - _modeStartedAt, _driveMode, _speed);
}

void KameRobot::buildGaitPose(float pose[8], uint32_t elapsed, DriveMode mode, uint8_t speed) const {
    memcpy(pose, HOME_POSE, sizeof(float) * SERVO_COUNT);

    const float speedScale = clampSpeed(speed) / 100.0f;
    float period = 900.0f - (speedScale * 430.0f);
    float stride = 7.0f + (speedScale * 15.0f);
    float lift = 7.0f + (speedScale * 19.0f);

    if (_gaitStyle == GaitStyle::Sneak) {
        period *= 1.35f;
        stride *= 0.58f;
        lift *= 0.56f;
    } else if (_gaitStyle == GaitStyle::Bounce) {
        period *= 0.86f;
        stride *= 1.08f;
        lift *= 1.34f;
    }

    const float t = elapsed * 360.0f / period;

    float hipSign = 1.0f;
    float turnSign = 0.0f;

    if (mode == DriveMode::Backward) {
        hipSign = -1.0f;
    } else if (mode == DriveMode::Left) {
        turnSign = -1.0f;
    } else if (mode == DriveMode::Right) {
        turnSign = 1.0f;
    }

    const float a = wave(elapsed, period, 0);
    const float b = wave(elapsed, period, 180);

    if (mode == DriveMode::Left || mode == DriveMode::Right) {
        pose[0] += turnSign * stride * a;
        pose[1] += turnSign * stride * b;
        pose[4] += turnSign * stride * b;
        pose[5] += turnSign * stride * a;
    } else {
        pose[0] += hipSign * stride * wave(elapsed, period, 90);
        pose[1] += hipSign * stride * wave(elapsed, period, 90);
        pose[4] += hipSign * stride * wave(elapsed, period, 270);
        pose[5] += hipSign * stride * wave(elapsed, period, 270);
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

        case ActionMode::Bow: {
            if (elapsed > 2600) {
                _actionMode = ActionMode::None;
                return;
            }
            const float dip = positive(wave(elapsed, 1300, 0));
            pose[0] = 118;
            pose[1] = 62;
            pose[2] = 56 - 32 * dip;
            pose[3] = 124 + 32 * dip;
            pose[4] = 84;
            pose[5] = 96;
            pose[6] = 124 - 10 * dip;
            pose[7] = 56 + 10 * dip;
            break;
        }

        case ActionMode::Wiggle:
            if (elapsed > 3000) {
                _actionMode = ActionMode::None;
                return;
            }
            pose[0] += 22 * wave(elapsed, 360, 0);
            pose[1] += 22 * wave(elapsed, 360, 180);
            pose[4] += 22 * wave(elapsed, 360, 180);
            pose[5] += 22 * wave(elapsed, 360, 0);
            pose[2] -= 8 * wave(elapsed, 720, 90);
            pose[3] += 8 * wave(elapsed, 720, 90);
            pose[6] += 8 * wave(elapsed, 720, 270);
            pose[7] -= 8 * wave(elapsed, 720, 270);
            break;

        case ActionMode::Stretch: {
            if (elapsed > 3400) {
                _actionMode = ActionMode::None;
                return;
            }
            const float reach = positive(wave(elapsed, 1700, 0));
            pose[0] = 132;
            pose[1] = 48;
            pose[2] = 66 - 24 * reach;
            pose[3] = 114 + 24 * reach;
            pose[4] = 42;
            pose[5] = 138;
            pose[6] = 116;
            pose[7] = 64;
            break;
        }

        case ActionMode::Patrol: {
            if (elapsed > 11500) {
                _actionMode = ActionMode::None;
                return;
            }
            const uint8_t segment = elapsed / 1900;
            DriveMode mode = DriveMode::Forward;
            if (segment == 1) mode = DriveMode::Right;
            else if (segment == 3) mode = DriveMode::Left;
            else if (segment == 4) mode = DriveMode::Backward;
            else if (segment == 5) mode = DriveMode::Right;
            buildGaitPose(pose, elapsed, mode, 48);
            break;
        }

        case ActionMode::ShowOff: {
            if (elapsed > 9600) {
                _actionMode = ActionMode::None;
                return;
            }
            if (elapsed < 1800) {
                const float dip = positive(wave(elapsed, 900, 0));
                pose[2] = 56 - 26 * dip;
                pose[3] = 124 + 26 * dip;
                pose[4] = 84;
                pose[5] = 96;
            } else if (elapsed < 3800) {
                pose[0] += 24 * wave(elapsed, 320, 0);
                pose[1] += 24 * wave(elapsed, 320, 180);
                pose[4] += 24 * wave(elapsed, 320, 180);
                pose[5] += 24 * wave(elapsed, 320, 0);
            } else if (elapsed < 6600) {
                pose[2] = 90 - 30 * wave(elapsed, 620, 0);
                pose[3] = 90 + 30 * wave(elapsed, 620, 120);
                pose[6] = 90 + 30 * wave(elapsed, 620, 180);
                pose[7] = 90 - 30 * wave(elapsed, 620, 290);
            } else {
                pose[0] += 18 * wave(elapsed, 470, 0);
                pose[1] += 18 * wave(elapsed, 470, 180);
                pose[4] += 18 * wave(elapsed, 470, 180);
                pose[5] += 18 * wave(elapsed, 470, 0);
                pose[2] -= 18 * wave(elapsed, 470, 90);
                pose[3] += 18 * wave(elapsed, 470, 270);
                pose[6] += 18 * wave(elapsed, 470, 270);
                pose[7] -= 18 * wave(elapsed, 470, 90);
            }
            break;
        }

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
