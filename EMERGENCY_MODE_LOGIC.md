# Emergency Mode Logic - Corrected Version

## Requirements

The Emergency Mode display and function must remain active until Pin 19 is opened again. After that, the system checks whether the level is still above the alarm level or below it.

## New State Machine Logic

### State: NORMAL MODE

**Conditions:**
- Emergency Mode is NOT active
- Relay: ON (HIGH)
- Alarm: Depends on threshold
  - Level ≥ Threshold → Alarm ON
  - Level < Threshold → Alarm OFF
- Display: Shows fill level and volume

**Transition to EMERGENCY MODE:**
- When BOTH conditions are met:
  1. Fill Level ≥ Alarm Threshold (e.g., 80%)
  2. Pin 19 closed (LOW - connected to GND)

### State: EMERGENCY MODE

**Conditions:**
- Emergency Mode is active
- Relay: ON (HIGH) - Safe state (returns to normal position)
- Alarm: ON (HIGH) - Always active
- Display: Shows "EMERGENCY MODE"

**Behavior during Emergency Mode:**
- Remains active AS LONG AS Pin 19 is closed
- Level changes are IGNORED
- Even if level falls below threshold, Emergency Mode stays active
- Only opening Pin 19 can end Emergency Mode

**Transition to NORMAL MODE:**
- ONLY when Pin 19 is opened (HIGH - disconnected from GND)
- After opening, level is checked:
  - Level ≥ Threshold → Normal Alarm Mode (Relay OFF, Alarm ON)
  - Level < Threshold → Normal Mode (Relay ON, Alarm OFF)

## Flow Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      NORMAL MODE                            │
│  Relay: ON/OFF  │  Alarm: Threshold-based  │  Display: Vol │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           │ Level ≥ Threshold AND Pin 19 closed
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                    EMERGENCY MODE                           │
│  Relay: ON  │  Alarm: ON  │  Display: "EMERGENCY MODE"     │
│                                                             │
│  Stays active as long as Pin 19 is closed!                 │
│  (Level changes are ignored)                                │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           │ Pin 19 opened
                           ▼
                    ┌──────────────┐
                    │ Check level  │
                    └──────┬───────┘
                           │
              ┌────────────┴────────────┐
              │                         │
    Level ≥ Threshold         Level < Threshold
              │                         │
              ▼                         ▼
    ┌─────────────────┐       ┌─────────────────┐
    │ NORMAL MODE     │       │ NORMAL MODE     │
    │ Relay: OFF      │       │ Relay: ON       │
    │ Alarm: ON       │       │ Alarm: OFF      │
    └─────────────────┘       └─────────────────┘
```

## Relay Logic Summary

**Normal Mode:**
- Fill level < 80%: Relay ON (normal state)
- Fill level ≥ 80%: Relay OFF (alarm state)

**Emergency Mode:**
- Relay ON (returns to safe/normal position)
- Stays ON regardless of level

**After Emergency Mode ends:**
- Check level and set relay accordingly

## Example Scenarios

### Scenario 1: Normal Alarm
```
1. Tank fills to 85% (Threshold: 80%)
2. Pin 19 is open
3. Status: NORMAL MODE
   - Relay: OFF (alarm state)
   - Alarm: ON (because 85% ≥ 80%)
   - Display: "Fill:XXcm 85%" / "Vol:XXXXL"
```

### Scenario 2: Activate Emergency
```
1. Tank is at 85% (above threshold)
2. Pin 19 is closed (connected to GND)
3. Status: EMERGENCY MODE activated
   - Relay: ON (safe state - returns to normal position)
   - Alarm: ON
   - Display: "Fill:XXcm 85%" / "EMERGENCY MODE"
   - Serial: "Emergency ON"
```

### Scenario 3: Level drops during Emergency Mode
```
1. Emergency Mode is active (Pin 19 closed)
2. Tank empties to 70% (below threshold)
3. Status: EMERGENCY MODE stays active!
   - Relay: ON (stays on)
   - Alarm: ON (stays on)
   - Display: "Fill:XXcm 70%" / "EMERGENCY MODE"
   - Pin 19 is still closed → Emergency Mode continues
```

### Scenario 4: End Emergency - Level still high
```
1. Emergency Mode is active, tank at 85%
2. Pin 19 is opened (disconnected from GND)
3. Status: EMERGENCY MODE deactivated
   - Level check: 85% ≥ 80% → Alarm remains
   - Relay: OFF (alarm state)
   - Alarm: ON (due to level)
   - Display: "Fill:XXcm 85%" / "Vol:XXXXL"
   - Serial: "Emergency OFF"
```

### Scenario 5: End Emergency - Level low
```
1. Emergency Mode is active, tank at 70%
2. Pin 19 is opened
3. Status: EMERGENCY MODE deactivated
   - Level check: 70% < 80% → No alarm
   - Relay: ON (normal state)
   - Alarm: OFF (level OK)
   - Display: "Fill:XXcm 70%" / "Vol:XXXXL"
   - Serial: "Emergency OFF"
```

## Code Logic

```cpp
void check_alarm_and_relay() {
  bool threshold_exceeded = (fill_percent >= alarm_threshold_percent);
  bool alarm_input_closed = (digitalRead(ALARM_INPUT_PIN) == LOW);
  
  if (emergency_mode) {
    // IN Emergency Mode
    // Only exit when Pin 19 is opened
    if (!alarm_input_closed) {
      emergency_mode = false;
      // After exit: check level for normal alarm
    }
  } else {
    // NOT in Emergency Mode
    // Activate when BOTH conditions are met
    if (threshold_exceeded && alarm_input_closed) {
      emergency_mode = true;
    }
  }
  
  // Control outputs
  // Relay: ON in emergency OR when below threshold
  bool relay_should_be_on = emergency_mode || !threshold_exceeded;
  digitalWrite(RELAY_PIN, relay_should_be_on ? HIGH : LOW);
  
  // Alarm: ON in emergency OR when above threshold
  digitalWrite(ALARM_PIN, (emergency_mode || threshold_exceeded) ? HIGH : LOW);
}
```

## Serial Monitor Output

### Emergency Mode activated:
```
Emergency ON
EMG: 85% A:ON R:ON (Pin18=1 Pin23=1)
```

### Emergency Mode deactivated (level still high):
```
Emergency OFF
NRM: 85% A:ON R:OFF (Pin18=0 Pin23=1)
```

### Emergency Mode deactivated (level OK):
```
Emergency OFF
NRM: 70% A:OFF R:ON (Pin18=1 Pin23=0)
```

## Upload

```bash
pio run -t upload
```

## Testing

1. **Activate Emergency:**
   - Fill tank above 80%
   - Connect Pin 19 to GND
   - Expected: "EMERGENCY MODE" on display, Relay ON

2. **Lower level during Emergency:**
   - Empty tank below 80%
   - Pin 19 stays closed
   - Expected: "EMERGENCY MODE" remains, Relay stays ON

3. **End Emergency (level high):**
   - Tank above 80%
   - Open Pin 19 (disconnect from GND)
   - Expected: Display shows volume, Relay OFF, Alarm ON

4. **End Emergency (level low):**
   - Tank below 80%
   - Open Pin 19
   - Expected: Display shows volume, Relay ON, Alarm OFF
