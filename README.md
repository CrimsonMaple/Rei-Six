# Rei-Six
[![License (GPL version 3)](https://img.shields.io/badge/license-GNU%20GPL%20version%203-red.svg?style=flat-square)](http://opensource.org/licenses/GPL-3.0)

*The original open source (N)3DS CFW!*


**Compiling:**

You'll need armips and firmtool added to your Path.

    make - Compiles Everything.
    make sighax - Compiles boot9Strap payload

Copy everything in 'out' folder to SD root and run!

**Recommended Setup and Instructions:**

EmuNand users now have to get their own firmware files. There will be instructions in this "guide" to do this.
The binary files can be pulled using [Decrypt9WIP](https://github.com/d0k3/Decrypt9WIP/releases)

To do this, launch Decrypt9WIP using your prefered launcher. I recommend [Fastboot3DS](https://github.com/derrekr/fastboot3DS/releases) for new users. 
However you can use other bootloaders like GodMode9 or Boot9Strap.

You can install Fastboot3DS using [OFI](https://github.com/d0k3/OpenFirmInstaller/releases) by d0k3.
To install Fastboot3DS you will need to download OFI and Fastboot3DS from their respective links.

On your 3ds' SD card create a folder called "OFI".

Now extract the contents of fastboot3DS.7z and place fastboot3DS.firm into "OFI".

Now launch OpenFirmInstaller. This will now install Fastboot3DS over FIRM0. Making it your bootloader.

Now would be a good time to put Rei-Six's files onto your 3ds. Don't worry. It is simply drag + drop onto the root of the SD.

When the console reboots you will be greeted with a menu. 
Here you can create boot slots as well as do other things like backing up and restoring NAND files.

Select the "Boot setup..." option. Select "Setup [slot 1]". This is main payload.
This will be the first file to launch on your 3ds. Essentially your "boot.firm" file.

Click "Select [slot 1] firm", this will open up your SD.
Now select "boot.firm". This is Rei-Six.
Now you are returned back a screen. 

If "Set [slot 1] autoboot" is not set; set it.

Now press "B", to go back a screen, and go to "Change boot mode..."

Set this to "Set quiet mode". This will make it so Rei-Six autoboots. 
If you need to access the menu, you can press "HOME" while your 3ds is being turned on.

You can also set other boot slots to other payloads like GodMode9.

**Emunand Setup:**

Now with Rei-Six 1.2 firmware files are no longer in the release zip.

You have to get them yourself. Here I will show you how.
Launch Decrypt9WIP using any method.

Go to "SysNAND options -> Miscellaneous... -> NCCH FIRMs Dump"

This will dump the 3ds firmware to your SD for your 3ds.
Take the resulting binary files and rename them to:

NATIVE_FIRM -> "native_firmware.bin"
TWL_FIRM -> "twl_firmware.bin"
AGB_FIRM -> "agb_firmware.bin"

Throw the renamed bin files into "/rei" on your SD card, and you are good to go!
Maybe someone can make a script to automate this. *shrug*

**Features:**

* B9S support!

* Emunand/Patched Sysnand (with '.:Rei' version string)

* Sig checks disabled

* Reboot Patch

* AGB/TWL Patches

* Flashcart Whitelist Patch

* LGY FIRM patches

* Firm partition update protection

* Ability to modify splash screen

* Ability to modify process patches
    * Region free CIAs
    * Region free Carts
    * Auto updates disabled
    * EShop update check disabled
    * RO verification disabled
    * and [MORE](https://reisyukaku.org/3DS/ReiNand/patches)

**Credits:**
 
[Link to Credits on our Wiki!](https://github.com/CrimsonMaple/Rei-Six/wiki/Credits)
