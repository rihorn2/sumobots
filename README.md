# Sumobots

### Getting started programming your Sumbot:

Your sumobot is powered by a small microcontroller called an Arduino. Installing the IDE and libraries will start you off with some examples. ZumoMotorExample is a simple beginner script for getting your bot to move. SumoCollisionDetect is a fully working (and complicated) sumo bot example.

Install arduino IDE (use installer, not Windows Universal App if you intend to develop in VSCode)
https://www.arduino.cc/en/Main/Software

Open IDE, Tools -> Manage Libraries; Search for Zumo and add Zumo Shield library.



#### Using Arduino IDE
Connect bot to computer via usb

In IDE -> Tools set Board to Arduino/Genuino Uno

set Tools -> Port to the COM port that looks right :)

File -> Examples -> ZumoShield -> ZumoMotorExample

Upload and validate that the board responds.


#### Using VS Code
follow the steps here (board type is Arduino/Genuino Uno): https://www.dmcinfo.com/latest-thinking/blog/id/9484/arduino-programming-with-vscode

Bring up the Arduino: Library Manager (ctrl + shift + p, type arduino) and add Zumo Shield library.

Under the examples, look for "Examples from Custom Libraries -> ZumoShield -> ZumoMotorExample"

Upload and validate that the board responds.
 


### Upgrading Sumobot Hardware (Optional)
 Teams can modify the I/O sensors used in the bot. By default, the bots have 6 downward facing reflection detectors. This can be reduced to 4 detectors, opening up 2 I/O ports for use with IR rangefinders. https://www.pololu.com/docs/0J57/2.c (requires soldering and board modification, don't be intimidated, this is what the Garage is for!)
 
 Teams can modify the motors to lower/higher torque and higher/lower speeds, or modify the wheels.
 
 ### SumoBot Build Rules:
 
 Robots need to be autonomous. No R/C robots. 

A robot may expand in size after a match begins, but must not physically separate into pieces, and must remain a single centralized robot.

No weapons or jamming devices. 

Jamming devices, such as IR LEDs intended to saturate the opponents IR sensors, are not allowed.

Devices that can store liquid, powder, gas or other substances for throwing at the opponent are not allowed.

Any flaming devices are not allowed.

Devices that throw things at your opponent are not allowed.

No sticky tires.  

Sticky substances to improve traction are not allowed. Tires and other components of the robot in contact with the ring must not be able to pick up and hold a standard 3"x5" index card for more than two seconds

No vacuum devices. 

### Competition Rules:

The referee will call the start, upon which the competitor can activate the robot. The robot has to wait 5 seconds after being activated before it can start.

There are 3 rounds:

- robots facing each other

- robots pointing outwards

- robots pointing sideways in opposite directions

Winning robot is the one who was able to push its opponent out the ring at least 2 times out of 3 rounds.

Each round lasts maximally 3 minutes.

If both robots are stationary for 10 seconds, the robot who stopped moving first will be declared the loser of the match.

A sumo ring is a black circle (diameter 77 cm), with a white outer edge.
