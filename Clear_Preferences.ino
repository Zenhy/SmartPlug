#include <Preferences.h>

Preferences preferences;

void setup() {
  // Start Serial communication for debugging
  Serial.begin(115200);

  // Initialize the Preferences object with a namespace (you can choose any name)
  preferences.begin("my_namespace", false);  // false means not read-only

  // Erase all the preferences stored under this namespace
  preferences.clear();

  // Print message to Serial Monitor
  Serial.println("Preferences cleared!");

  // End Preferences
  preferences.end();
}

void loop() {
  // No continuous operation in loop
}
