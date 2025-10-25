#include <Arduino.h>
#include <P1AM.h>
#include <MotorEncoder.h>

enum MachineStates
{
    Waiting,
    GetEmptyBox,
    LoadEmptyBox,
    BoxToConveyor,
    UnloadEmptyBox,
    LightSense,
    LoadCargo,
    DropCargo,
    PickUpBox,
    BoxSet,
    BoxHome
};

MachineStates curState = Waiting;

// Modules
int modInput = 1;
int modOutput = 2;
int modAnalog = 3;

// Inputs
int rpbHor = 1;        // Reference pushbutton horizontal	1	1
int lbIn = 2;          // Light barrier inside	2	1
int lbOut = 3;         // Light barrier outside	3	1
int rpbVert = 4;       // Reference pushbutton vertical	4	1
int horPulse1 = 5;     // Encoder horizontal pulse 1
int horPulse2 = 6;     // Encoder horizontal pulse 2
int vertPulse1 = 7;    // Encoder vertical pulse 1
int vertPulse2 = 8;    // Encoder vertical pulse 2
int rpbProbeFront = 9; // Reference pushbutton probe arm front	9	1
int rpbProbeRear = 10; // Reference pushbutton probe arm rear	10	1
int sigArm = 11;       // Input Signal From Robot Arm	11	1

// Outputs
int convFor = 1;       // Conveyor belt motor forwards	1	2
int convBack = 2;      // Conveyor belt motor backwards	2	2
int motorToRack = 3;   // Motor horizontal towards rack	3	2
int motorToConv = 4;   // Motor horizontal towards conveyor belt	4	2
int vertDown = 5;      // Vertical motor down	5	2
int vertUp = 6;        // Vertical motor up	6	2
int cantMotorFor = 7;  // Cantilever motor forwards	7	2
int cantMotorBack = 8; // Cantilever motor backwards	8	2
int outReady = 13;      // Output Ready To Robot Arm	9	2

// Other

int vertBoxPosition[] = {30, 30, 30, 225, 225, 225, 410, 410, 410};
int horBoxPosition[] = {960, 665, 370, 955, 665, 370, 955, 665, 370};
int box = 0;
int conveyorHeight = 365;

MotorEncoder vertMotor(modInput, modOutput, vertDown, vertUp, vertPulse1, rpbVert);
MotorEncoder horMotor(modInput, modOutput, motorToRack, motorToConv, horPulse1, rpbHor);

void ConveyorForward(bool s)
{
    P1.writeDiscrete(s, modOutput, convFor);
}
void ConveyorBack(bool s)
{
    P1.writeDiscrete(s, modOutput, convBack);
}
void ArmForward(bool s)
{
    P1.writeDiscrete(s, modOutput, cantMotorFor);
}
void ArmBack(bool s)
{
    P1.writeDiscrete(s, modOutput, cantMotorBack);
}
bool vertTo(int d)
{
    return vertMotor.MoveTo(d);
}
bool horTo(int d)
{
    return horMotor.MoveTo(d);
}

void PickupReady(bool s)
{
    P1.writeDiscrete(s, modOutput, outReady);
}

bool ArmFrontLimit()
{
    return P1.readDiscrete(modInput, rpbProbeFront);
}
bool ArmBackLimit()
{
    return P1.readDiscrete(modInput, rpbProbeRear);
}
bool HorizontalLimit()
{
    return P1.readDiscrete(modInput, rpbHor);
}
bool VerticalLimit()
{
    return P1.readDiscrete(modInput, rpbVert);
}
bool RobotSignal()
{
    return P1.readDiscrete(modInput, sigArm);
}
bool LightInside()
{
    return P1.readDiscrete(modInput, lbIn);
}
bool LightOutside()
{
    return P1.readDiscrete(modInput, lbOut);
}

void setup()
{
    delay(1000);
    Serial.begin(9600);
    delay(1000);

    while (!P1.init())
    {
        Serial.print("Waiting for connection...");
        delay(500);
    }
    Serial.println("Connected!!!");
    if (!ArmBackLimit())
    {
        ArmForward(false);
        ArmBack(true);
    }
    if (ArmBackLimit())
    {
        ArmForward(false);
        ArmBack(false);
    }
    vertMotor.Home();
    horMotor.Home();
}

