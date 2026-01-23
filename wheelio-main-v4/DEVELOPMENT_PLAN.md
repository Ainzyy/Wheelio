# Wheelio Development Plan & Roadmap

**Project**: ESP32 Mobility Safety System (Wheelio v4)  
**Status**: MVP Complete - Phase 1  
**Last Updated**: January 23, 2026

---

## Table of Contents
1. [Project Overview](#project-overview)
2. [Phase 1: MVP (Complete ✅)](#phase-1-mvp-complete-)
3. [Phase 2: Hardware Validation (Current 🔄)](#phase-2-hardware-validation-current-)
4. [Phase 3: WiFi Connectivity (Planned)](#phase-3-wifi-connectivity-planned)
5. [Phase 4: Firebase Integration (Planned)](#phase-4-firebase-integration-planned)
6. [Phase 5: Remote Configuration (Planned)](#phase-5-remote-configuration-planned)
7. [Architecture & Technical Details](#architecture--technical-details)

---

## Project Overview

### Vision
Build a modular, testable ESP32-based safety system for mobility devices with:
- Real-time hazard detection (5 types)
- Sensor calibration support
- Remote configuration capabilities
- Comprehensive monitoring and logging

### Success Criteria
- ✅ All 5 hazard detection types working
- ✅ Debug output with clear event labels
- ✅ Sensor calibration framework
- ✅ Comprehensive unit tests (52 tests)
- ✅ Complete documentation
- 🔄 Hardware validation with real sensors
- ⏳ Remote configuration capability

---

## Phase 1: MVP (Complete ✅)

### Scope
Implement core safety system with all sensor integrations and actuator control.

### Deliverables

#### Hardware Abstraction ✅
- `VL53L1XSensor` - LiDAR distance measurement
- `MPU6050Sensor` - 6-axis IMU (dual-axis tilt + acceleration)
- `LightSensor` - Ambient light detection
- `Relay` - Actuator control (fog light, warning)
- `Buzzer` - Audio alert

#### Software Features ✅
| Feature | Status | Details |
|---------|--------|---------|
| Fog Light Control | ✅ | Triggered when light < 1000 ADC |
| Proximity Warning | ✅ | Triggered when distance < 200cm |
| Forward/Back Tilt Warning | ✅ | Triggered when \|tiltFB\| > 9° |
| Side Tilt Warning | ✅ | Triggered when \|tiltSide\| > 30° |
| Impact Warning | ✅ | Triggered when acceleration > 2.0G |
| EMA Filtering | ✅ | Configurable alpha (0.2f default) |
| Debug Labels | ✅ | Shows which warnings triggered |
| Sensor Calibration | ✅ | Distance offset + tilt zero reference |
| Mock Data Mode | ✅ | Simulate sensors without hardware |
| Unit Tests | ✅ | 52 comprehensive tests |
| Configuration | ✅ | Centralized config.h |
| Documentation | ✅ | Complete README |

#### Code Quality ✅
- Modular architecture (each component isolated)
- Single I2C bus (optimized)
- Non-blocking sensor polling (100ms interval)
- Zero compilation errors
- All tests passing

---

## Phase 2: Hardware Validation (Current 🔄)

### Objective
Validate that real hardware responds correctly to detected hazards and that mock data accurately simulates sensor behavior.

### Activities

#### 2.1 Distance Sensor Calibration
```
Task: Establish baseline distance accuracy
├── Test at 50cm, 100cm, 150cm, 200cm, 250cm
├── Measure offset errors
├── Set DISTANCE_CALIBRATION_OFFSET in config.h
└── Verify ±2cm accuracy
Timeline: 1-2 hours
```

#### 2.2 IMU Tilt Validation
```
Task: Verify dual-axis tilt thresholds
├── Place device level and verify 0° tilt reading (after calibrateLevel())
├── Tilt forward to exactly 9° and verify threshold boundary
├── Tilt side to exactly 30° and verify threshold boundary
├── Test negative angles (backward/side roll)
└── Verify warnings trigger at correct angles
Timeline: 1-2 hours
```

#### 2.3 Impact Detection Testing
```
Task: Validate acceleration threshold
├── Gently shake device (< 2.0G) - no warning
├── Firm tap/drop simulation (> 2.0G) - warning triggers
├── Measure actual G-forces with sensor
└── Adjust ACCEL_THRESHOLD if needed
Timeline: 30 minutes
```

#### 2.4 Light Sensor Verification
```
Task: Test fog light activation
├── Test in bright conditions (> 1000 ADC) - FOG:OFF
├── Dim lighting to < 1000 ADC - FOG:ON
├── Measure ambient light levels at threshold
└── Verify relay activation/deactivation
Timeline: 30 minutes
```

#### 2.5 Proximity Testing
```
Task: Validate LiDAR distance threshold
├── Position obstacle at 250cm - no warning
├── Move to 200cm - warning may trigger (at boundary)
├── Move to 150cm - warning triggered
├── Verify distance readings match actual values
└── Compare against calibration offset
Timeline: 1 hour
```

#### 2.6 Integration Testing
```
Task: Test all hazards together
├── Simulate multiple simultaneous warnings
├── Verify debug output shows all triggered hazards (e.g., WARN:[PFBSI])
├── Check relay/buzzer response to combined hazards
├── Verify FOG light state doesn't interfere with warnings
Timeline: 1 hour
```

### Success Criteria
- All sensors within ±5% accuracy of expected values
- All thresholds trigger at specified boundaries
- Debug output labels correctly identify hazards
- Mock data mode produces similar patterns to real data
- No sensor conflicts on shared I2C bus

### Expected Output
```
Example hardware validation session:
[R] L:1250 | D:450 | TFB:0.1 | TS:0.2 | A:1.02
[R] L:1240 | D:420 | TFB:0.0 | TS:0.1 | A:1.01
[R] L:1200 | D:200 | TFB:0.1 | TS:0.0 | A:1.02 | WARN:[P]
[R] L:1100 | D:180 | TFB:9.2 | TS:1.0 | A:1.05 | WARN:[P,FB]
[R] L:800 | D:180 | TFB:9.1 | TS:0.9 | A:1.04 | FOG:ON | WARN:[P,FB]
         ↑ Fog light activated
```

---

## Phase 3: WiFi Connectivity (Planned)

### Objective
Add WiFi capability for real-time data transmission and log uploads.

### Implementation Timeline
**Estimated**: 4-6 hours

### Tasks

#### 3.1 WiFi Setup
```cpp
// Add to platformio.ini
lib_deps = 
    WiFi
    HTTPClient
```

**Code Structure**:
- `include/WiFiManager.h` - WiFi connection handling
- `src/WiFiManager.cpp` - Implementation
- Integration point in `main.cpp` setup

**Features**:
- Auto-connect to configured SSID
- Reconnection logic with exponential backoff
- Signal strength monitoring

#### 3.2 Data Logging to Cloud
```
Structure: Wheelio → WiFi → Cloud (Firebase/AWS)
├── Every hazard event logged with:
│   ├── Timestamp
│   ├── Hazard type (P/FB/S/I/FOG)
│   ├── Sensor values (distance, tilt, accel, light)
│   └── Device ID
└── Periodic health check (CPU temp, battery, uptime)
```

#### 3.3 Configuration
```
Required secrets:
├── WiFi SSID
├── WiFi Password
└── Firebase/Cloud credentials

Storage: NVS (Non-Volatile Storage) on ESP32
```

---

## Phase 4: Firebase Integration (Planned)

### Objective
Enable real-time database connectivity for remote monitoring and configuration.

### Implementation Timeline
**Estimated**: 4-6 hours

### Tasks

#### 4.1 Firebase Setup
```
Add to platformio.ini:
lib_deps = 
    Firebase-ESP-Client
    ArduinoJson
```

#### 4.2 Data Structures

**Device Status (Real-time)**:
```json
{
  "wheelio_001": {
    "status": "online",
    "last_heartbeat": 1674432000,
    "current_readings": {
      "light": 1200,
      "distance": 450,
      "tilt_fb": 2.3,
      "tilt_side": 5.1,
      "accel": 1.05
    },
    "active_warnings": []
  }
}
```

**Event Log**:
```json
{
  "events": {
    "event_001": {
      "timestamp": 1674432000,
      "device_id": "wheelio_001",
      "event_type": "WARN",
      "hazards": ["P", "FB"],
      "readings": {...}
    }
  }
}
```

#### 4.3 Real-time Listeners
- Subscribe to device status changes
- Stream warning events to connected clients
- Monitor device connectivity

---

## Phase 5: Remote Configuration (Planned)

### Objective
Allow configuration changes without firmware re-upload.

### Implementation Timeline
**Estimated**: 6-8 hours

### Architecture

#### 5.1 Remote Config Structure
```json
{
  "wheelio_config": {
    "device_id": "wheelio_001",
    "version": 1,
    "thresholds": {
      "light": 1000,
      "distance": 200,
      "tilt_fb": 9.0,
      "tilt_side": 30.0,
      "accel": 2.0
    },
    "calibration": {
      "distance_offset": 0,
      "tilt_fb_zero": 0.0,
      "tilt_side_zero": 0.0
    },
    "features": {
      "fog_light_enabled": true,
      "warning_system_enabled": true,
      "debug_enabled": true
    },
    "sensor": {
      "poll_interval_ms": 100,
      "filter_alpha_lidar": 0.2,
      "filter_alpha_imu": 0.2
    },
    "last_updated": 1674432000
  }
}
```

#### 5.2 ConfigManager Class
```cpp
// include/ConfigManager.h
class ConfigManager {
public:
    bool loadRemoteConfig();
    void applyConfig();
    void saveLocalCache();
    RemoteConfig getConfig() const;
    bool hasConfigChanged() const;
};
```

#### 5.3 Main Loop Integration
```cpp
// Check for updates every 60 seconds
if (millis() - lastConfigCheck > 60000) {
    if (configManager.loadRemoteConfig()) {
        configManager.applyConfig();
        // Apply to sensor objects
        lidar.setCalibrationOffset(config.distanceOffset);
        imu.setTiltZeroReference(config.tiltFBZero, config.tiltSideZero);
    }
    lastConfigCheck = millis();
}
```

#### 5.4 Features
- **No Firmware Update** - Change thresholds instantly
- **Multi-Device Management** - Control fleet of Wheelio units
- **A/B Testing** - Different configs for different units
- **Emergency Control** - Disable hazard detection if needed
- **Audit Trail** - All changes logged with timestamps
- **Fallback** - Local cache if offline

---

## Architecture & Technical Details

### System Components

```
┌─────────────────────────────────────────────────────────┐
│                    WHEELIO SYSTEM                       │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  SENSORS (I2C Bus)                                     │
│  ├── VL53L1X (LiDAR, 0x29)                            │
│  ├── MPU6050 (IMU, 0x68)                              │
│  └── Light Sensor (ADC, GPIO 33)                      │
│                                                         │
│  ACTUATORS (GPIO)                                      │
│  ├── Fog Light Relay (GPIO 16)                        │
│  ├── Warning Relay (GPIO 17)                          │
│  └── Buzzer (GPIO 5)                                  │
│                                                         │
│  PROCESSING (ESP32)                                    │
│  ├── Sensor Reading (100ms poll)                      │
│  ├── EMA Filtering (α=0.2)                            │
│  ├── Threshold Detection                               │
│  ├── Actuator Control                                 │
│  ├── Config Management                                │
│  └── Debug Output                                     │
│                                                         │
│  CONNECTIVITY (Future Phases)                          │
│  ├── WiFi (Phase 3)                                   │
│  ├── Firebase (Phase 4)                               │
│  └── Remote Config (Phase 5)                          │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### Signal Flow

```
HAZARD DETECTION → WARNING LOGIC → ACTUATOR RESPONSE
        ↓              ↓                   ↓
    5 sensors    4 boolean checks    Relay + Buzzer
    + filter     + OR combination    + Debug labels
```

### Testing Strategy

| Phase | Testing Method | Coverage |
|-------|----------------|----------|
| Phase 1 | Unit tests (52) | 100% of core logic |
| Phase 2 | Hardware validation | Real sensor accuracy |
| Phase 3 | Integration tests | WiFi + data transmission |
| Phase 4 | Firebase tests | Real-time DB operations |
| Phase 5 | End-to-end tests | Remote config application |

---

## Development Timeline

```
Phase 1: MVP              [████████████████] Complete ✅
Phase 2: Hardware Val.    [██████░░░░░░░░░░] 30% (In Progress)
Phase 3: WiFi             [░░░░░░░░░░░░░░░░] Planned
Phase 4: Firebase         [░░░░░░░░░░░░░░░░] Planned
Phase 5: Remote Config    [░░░░░░░░░░░░░░░░] Planned

Estimated Total Timeline: 6-8 weeks
```

---

## Dependencies & Libraries

### Current (Phase 1)
- `Adafruit_VL53L1X@3.1.2` - LiDAR driver
- `Adafruit_MPU6050@2.2.6` - IMU driver
- `Arduino` - Core ESP32 support

### Upcoming (Phases 3-5)
- `WiFi` - ESP32 WiFi support
- `Firebase-ESP-Client` - Firebase integration
- `ArduinoJson` - JSON parsing
- `HTTPClient` - HTTP requests

---

## Known Limitations & Future Improvements

### Current MVP Limitations
- **No temperature compensation** - Tilt/distance affected by temp changes
- **Single device** - No fleet management
- **No data logging** - Only real-time output
- **Manual calibration** - Requires manual offset entry
- **No battery monitoring** - Can't detect low battery

### Future Improvements
- Battery level monitoring
- Temperature-compensated sensor readings
- Machine learning for anomaly detection
- Multiple device dashboard
- Mobile app with real-time alerts
- Predictive maintenance

---

## How to Use This Document

### For Hardware Validation (Phase 2)
1. Reference Section [Phase 2](#phase-2-hardware-validation-current-) 
2. Follow each task in order
3. Update config.h with calibration offsets
4. Verify all success criteria are met

### For Future Development (Phases 3-5)
1. Use architecture overview as reference
2. Follow implementation timeline estimates
3. Refer to proposed file structures
4. Update this document as phases progress

### For Team Reference
- Check status section for current progress
- Review component list for dependencies
- Reference test coverage for validation
- Update "last_updated" date with changes

---

## Questions & Support

For questions about:
- **Current MVP**: See [README.md](README.md)
- **Hardware Integration**: See [Phase 2](#phase-2-hardware-validation-current-)
- **Future Plans**: See [Phases 3-5](#phase-3-wifi-connectivity-planned)
- **Architecture**: See [Architecture Section](#architecture--technical-details)

---

**Document Version**: 1.0  
**Last Updated**: January 23, 2026  
**Next Review**: After Phase 2 completion
