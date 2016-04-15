TopHat Kobo - Quick Start Guide
================================

Disclaimer
----------
User of TopHat program is solely responsible for its use. The software is
only a navigational aid and does not relieve the pilot in any way with responsibility for
compliance with aviation regulations.

Turn on the device
------------------
Kobo device with installed TopHat starts up in the same way as regular e-book software. The first possible method is
using a mechanical switch on the top of the housing. The switch should be moved to the left for couple of seconds
and then release. The second method of starting up of the device is to connect it to the charger via USB.
Regardless of how the process of starting up is initiated software startup process is indicated by the LED next to the switch.
After initialisation there will be control buttons visible on the screen. Buttons allows controlling the program.

Welcome screen allows you to select the operating mode of the device. There are two buttons `FLY` and `SIM` visible
lightly below the centre of the screen. The first mode is intended for actual flight. In this mode, program reads the
position from the GPS receiver and actual navigation calculations are carried out. The second mode labeled `SIM` is designed
to learn to use the navigation. In this mode, instead of getting the actual location from GPS receiver position and glider
movement are simulated.

Turning off the device
----------------------

To exit from the navigation program it is required to press menu button `M` (menu) four times and then press
`Exit program` button. Before switching off program asks you to confirm if you really want to exit. After that
main mode selection screen will be visible.

The second method require to use a mechanical switch the top of the housing. The button must be slide to the left.
In this case, the program also will ask you to confirm that you actually want to quit. After that
main mode selection screen will be visible.

To turn device completely and preserve energy stored in the battery it is required to turn off also startup menu. 
Again there are two possible options. The first one is simply by pressing `Poweroff` button in the lower right
corner of the screen.
The second way is to slide mechanical power button to the left for a moment.

Before the navigation turns power off completely it displays a list of the most recent flight with takeoff and landing date.
At the right, bottom corner of the screen _powered off_ will be printed. This screen remains visible even after 
completed shutdown. Electronic paper displays does not consume electricity in the state.


Charging Device Battery 
-----------------------

Kobo has an internal battery. It is able to run navigation from internal battery for a few hour. 
Charging is possible by USB plug, like most of today's phones. During the long flights it is required to provided
power from glider battery or from another source, eg. a portable battery _Power Bank_.

Kobo with deeply discharged battery does not work, it also can not be turn on. In deep discharged battery it is
usually required to charge battery for some time before you can turn it on. It can take even 1 hour.

Configure GPS
-------------

Kobo Reader has a built-in serial port it allows you to connect a GPS receiver or electronic variometer. It is required to
disassemble device and solder connector to soldering pads prepared by the manufacturer of the device. You need to remember
that the built-in serial port operates at 3.3V voltage levels so the voltage converter is required.

In addition, it is possible to connect a GPS via a USB port using a USB <-> serial port converter. Kobo can
function as a USB host, however, it can not provide the power supply to external devices. It means
that power need to delivered to USB device by UBS Y cable. Power connected through the Y cable can power both USB device
and Kobo at the same time.

GPS source selection can be reached after pressing the `M` (menu) button three times and then selecting a cog symbol. 

To configure input devices selects `Device` button. TopHat / XCSoar allows
getting navigational data simultaneously from more than one input device. List of all input channels is visible
on the screen. After selecting a channel it is possible to configure it by pressing the `Edit` button.

_TopHat / XCSoar accepts and processes the GPS data only in flight mode (Fly). No data is displayed when the program runs
in simulation mode. It is worth remembering when configuring input ports._

### Internal Serial Port

Kobo Mini built-in serial port is visible as `ttymxc0`. The speed should be selected depending on the type of the
connected GPS receiver. The standard GPS speed is 4800 baud, newer types
GPS receivers can work with a higher speed (often 9600). After selecting the correct transfer speed status of device
should change to _Connected_.

### Serial port USB

Serial devices connected via a USB <-> serial port converter can be seen as `ttyUSB0`. Setting transfer speed is the same
as in case of built-in serial port.

Data Synchronization
--------------------

Kobo TopHat allows you to synchronize data with an external computer. There are two methods
data synchronization. Use USB memory stick or by direct connection to a computer.

### Synchronizing via USB stick

USB memory can be connected by USB Y cable, external power supply is required to power up USB memory. If TopHat is on the main
mode selection screen (as after turning power on) it is able to recognise new connected USB device. Once USB memory device is
connected to the Kobo TopHat navigation sycnchornization menu should appear on the screen. This sychrobization menu 
contains several buttons that allow you to select the direction of data transfer.

For the purposes of synchronization TopHat is using a directory named XCSoarData located on a connected USB drive.

#### Transfer flight logs to USB drive

_`Download Flights to USB card`_ - copy all flight logs saved by TopHat to a subdirectory `XCSoarData / logs`

#### Upload Tasks to Kobo

_`Upload Tasks`_ - copy tasks from the `XCSoarData / tasks` USB directory to the TopHat navigation.

#### Copy everything to a USB stick

_`Download Everything to USB card`_ - copy the entire contents of internal directories used by TopHat Kobo to USB memory.
This includes also a copy of the configuration files.

#### Replace TopHat directories by USB device content

_`Clean Kobo data directory and then upload everything to Kobo`_ - cleans the internal device and then copy USB memory
to TopHat.

### Connecting to a PC

TopHat navigation be connected to the PC by USB cable. Then from the main menu (welcome screen) you should choose
the `PC connect` button. After approval warning the device restarts and begins to act as a book reader. If it is not
registered just select `Do not have WiFi network`. After a while on the computer to which
Kobo is connected you should see the drive named `KOBOeReader`. On this drive you should be able to find
XCSoarData catalog containing the internal configuration files. 

Software Upgrade
----------------
To perform software updates you need to connect a USB drive like in the case of a file transfer as described above.
On the USB disk you should place a new version of installation package `KoboRoot.tgz`. After connecting the USB drive
TopHat navigation check the existence of the file, and in addition to the synchronisation options additional 
_`Upgrade Top Hat`_ button will appear. This button allows you to upgrade the software.

Emergency procedures - Kobo reboot
----------------------------------
In case of a problem with the TopHat navigation it is possible to restart it. The procedure is as follows:
- slide the button on the top of the device to the right, wait 10 seconds, release the button
- wait another 10 seconds
- move to the button to the right again and wait 2 seconds and then release the button again
If the battery is charged Kobo should start restart.

In addition, on the back of the Kobo is an additional mechanical _`reset`_ switch.
To access it first part of the back housing part needs to be remove. You can start it from the upper right corner where
housing corner is cut a little bit just for this purpose.
 
The _`reset`_ button is inside a tiny hole (about 1mm diameter). To press it you need some pin, it can be paper clip.

