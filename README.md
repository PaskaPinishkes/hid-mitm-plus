# hid-mitm-plus

Sysmodule forked from hid-mitm by jakibaki to allow gamepads to be used from the network on the nintendo switch for services such as Parsec to be usable.

# Disclaimer:
This sysmodule is in **beta** and is a **work in progress**! Problems are to be expected. If you encounter any that aren't listed in the known issues section please report it [here](https://github.com/PaskaPinishkes/hid-mitm-plus/issues) if nobody else did already.
The code added in both the sysmodule and python proxy is a mess since this is my first time programming in Python and also my first time working on sysmodules, so any tips on how to fix it is welcomed!

# Features
* Use custom gamepads on your nintendo switch by using your pc as a proxy (Take in mind that this sysmodule isn't retrocompatible with the original hid-mitm proxies).

* Rebind buttons on your gamepads. [UNTESTED AFTER CHANGES]



# Setup
While hid-mitm will work perfectly fine on stock atmosphere and possibly reinx the recommended setup is to download the latest [Kosmos](https://github.com/AtlasNX/Kosmos) since it comes preconfigured with the awesome Kosmos Toolbox which allows you to easily configure hid-mitm.

On Kosmos it's just a matter of opening the Kosmos Toolbox, enabling the hid-mitm sysmodule and rebooting.

On Atmosphere, installing hid-mitm can be done by dragging the content of the [release](https://github.com/jakibaki/hid-mitm/releases).

This sysmodule wasn't tested on ReiNX and SX OS, so expect it to have issues on those CFW.

# Usage
## Custom gamepads
The way that custom gamepads work with hid-mitm is not by directly connecting the gamepad to your switch but by connecting your gamepad to your pc and then running the `input_pc.py` app which then in turn sends the input to your switch which will recognize it as an additional pro-controller. Right now, it is possible to emulate up to 4 controllers and in the future, 8. Please take in mind that everything in this sysmodule and script is experimental and was only tested with an Xbox controller.
The android and iOS apps made for the original hid-mitm weren't tested, but are most likely unsupported due to the changes made to support more controllers.

Hid-mitm-plus can also easily be repurposed to do scripted inputs (for example a "twitch plays").  
Check out the `input_pc.py` for some reference of how to talk to hid-mitm.

## Rebinding buttons [UNTESTED AND GETTING REMOVED IN NEXT RELEASE DUE TO HORIZON 10.0 FEATURING THIS]
Simply open the Kosmos toolbox, go to the sysmodule menu and continue to the hid-mitm menu from there (it's a simple button-press explained in the bottom bar)

After that a screen will pop up in which you can select the button you want to rebind and then what you want it to rebind for.  
After you're done, be sure to touch the button in the middle to save your changes.

You can also configure it manually by editing the `/config/hid_mitm/config.ini` file.



# Known issues
* Some games don't work properly (check Games with issues)
* If a controller is disconnected, it's not possible to reconnect it unless you restart the sysmodule/switch
* The sysmodule freezes at boot at some consoles (possibly because of kosmos?)

# Planned features

* Clean code to avoid unnecessary input delay (for example, join the 4 identical functions in the python script into 1)
* Version to emulate a Joy Con instead of the Pro Controller
* Home Button and Capture Button support
* Support for motion controls
* (maybe) USB support
* (maybe) vibration support

# Games with issues
* BOXBOY! + BOXGIRL! (Input is not detected for some reason)
* 1-2 Switch (Needs gyroscope and Joy Con support)
* Pokemon Let's Go Pikachu/Eevee (Needs gyroscope and Joy Con support)
* Super Mario Party (Needs gyroscope and Joy Con support)
* Mario & Sonic at the Olympic Games Tokyo 2020 (Some sports require the gyroscope, but most of the game is still playable)

# Support

The best way to get your questions answered is by asking me in Discord (ignacio#5841).

Please don't pollute the issue-tracker with general usage questions.

# Building

To build this sysmodule, all you need to do is install libnx 3.0 and use the make command.

# Credits
* jakibaki - original sysmodule (hid-mitm)
* PaskaPinishkes - code edits to support up to 4 players
* ReSwitched Discord Server - help in the support channels
