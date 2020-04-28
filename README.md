# hid-mitm-plus

Sysmodule mitming the Horizon hid (human interface device) services.

# Disclaimer:
This sysmodule is in **beta** and is a **work in progress**! Problems are to be expected. If you encounter any that aren't listed in the known issues section please report it [here](https://github.com/PaskaPinishkes/hid-mitm-plus/issues) if nobody else did already.
The added code in both the sysmodule and python proxy is a mess, so any tips on how to fix it is welcomed!

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

Hid-mitm-plus can also easily be repurposed to do scripted inputs (for example a "twitch plays").  
Check out the `input_pc.py` for some reference of how to talk to hid-mitm.

## Rebinding buttons [UNTESTED]
Simply open the Kosmos toolbox, go to the sysmodule menu and continue to the hid-mitm menu from there (it's a simple button-press explained in the bottom bar)

After that a screen will pop up in which you can select the button you want to rebind and then what you want it to rebind for.  
After you're done, be sure to touch the button in the middle to save your changes.

You can also configure it manually by editing the `/config/hid_mitm/config.ini` file.



# Known issues
* Some games don't work properly (check Games with issues)
* If a controller is disconnected, it's not possible to reconnect it unless you restart the sysmodule/switch

# Planned features

* Version to emulate a Joy Con instead of the Pro Controller
* Support for motion controls
* (maybe) vibration support

# Games with issues
* BOXBOY! + BOXGIRL! (Input is not detected for some reason)
* 1-2 Switch (Needs gyroscope and Joy Con support)
* Pokemon Let's Go Pikachu/Eevee (Needs gyroscope and Joy Con support)
* Super Mario Party (Needs gyroscope and Joy Con support)

# Support

The best way to get your questions answered is by asking in the [AtlasNX discord](https://discord.gg/qbRAuy7).

Please don't pollute the issue-tracker with general usage questions.
