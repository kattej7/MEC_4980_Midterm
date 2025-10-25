#include <Arduino.h>
#include <P1AM.h>
#include <MotorEncoder.h>

// The Pickup Station has three encoder motors which drive horizontal, vertical, and rotational motion. Each axis has a limit switch for initial positioning. The Pickup Station also has a compressor which provides pressure to operate its vacuum gripper.
// The Pickup Station needs to sense when a colored part is available from the sorting line, move to and pick up the part, move to the drop off location, wait for the warehouse to be ready, drop off the part, and return to its 'home' position.  Important to note, the home, pickup, and dropoff locations are at different rotations and heights.

enum MachineStates
{
  Waiting,
  Dropoff,
  CheckWarehouse,
  Warehouse
};

MachineStates curState = Waiting;

// Modules
int modInput = 1;
int modOutput = 2;
int modAnalog = 3;

// Inputs
int rsVert = 1;      // Vertical reference switch
int rsHor = 2;       // Horizontal reference switch
int rsTurn = 3;      // Turn reference switch
int vertPulse1 = 5;  // Encoder vertical pulse 1
int vertPulse2 = 6;  // Encoder vertical pulse 2
int horPulse1 = 7;   // Encoder horizontal pulse 1
int horPulse2 = 8;   // Encoder horizontal pulse 2
int turnPulse1 = 9;  // Turn encoder pulse 1
int turnPulse2 = 10; // Turn encoder pulse 2
int waitWhite = 11;  // ColorWaitingWhite
int waitBlue = 12;   // ColorWaitingBlue
int waitRed = 13;    // ColorWaitingRed
int wareReady = 14;  // WarehouseReady

// Outputs
int vertUp = 1;   // Vertical motor up
int vertDown = 2; // Vertical motor down
int horBack = 3;  // Horizontal motor backwards
int horFor = 4;   // Horizontal motor forwards
int turnCw = 5;   // Turn motor clockwise
int turnCcw = 6;  // Turn motor anti-clockwise
int comp = 7;     // Compressor
int valvac = 8;   // Valve vacuum
int wareSig = 9;  // WarehouseSignal

int BlueTurnPos = 10;
int RedTurnPos = 15;
int WhiteTurnPos = 20;

//     MotorEncoder(int mInput, int mOutput, int pCw, int pCcw, int pE, int sw)
MotorEncoder turnMotor(modInput, modOutput, turnCcw, turnCw, turnPulse1, rsTurn);
MotorEncoder vertMotor(modInput, modOutput, vertDown, vertUp, vertPulse1, rsVert);
MotorEncoder horMotor(modInput, modOutput, horFor, horBack, horPulse1, rsHor);

void setup()
{
  delay(1000);
  Serial.begin(9600);
  delay(1000);

  while (!P1.init())
  {
    Serial.print("Waiting for connection...");
  }
  Serial.println("Connected!!!");
  vertMotor.Home();
  horMotor.Home();
  turnMotor.Home();

  P1.writeDiscrete(true, modOutput, comp);
}

bool WarehouseReady()
{
  return P1.readDiscrete(modInput, wareReady);
}
bool blueReady()
{
  return P1.readDiscrete(modInput, waitBlue);
}
bool whiteReady()
{
  return P1.readDiscrete(modInput, waitWhite);
}
bool redReady()
{
  return P1.readDiscrete(modInput, waitRed);
}
bool turnTo(int d)
{
  return turnMotor.MoveTo(d);
}
bool vertTo(int d)
{
  return vertMotor.MoveTo(d);
}
bool horTo(int d)
{
  return horMotor.MoveTo(d);
}
void Suction(bool s)
{
  P1.writeDiscrete(s, modOutput, valvac);
}
void SignalWarehouse(bool s)
{
  P1.writeDiscrete(s, modOutput, wareSig);
}


void loop()
{
    switch (curState)
    {
    case Waiting:
      vertMotor.Home();
      horMotor.Home();
      turnMotor.Home();
      SignalWarehouse(false);

      if (whiteReady())
      {
        while (!turnTo(223))
        {
          ;
        }

        while (!horTo(180))
        {
          ;
        }

        while (!vertTo(390))
        {
          ;
        }

        delay(500);
        Suction(true);
        delay(500);
        curState = Dropoff;
      }

      else if (redReady())
      {
        while (!turnTo(180))
        {
          ;
        }

        while (!horTo(210))
        {
          ;
        }

        while (!vertTo(390))
        {
          ;
        }

        delay(500);
        Suction(true);
        delay(500);
        curState = Dropoff;
      }

      else if (blueReady())
      {
        while (!turnTo(150))
        {
          ;
        }

        while (!horTo(268))
        {
          ;
        }

        while (!vertTo(390))
        {
          ;
        }
        delay(500);
        Suction(true);
        delay(500);
        curState = Dropoff;
      }
      break;

    case Dropoff:
      Serial.println("Dropoff");
      vertMotor.Home();
      horMotor.Home();
      while (!turnTo(665))
      {
        ;
      }
      curState = CheckWarehouse;
      break;
    case CheckWarehouse:
      if (WarehouseReady())
      {
        SignalWarehouse(true);
        curState = Warehouse;
      }
      break;
    case Warehouse:
      while (!horTo(100))
      {
        ;
      }
      while (!vertTo(60))
      {
        ;
      }
      delay(500);
      Suction(false);
      delay(500);
      vertMotor.Home();

      curState = Waiting;
      break;
    }
      
}
