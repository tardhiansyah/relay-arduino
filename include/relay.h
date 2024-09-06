#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>
#include <Preferences.h>
#include <time.h>

class Relay {
public:
  Relay(int pin);
  Relay(int pin, const char* preferencesNamespace);

  void setup();
  void loop();

  void on();
  void on(unsigned long seconds);
  void off();

  bool timeout();
  bool getState();
  time_t getOnTime();
  time_t getOffTime();

private:
  int mPin;
  bool mState;
  const char* mPreferencesNamespace;
  time_t mUnixOnTime;
  time_t mUnixOffTime;
  static Preferences sPreferences;

  void setState(bool newState);
  size_t saveState();
  void loadState();
  void deleteSavedState();

  time_t getUnixTime();
  String getTimeString(time_t unixTime);
  size_t saveOnOffTime();
  void loadOnOffTime();
  void deleteSavedOnOffTime();
  void resetOnOffTime();
  String getPrefsKey();
};

#endif