void loop()
{
    switch (curState)
    {
    case Waiting:
        PickupReady(false);
        if (!ArmBackLimit())
        {
            ArmForward(false);
            ArmBack(true);
        }
        if (ArmBackLimit())
        {
            ArmForward(false);
            ArmBack(false);
            Serial.println(box);
            vertMotor.Home();
            horMotor.Home();
            curState = GetEmptyBox;
        }

        break;

    case GetEmptyBox:
        Serial.println("Get Box");
        while (!vertTo(vertBoxPosition[box]))
        {
            ;
        }

        while (!horTo(horBoxPosition[box]))
        {
            ;
        }

        curState = LoadEmptyBox;
        break;

    case LoadEmptyBox:
        delay(500);
        if (!ArmFrontLimit())
        {
            ArmForward(true);
            ArmBack(false);
        }
        if (ArmFrontLimit())
        {
            ArmForward(false);
            ArmBack(false);
            while (!vertTo(vertBoxPosition[box] - 20))
                ;

            curState = BoxToConveyor;
        }

        break;
    case BoxToConveyor:
        Serial.println("box to Conveyor");
        if (!ArmBackLimit())
        {
            ArmForward(false);
            ArmBack(true);
        }
        if (ArmBackLimit())
        {
            ArmForward(false);
            ArmBack(false);
            vertMotor.Home();
            horMotor.Home();
            curState = UnloadEmptyBox;
        }
        break;

    case UnloadEmptyBox:
        if (!ArmFrontLimit())
        {
            ArmForward(true);
            ArmBack(false);
        }
        if (ArmFrontLimit())
        {
            ArmForward(false);
            ArmBack(false);
            while (!vertTo(conveyorHeight))
            {
                ;
            }
            curState = LightSense;
            break;

        case LightSense:

            if (LightInside())
            {
                Serial.println("No Cargo");
                ConveyorForward(false);
                ConveyorBack(false);
            }
            if (!LightInside())
            {
                Serial.println("Yes Cargo");
                ConveyorForward(true);
                ConveyorBack(false);

                curState = LoadCargo;
            }
        }
        break;
    case LoadCargo:
        Serial.println("loading Cargo");

        if (LightOutside())
        {
            ConveyorForward(true);
            ConveyorBack(false);
        }
        if (!LightOutside())
        {
            ConveyorForward(false);
            ConveyorBack(false);
            delay(100);
            PickupReady(true);
            if (RobotSignal())
            {
                delay(5000);
                curState = DropCargo;
            }
        }
        break;
    case DropCargo:

        if (LightInside())
        {
            ConveyorForward(false);
            ConveyorBack(true);
        }
        if (!LightInside())
        {
            ConveyorForward(false);
            ConveyorBack(false);
            delay(100);
            curState = PickUpBox;
        }

        break;
    case PickUpBox:
        if (!ArmBackLimit())
        {
            ArmForward(false);
            ArmBack(true);
        }
        if (ArmBackLimit())
        {
            ArmForward(false);
            ArmBack(false);
        }
        vertMotor.Home();

        while (!horTo(horBoxPosition[box]))
        {
            ;
        }
        while (!vertTo(vertBoxPosition[box] - 20))
        {
            ;
        }
        curState = BoxSet;
        break;
    case BoxSet:
        if (!ArmFrontLimit())
        {
            ArmForward(true);
            ArmBack(false);
        }
        if (ArmFrontLimit())
        {
            ArmForward(false);
            ArmBack(false);
            while (!vertTo(vertBoxPosition[box]))
            {
                ;
            }
            curState = BoxHome;
        }
        break;
    case BoxHome:

        if (!ArmBackLimit())
        {
            ArmForward(false);
            ArmBack(true);
        }
        if (ArmBackLimit())
        {
            ArmForward(false);
            ArmBack(false);
            box = (box + 1) % 9;
            curState = Waiting;
        }
        break;
    }
}