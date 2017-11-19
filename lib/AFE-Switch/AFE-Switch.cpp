#include "AFE-Switch.h"

AFESwitch::AFESwitch(){};

AFESwitch::AFESwitch(uint8_t id) { begin(id); }

void AFESwitch::begin(uint8_t id) {
  AFEDataAccess Data;
  SwitchConfiguration = Data.getSwitchConfiguration(id);
  if (SwitchConfiguration.present) {
    pinMode(SwitchConfiguration.gpio, INPUT_PULLUP);
    state = digitalRead(SwitchConfiguration.gpio);
    previousState = state;
    Led.begin(0);
    _initialized = true;
  }
}

boolean AFESwitch::getState() { return state; }

boolean AFESwitch::isPressed() {
  if (pressed) {
    Led.blink(50);
    pressed = false;
    return true;
  } else {
    return false;
  }
}

boolean AFESwitch::is5s() {
  if (pressed4fiveSeconds) {
    pressed4fiveSeconds = false;
    return true;
  } else {
    return false;
  }
}

boolean AFESwitch::is10s() {
  if (pressed4tenSeconds) {
    pressed4tenSeconds = false;
    Led.blink(50);
    return true;
  } else {
    return false;
  }
}

void AFESwitch::listener() {
  if (_initialized) {
    boolean currentState = digitalRead(SwitchConfiguration.gpio);
    unsigned long time = millis();

    if (currentState != previousState) {

      if (startTime == 0) {
        startTime = time;
      }

      if (time - startTime >= SwitchConfiguration.sensitiveness) {
        if (SwitchConfiguration.type == SWITCH_TYPE_MONO) {

          if (!_pressed) {
            state = !state;
            _pressed = true;
            pressed = true;
          }

          /* Code only for Mulitifunction switch: pressed for 5 and 10 seconds
           */
          if (SwitchConfiguration.functionality == SWITCH_MULTI) {
            if (time - startTime >= 10000 && !pressed4tenSeconds) {
              pressed4tenSeconds = true;
            }

            if (time - startTime >= 5000 && !_pressed4fiveSeconds) {
              Led.blink(500);
              _pressed4fiveSeconds = true;
            }
          }
        } else { // This is BI-stable code
          state = !state;
          previousState = currentState;
          pressed = true;
        }
      }

    } else if (currentState == previousState && startTime > 0) {
      /* Code only for Mulitifunction switch: pressed for 5 and 10 seconds */
      if (SwitchConfiguration.functionality == SWITCH_MULTI) {
        if ( // SwitchConfiguration.type == SWITCH_TYPE_MONO &&
            time - startTime >= 5000 && time - startTime < 10000) {
          pressed4fiveSeconds = true;
        }
        _pressed4fiveSeconds = false;
      }
      startTime = 0;
      _pressed = false;
    }
  }
}

uint8_t AFESwitch::getFunctionality() {
  return SwitchConfiguration.functionality;
}
