#include "relay.h"

Preferences Relay::sPreferences;

Relay::Relay(int pin) : mPin(pin), mPreferencesNamespace("relay"), mUnixOnTime(-1), mUnixOffTime(-1), mState(false) {}

Relay::Relay(int pin, const char* preferencesNamespace) : mPin(pin), mPreferencesNamespace(preferencesNamespace), mUnixOnTime(-1), mUnixOffTime(-1), mState(false) {}

void Relay::setup() {
  pinMode(mPin, OUTPUT);
  loadState();
  loadOnOffTime();
}

void Relay::loop() {
  if (mUnixOnTime == -1 || mUnixOffTime == -1) {
    return;
  }

  time_t now = getUnixTime();
  if (now > mUnixOffTime) {
    off();
  }
}

bool Relay::timeout() {
  if (mUnixOffTime == -1) {
    return false;
  }

  time_t now = getUnixTime();
  if (now >= mUnixOffTime) {
    off();
    return true;
  }

  return false;
}

void Relay::on() {
  setState(true);
  saveState();
  resetOnOffTime();
}

void Relay::on(unsigned long seconds) {
  setState(true);
  saveState();

  mUnixOnTime = getUnixTime();
  mUnixOffTime = mUnixOnTime + seconds;
  saveOnOffTime();
}

void Relay::off() {
  setState(false);
  deleteSavedState();
  resetOnOffTime();
}

bool Relay::getState() {
  return mState;
}

time_t Relay::getOnTime() {
  return mUnixOnTime;
}

time_t Relay::getOffTime() {
  return mUnixOffTime;
}

void Relay::setState(bool newState) {
  mState = newState;
  digitalWrite(mPin, mState ? HIGH : LOW);
}

size_t Relay::saveState() {
  String prefsKey = getPrefsKey();

  sPreferences.begin(mPreferencesNamespace, false);
  size_t success = sPreferences.putBool(prefsKey.c_str(), mState);
  sPreferences.end();

  return success;
}

void Relay::loadState() {
  String prefsKey = getPrefsKey();

  sPreferences.begin(mPreferencesNamespace, true);
  bool lastState = sPreferences.getBool(prefsKey.c_str(), false);
  sPreferences.end();

  setState(lastState);
}

time_t Relay::getUnixTime() {
  time_t now;
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    return 0;
  }

  time(&now);
  return now;
}

String Relay::getTimeString(time_t unixTime) {
  struct tm timeinfo;
  localtime_r(&unixTime, &timeinfo);

  char buffer[64];
  strftime(buffer, sizeof(buffer), "%F %T", &timeinfo);

  return String(buffer);
}

size_t Relay::saveOnOffTime() {
  String prefsKey = getPrefsKey() + "Time";
  String onOffTime = getTimeString(mUnixOnTime) + "," + getTimeString(mUnixOffTime);

  sPreferences.begin(mPreferencesNamespace, false);
  size_t success = sPreferences.putString(prefsKey.c_str(), onOffTime);
  sPreferences.end();

  return success;
}

void Relay::loadOnOffTime() {
  resetOnOffTime();

  String prefsKey = getPrefsKey() + "Time";

  sPreferences.begin(mPreferencesNamespace, true);
  String lastOnOffTime = sPreferences.getString(prefsKey.c_str());
  sPreferences.end();

  int separatorIndex = lastOnOffTime.indexOf(',');
  if (separatorIndex == -1) {
    return;
  }

  String unixOnTime = lastOnOffTime.substring(0, separatorIndex);
  String unixOffTime = lastOnOffTime.substring(separatorIndex + 1);

  struct tm timeinfo;
  strptime(unixOnTime.c_str(), "%F %T", &timeinfo);
  mUnixOnTime = mktime(&timeinfo);

  strptime(unixOffTime.c_str(), "%F %T", &timeinfo);
  mUnixOffTime = mktime(&timeinfo);
}

void Relay::deleteSavedState() {
  String prefsKey = getPrefsKey();

  sPreferences.begin(mPreferencesNamespace, false);
  sPreferences.remove(prefsKey.c_str());
  sPreferences.end();
}

void Relay::deleteSavedOnOffTime() {
  String prefsKey = getPrefsKey() + "Time";

  sPreferences.begin(mPreferencesNamespace, false);
  sPreferences.remove(prefsKey.c_str());
  sPreferences.end();
}

void Relay::resetOnOffTime() {
  mUnixOnTime = -1;
  mUnixOffTime = -1;
  deleteSavedOnOffTime();
}

String Relay::getPrefsKey() {
  return mPreferencesNamespace + String(mPin);
}