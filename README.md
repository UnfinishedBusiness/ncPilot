# ncPilot
Front-End Control Software for Xmotion CNC Controller. Native Cross-Platform (Windows, Linux, and MacOS)

ncPilot is written in Javascript and is executed by our [Xkernel](https://github.com/UnfinishedBusiness/Xkernel) VM. This allows
ncPilot to have the same code-base for every platform and allows the rapid-development for custom CNC applications.

ncPilot communicates via USB serial connection to XmotionFirmware which is GRBL based (ATMega328P) but heavily modified for CNC plasma use. I've added CRC communication redundancy and a builtin arc voltage torch height controller to the firmware. Due to a handfull of changed to the original GRBL comunication protocal, XmotionFirmware is not compatible with other GRBL front-ends.

# Features
- 2D Gcode Viewer
- Built-In Machine Parameter Editing (No editing of Config files necessary)
- Has a primary focus on CNC Plasma cutting and CNC Routing as a secondary focus
- Click 'n Point Jogging
- Arrow Key Jogging
- Click and Point Gcode Jump in (No need to manually find a gcode line to jump into)
- MDI input
- User configurable Layout
- NotchMaster (Conversational utility to create Gcode for CNC plasma Tube notching for people with A Axis's)
- Built in Gcode Editor to modify Gcode
- Built in torch touchoff routine. Instead of a gcode, use fire_torch [pierce_height] [pierce_delay] [cut_height]. torch_off to shut torch off

# Post Processors
- SheetCAM post processor is included with the XmotionFirmware repository at [XmotionFirmware](https://github.com/UnfinishedBusiness/XmotionFirmware)

# Simple plasma Gcode Program for slicing a sheet
```
fire_torch 0.160 1.6 0.075
G1 X0 Y0 F45
torch_off
M30
```
# Prepare a Arduino Uno (or other ATMega328p device with USB serial interface and arduino bootloader)
- Install avrdude (If on windows, install via MSYS2 with pacman -S mingw-w64-x86_64-avrdude)
- cd /c/Xmotion/ncPilot/extra/
- ./flash.sh YOUR_COM_PORT
- Windows will be COM0-6 or somthing, linux will be a /dev/ttyUSB or something like that

# Run ncPilot on Windows
- Open MSYS2
- cd /c/Xmotion/ncPilot
- ../Xkernel/Xkernel src/main.js
- This assumes you followed the build instructions on the Xkernel repository.
- Alternativly, instead of invoking the Xkernel bianry directly with the ncPilot main.js file as an argument, you can double - click on `run_no_console.vbs` which will launch ncPilot as well (as long as Xkernel was built and is at c:\Xmotion\Xkernel\Xkernel). You can right click on that vbs file and send to desktop as a shortcut as well.

# Preamble
I own [BadApple Machine & Fab](https://badappleproducts.com), our main business is manufacturing light-duty entry-level CNC plasma cutters. My goal in business is to lower costs for these types of machines without compromising quality or needed features. All of our machines have Teknic AC brushless servos, Rack and Pinion drives, Leadscrew Z, Floating Head, AVTHC and Waterpan. I've a great deal of time creating this control package (The circuit board we use in our pruduction machines, the software, the firmware, etc) because while Mach3, LinuxCNC, Flashcut, and a handfull of controls that already exists work great, they are all expensive and fairly complicated. We used to use a LinuxCNC based control on our productions machines with Messa hardware which is a great way to go but expensive and fairly difficult for us to add new features (We're always listening to what customers would like to see), also expensive and time consuming to replace (as opposed to now switching out a refurbished laptop and installing software). By making this group of software open source and available to hobbyist around the world who are instested in using DIY delopment boards like the Arduino UNO (ATMega328P) to build their CNC motion control project, I hope I can help you get your machine up and running with less time and money spent. Feel free to submit pull requests, I'd appreciate any help I can get. Thanks
Checkout our other projects
- [JetCAD](https://jetcad.io) (2D CAD in the cloud, geared for plasma design, offline version in the works)
- [Xkernel](https://github.com/UnfinishedBusiness/Xkernel) (Powerfull Cross-Platform Javascript VM the runs code that looks and feels like Arduino code)
- [XmotionFirmware](https://github.com/UnfinishedBusiness/XmotionFirmware) (GRBL based motion control for CNC plasma cutting, builtin arc voltage torch height control)
- [ncPilot](https://github.com/UnfinishedBusiness/ncPilot) (Front-End for XmotionFirmware. Runs on just about anything hardware or operating system wise. Has built-in Gcode Viewer, Live DRO, Click and Go way points, ctrl-click viewer program jump-ins)
