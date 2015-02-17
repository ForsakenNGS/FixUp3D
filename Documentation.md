Known Bugs/Problems
===================
- The preheat timer is still quite hacky and may cause trouble with larger models. (See more below)

General
=======

For all input fields / adjustable values:
- Green color means default values will be used / are set
- Yellow color means a custom value is set and will be used
- Delete the input field's content to restore the default value

Configuration
=============
Currently the configuration file is stored in a folder named "FixUp3D" within your profiles documents folder. When closing the Up! Software the current settings and printsets will be saved into the "config.cfg" file and restored upon the next start. Beside that you can use the import/export button to save the current configuration into another file or load a configuration you exported before. When using an imported configuration be sure to export it again if you want to change the used file.

Heater Temperatures
===================

The heater temperatures are configurable for different layers individually:
- Layer 1: Temperature for the first layer. By default this is 10°C higher than layers 3+
- Layer 2: Temperature for the second layer. By default this is 5°C higher than layers 3+
- Layer 3+: Temperature for the 3rd and following layers. This temperature is also used for extrusion/withdraw.

Preheat timer
=============

WARNING! This feature is unfortunally not very reliable yet. Please read the last paragraph about the details if you want to use this feature.

Preheat timer: Delays print jobs by the given number of minutes. Use as following: (Keep in mind this feature is quite hacky atm.)
- Input the desired delay
- Start print job with the desired settings
- The software transfers the data to the printer and stops the job before it actually starts printing
- Preheating for the given delay starts (See the title of the FixUp3D window)
- After preheating is done, the printer starts the job

This feature is currently only working good for small prints. If the printer finishes heating before the transfer is complete it will fail and propably leave you with a permanent beeping until you trigger another beep (by initializing e.g.) or shutting down the printer. The easiest way around this is to disable the preheat timer, start the print job and cancel it right away. Now turn back on the preheat timer and just press print again. It should now cancel the now starting print job and properly preheat.

Print sets
==========

The print sets can be changed by simply replacing the values you want to change for the respective set. The sets are selectable within the printer settings as layer height. Because of that you should avoid having multiple sets with the same layer height. (Use multiple configuration files instead)
Known values and wild guesses:
- "Nozzle Diameter": The diameter of the printers nozzle.
- "Layer Thickness": The layer height for this set
- "Scan Width": Wild guess: The resulting width of a "line" when printing the outer hull / surface?
- "Scan Times": ??? Always 1.0 for Up Mini
- "Hatch Width":  Wild guess: The resulting width of a "line" when printing the inner hull / infill?
- "Hatch Space": Wild guess: A factor for the distance between the infill threads?
- "Support Width":  Wild guess: The resulting width of a "line" when printing the support?
- "Support Space": Wild guess: A factor for the distance between the support "walls"?
- "Support Layer": ??? Always 3.0 for Up Mini
- "Scan Speed": This is the speed used for printing the outer hull of the object. It only affects the outer of the 2 outlines the printer draws.
- "Hatch Speed": This seems to be the speed used for the inner hull and infill of the object. (To be verified)
- "Support Speed": This is obviously the speed for printing the support :)
- "Jump Speed": The speed used to jump from one position to another without printing.
- "Scan Scale": ??? Always 1.0 for Up Mini
- "Hatch Scale": ??? Always 1.05 for Up Mini
- "Support Scale": ??? Always 0.9 for Up Mini
- "Feed Scale": This seems to be the (base?) speed value for the extruder.

TODOs / Future Features
=======================
- More reliable preheat timer
- More comfortable configuration management (File > Open/Save/SaveAs/Recent)
- Renameable print sets
- Adding more print sets (up to 8 could be possible without major effort. to be checked!)
- Lower bed after the print is done
- Allow the window to be minimized or in background so it doesn't take up so much of the screen when you're working with a print
