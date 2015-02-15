Fix Up 3D
=========

FixUp3D is a project which extends the functionality of the UP! printer Windows software from pp3dp / tiertime.

The software is implemented as an intelligent filter proxy between the UP! software and the WinUSB driver.
It intercepts all data sent and received via USB and changes values like temperature, supported layer height, speeds, etc. on the fly.

Installation is fairly simple. You just have to compile and place the resulting new winusb.dll inside of the UP directory ("C:\Program Files\UP").

Upon startup of the UP software an additional window with new settings will pop up.

![Screen shot of the extra window](pictures/FixUp3D_ExtraWindow_01.png?raw=true "Screen shot of the extra window")


Please note: This project is in an early state. A lot of tests need to be done before you can use it safely. Use it on your own risk as it might damage the nozzle or other parts of the printer.


Pre-Compiled Version
====================
You can find a precompiled version here: <a href="/downloads/winusb.dll">FixUp3D winusb.dll</a>

Just download the file "winusb.dll" and copy it to the UP software directory (e.g. "C:\Program Files\Up\")

When you start the UP software the next time, the extra window will come up automatically.

In case you want to uninstall FixUp3D just delete or rename the "winusb.dll" file in the UP software directory.


Compilation
===========

You should install MinGW and Eclipse CDC in order to compile the project. A simple step by step guide for installation can be found here:

https://www.ics.uci.edu/~pattis/common/handouts/mingweclipse/mingweclipse.html

