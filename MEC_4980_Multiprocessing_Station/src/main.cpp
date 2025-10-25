#include <Arduino.h>
#include <P1AM.h>

enum MachineStates
{
  Waiting,
  MoveToKiln,
  Firing,
  Fired,
  MovingKiln,
  PickUp,
  MoveToTurn,
  Sawing,
  Eject,
  HomeTurn,
  MoveConveyor
};

MachineStates curState = Waiting;

// Modules
int modInput = 1;
int modOutput = 2;
int modAnalog = 3;

// Inputs
int rsSuc = 1;     // Reference switch turntable (suction position)
int rsConv = 2;    // Reference switch turntable (conveyor belt position)
int lbConv = 3;    // Light barrier end of conveyor belt
int rsSaw = 4;     // Reference switch turntable (sawposition)
int rsTurn = 5;    // Reference switch suction (turntable position)
int rsOvenIn = 6;  // Reference switch oven feeder inside
int rsOvenOut = 7; // Reference switch oven feeder outside
int rsKiln = 8;    // Reference switch vacuum (kiln position)
int lbKiln = 9;    // Light barrier kiln

// Outputs
int turnCw = 1;     // Turn motor turntable clockwise
int turnCcw = 2;    // Turn motor turntable counter clockwise
int convFor = 3;    // Conveyor belt motor forwards
int motorSaw = 4;   // Motor saw
int ovenRet = 5;    // Retract oven feeder motor
int ovenExt = 6;    // Extend oven feeder motor
int vacToOven = 7;  // Vacuum towards oven motor
int vacToTurn = 8;  // Vacuum towards turntable motor
int lampKiln = 9;   // Lamp kiln
int Comp = 10;      // Compressor
int valveVac = 11;  // Valve vacuum
int valveLow = 12;  // Valve lowering
int valveDoor = 13; // Valve kiln door
int valveFeed = 14; // Valve feeder

void setup()
{
  delay(500);
  Serial.begin(9600);
  delay(500);
  // Start up P1am modules!

  while (!P1.init())
  {
    delay(1);
  }
}

/*The Multi-processing station should start when a workpiece is placed through the kiln light barrier. The kiln has a carriage with two limit switches, a heating lamp, and a pneumatic door. The suction gripper carriage has two limit switches, a pneumatic plunger, and a pneumatic gripper. The turntable has three position switches, a saw, a motor for rotation, and a pneumatic ejector. The conveyor belt is controlled by a motor and has a light barrier at its exit location. A single compressor is connected to each of the pneumatic components.
The part should be exposed to the kiln heating lamp for 3 seconds and the part should pause at the saw station for 5 seconds. */

bool InputTriggered()
{
  return !P1.readDiscrete(modInput, lbKiln);
}
bool OvenInsideLimit()
{
  return P1.readDiscrete(modInput, rsOvenIn);
}
bool OvenOutsideLimit()
{
  return P1.readDiscrete(modInput, rsOvenOut);
}
bool KilnLimit()
{
  return P1.readDiscrete(modInput, rsKiln);
}
bool TurntableLimit()
{
  return P1.readDiscrete(modInput, rsTurn);
}
bool SawLimit()
{
  return P1.readDiscrete(modInput, rsSaw);
}
bool EjectLimit()
{
  return P1.readDiscrete(modInput, rsConv);
}
bool SuctionLimit()
{
  return P1.readDiscrete(modInput, rsSuc);
}
bool LBConveyor()
{
  return P1.readDiscrete(modInput, lbConv);
}

void KilnDoor(bool s)
{
  P1.writeDiscrete(s, modOutput, valveDoor);
}
void OvenExtend(bool s)
{
  P1.writeDiscrete(s, modOutput, ovenExt);
}
void OvenRetract(bool s)
{
  P1.writeDiscrete(s, modOutput, ovenRet);
}
void KilnLight(bool s)

{
  P1.writeDiscrete(s, modOutput, lampKiln);
}
void VacToOven(bool s)
{
  P1.writeDiscrete(s, modOutput, vacToOven);
}
void VacToTurntable(bool s)
{
  P1.writeDiscrete(s, modOutput, vacToTurn);
}
void ValveLower(bool s)
{
  P1.writeDiscrete(s, modOutput, valveLow);
}
void Suction(bool s)
{
  P1.writeDiscrete(s, modOutput, valveVac);
}
void TurnCW(bool s)
{
  P1.writeDiscrete(s, modOutput, turnCw);
}
void TurnCCW(bool s)
{
  P1.writeDiscrete(s, modOutput, turnCcw);
}
void Saw(bool s)
{
  P1.writeDiscrete(s, modOutput, motorSaw);
}
void Ejector(bool s)
{
  P1.writeDiscrete(s, modOutput, valveFeed);
}
void Conveyor(bool s)
{
  P1.writeDiscrete(s, modOutput, convFor);
}

void loop()
{
  P1.writeDiscrete(true, modOutput, Comp);

  switch (curState)
  {
  case Waiting:
    if (InputTriggered())
    {
      Serial.println("sensed");
      delay(500);
      curState = MoveToKiln;
    }
    break;

  case MoveToKiln:
    KilnDoor(true);

    if (!OvenInsideLimit())
    {
      OvenRetract(true);
    }
    else if (OvenInsideLimit())
    {
      OvenRetract(false);
      curState = Firing;
    }
    break;

  case Firing:
    KilnDoor(false);
    KilnLight(HIGH);
    delay(3000);
    KilnLight(LOW);
    curState = Fired;
    break;
  case Fired:
    KilnDoor(true);
    delay(500);
    OvenExtend(true);
    if (OvenOutsideLimit())
    {
      OvenExtend(false);
      KilnDoor(false);
      curState = MovingKiln;
    }
    break;
  case MovingKiln:
    if (!KilnLimit())
    {
      VacToOven(true);
    }
    else if (KilnLimit())
    {
      VacToOven(false);
      curState = PickUp;
    }
    break;

  case PickUp:
    ValveLower(true);
    delay(1000);
    Suction(true);
    ValveLower(false);
    delay(500);

    curState = MoveToTurn;
    break;
  case MoveToTurn:

    if (!TurntableLimit())
    {
      VacToTurntable(true);
    }
    else if (TurntableLimit())
    {
      VacToTurntable(false);
      if (!SuctionLimit())
      {
        TurnCCW(true);
      }
      else if (SuctionLimit())
      {
        TurnCCW(false);
        ValveLower(true);
        delay(1000);
        Suction(false);
        delay(500);
        ValveLower(false);
        delay(1000);
        curState = Sawing;
      }
    }

    break;
  case Sawing:
    if (!SawLimit())
    {
      TurnCW(true);
    }
    else if (SawLimit())
    {
      TurnCW(false);
      Saw(true);
      delay(5000);
      Saw(false);
      curState = Eject;
    }

    break;

  case Eject:

    if (!EjectLimit())
    {
      TurnCW(true);
    }
    else if (EjectLimit())
    {
      TurnCW(false);
      Ejector(true);
      delay(500);
      Ejector(false);
      curState = HomeTurn;
    }
    break;
  case HomeTurn:
    if (!SuctionLimit())
    {
      TurnCCW(true);
    }
    else if (SuctionLimit())
    {
      TurnCCW(false);
      curState = MoveConveyor;
    }
    break;

  case MoveConveyor:

    while (LBConveyor())
    {
      Conveyor(true);
    }
    if (!LBConveyor())
    {delay(1000);
      Conveyor(false);
      curState = Waiting;
    }
    break;
  }
}