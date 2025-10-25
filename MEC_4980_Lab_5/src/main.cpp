#include <Arduino.h>
#include <P1AM.h>

enum MachineStates
{
  Waiting,
  ColorSensing,
  CountedMove,
  ejectState
};

MachineStates curState = Waiting;

// Modules
int modInput = 1;
int modOutput = 2;
int modAnalog = 3;

// Inputs
int pulse = 1;
int lbIn = 2;
int lbOut = 3;
int lbW = 4;
int lbR = 6;
int lbB = 5;

// Outputs
int conv = 1;
int compressor = 2;
int ejectW = 3;
int ejectR = 4;
int ejectB = 5;
int outW = 6;
int outR = 7;
int outB = 8;

// Analog Inputs
int color = 1;

// Vars
int colorValue = 10000;
int distToEject = 0;
int distMoved = 0;
bool prevKeyState = false;
bool curKey = false;
char targetColor = 'b';

void setup()
{
  delay(1000);
  Serial.begin(9600);
  delay(1000);
  // Start up P1am modules!
  while (!P1.init())
  {
    delay(1);
  }
}

bool InputTriggered()
{
  return !P1.readDiscrete(modInput, lbIn);
}

bool OutputTriggered()
{
  return !P1.readDiscrete(modInput, lbOut);
}

void ToggleConveyor(bool s)
{
  P1.writeDiscrete(s, modOutput, conv);
}

int GetColor()
{
  return P1.readAnalog(modAnalog, color);
}

bool GetPulseKey()
{
  return P1.readDiscrete(modInput, pulse);
}

void ToggleCompressor(bool s)
{
  P1.writeDiscrete(s, modOutput, compressor);
}

void UseEjector(char c)
{
  int tempPin;
  if (c == 'w')
  {
    tempPin = ejectW;
  }
  else if (c == 'r')
  {
    tempPin = ejectR;
  }
  else
  {
    tempPin = ejectB;
  }
  P1.writeDiscrete(true, modOutput, tempPin);
  delay(1500);
  P1.writeDiscrete(false, modOutput, tempPin);
}

void loop()
{

  switch (curState)
  {
  case Waiting:
    // Wait for light barrier to be tripped
    // After tripped, switch state and turn on conveyor
    if (InputTriggered())
    {
      curState = ColorSensing;
      ToggleConveyor(true);
      colorValue = 10000;
    }
    break;
  case ColorSensing:
    // Get color and find min
    colorValue = min(GetColor(), colorValue);
    // Keep on going until second light barrier
    // Then switch states
    if (OutputTriggered())
    {
      curState = CountedMove;
      distMoved = 0;
      // Decide how far to move
      if (colorValue < 2500)
      {
        distToEject = 3;
        targetColor = 'w';
      }
      else if (colorValue < 4800)
      {
        distToEject = 9;
        targetColor = 'r';
      }
      else
      {
        distToEject = 14;
        targetColor = 'b';
      }
      ToggleCompressor(true);
    }

    break;
  case CountedMove:
    // Watch pulse key to move that far
    curKey = GetPulseKey();
    if (curKey && !prevKeyState)
    {
      distMoved++;
    }
    prevKeyState = curKey;
    // Switch states and turn off conveyor
    if (distMoved >= distToEject)
    {
      curState = ejectState;
      ToggleConveyor(false);
    }
    break;
  case ejectState:
    UseEjector(targetColor);
    curState = Waiting;
    break;
  default:
    break;
  }

  if (P1.readDiscrete(modInput, lbW) == true)
  {
    P1.writeDiscrete(false, modOutput, outW);
  }
  else
  {
    P1.writeDiscrete(true, modOutput, outW);
  }

  if (P1.readDiscrete(modInput, lbR) == true)
  {
    P1.writeDiscrete(false, modOutput, outR);
  }
  else
  {
    P1.writeDiscrete(true, modOutput, outR);
  }

  if (P1.readDiscrete(modInput, lbB) == true)
  {
    P1.writeDiscrete(false, modOutput, outB);
  }
  else
  {
    P1.writeDiscrete(true, modOutput, outB);
  }
}