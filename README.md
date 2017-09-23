# Rei-Six
[![License (GPL version 3)](https://img.shields.io/badge/license-GNU%20GPL%20version%203-red.svg?style=flat-square)](http://opensource.org/licenses/GPL-3.0)

*The original open source (N)3DS CFW!*


**Compiling:**

You'll need armips, bin2c and firmtool added to your Path. [HERE](https://reisyukaku.org/downloads/buildtools.zip) are the pre-compiled binaries for Windows.

    make - Compiles Everything.
    make sighax - Compiles boot9Strap payload

Copy everything in 'out' folder to SD root and run!


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
 
 Cakes team for teaching me a few things and just being helpful in general! And for ROP/mset related code, and crypto libs.
    
 3DBREW for saving me plenty of reverse engineering time.
    
 Patois/Cakes for CakesBrah.
 
 Yifanlu for custom loader module!
 
 Steveice10/Gudenaurock for helping a lot with arm11 stuff!
 
 Normmatt for sdmmc.c and generally being helpful!
 
 Luma Dev Team for their AGB/TWL patches, and being generally helpful! No, Seriously.
 
 MidKid and Delebile for the reboot code.
 
 CrimsonMaple for continuing ReiNAND.
    
 Reisyukaku for coding the original ReiNAND.
 
 Lilith Valentine for helping to reboot the project and testing.
 
 The community for your support and help!
