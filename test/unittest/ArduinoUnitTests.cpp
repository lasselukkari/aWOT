#include "ArduinoUnitTests.h"

TestSetup *TestSetup::sInstance;
TestTeardown *TestTeardown::sInstance;

Test* Test::sRoot = 0;
