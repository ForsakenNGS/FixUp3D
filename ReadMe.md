Fix Up 3D
=========

FixUp3D is a project which extends the functionality of the UP! printer software from pp3dp / tiertime.

The software is implemented as a intelligent filter proxy between the UP! software and the WinUSB driver.
It intercepts all data sent and received via USB and changes values like temperature, supported layer height, speeds, etc. on the fly.

Installation is fairly simple. You just have to compile and place the resulting new winusb.dll inside of the UP directory ("C:\Program Files\UP").

Upon startup of the UP software an additional window with new settings will pop up.


Please note: This project is in a very early state. A lot of tests need to be done before you can use it safely. Use it on your own risk as it might damage the nozzle or other parts of the printer.