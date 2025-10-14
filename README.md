# About
This was ment as a tool to edit games made with DreamFactory. I was starting to work on it 3 years ago already but just dont have the time to actually finish it. Will I ever come back to this? Hopefully, but I don't know and this is why it was decided to upload its code and a very early build, so someone may continue on it or use it to learn more about DF in general.
NOTE: THIS IS VERY UNFINISHED AND JUST HERE FOR LEARNING PURPOSE, SO PLEASE IGNORE THE ISSUES TAB FOR THIS ABANDOD PROJECT

# What can it do?
Not so much honestly. I used it to make new sets in the past or modify them, but without knowing the internals its not very usueful. However, the least you can do is inspecting the SET files and partially the PUP files (but only the tree view). If you want to change the SETs logic, you can do so by changing the code in the scripts. You will find them in the tree view. Click them, a new window will pop up and save the changes. However, there is no error checking whatssoever. If something is wrong in the script, the set file will crash. Also if you do that, don't forget to also save the actual SET file.

# UI
The UI should look like this:
<img width="964" height="670" alt="grafik" src="https://github.com/user-attachments/assets/201564c2-4525-4c2e-accb-8d0571cb7f2c" />


The right window is the viewport and below a console to output text based information about the file. The viewport is used to load maps from the SET files. On top of that it will also draw additional information like waypoints, markers etc.

The left window will show the files structure as a tree view. Below is an infobox holding a table with values. Depending which item you click in the tree, it will refresh the table.
The "Apply changes" button can be ignored. Earlier it was ment to apply changes via the table, but it does it by other means now.

# Menu
File:
- Open File: It mostly supports SET files, but you can also load PUP files.
- Save File: Saves the file when finished. If you do that, I highly suggest you make a backup of the original file!
- Save File as: This will allow you to save the file directly with the new name
- Exit: Hit this to close. Mind that unsaved changes will be lost.

Resources:
- Read New Resources: I suggest you dont use it. This is used to change all image data of a set at once, but the images must be in a very special format with a special color palette and located in a "img" folder to make it work. If you are a programmer, check the cMain::OnReadNewRes function to understand it fully.
- Delete All Resources: Don't hit that unless you know what you are doing. This action will remove all image containers from the file.

Print Data
- Container Information: This will output additional info to the console

Edit:
- Scenes: This will open the scene editor, used for SET files
- Roads: This opens the road editor. A road is essentially the connection between two scenes.
- Change map: This will change the map, only useful if the SET will be extended or completly changed.
- SET UE Reference: You can ignore that. I was testing some stuff with Unreal Engine back then
- Use only one rotation table: You will probably not need that. Every scene uses 2 rotation tables, one for left and one for right turns. This option will make it so that all view images are referenced from only one rotation table. I used that to fix some loading errors that the steam version of TAOOT has.

# Libraries
To make the code run you need the DFfile library, which you can find in my other repository "DFET". Both programs access this same library.
For the UI you need wxWidgets.
Also use at least C++ 17 to compile.
hats all. With these two you should be all set.

# Anything else?
Nah, not really. I know its not great and if I ever come back to it I would probably rewrite the whole thing. The code is a mess, but if you are a programmer who finds this interesting, be my guest. You can also make pull requests if you want to work on it, so eventually its not gonna be abandod anymore. But of course you can also use the knowledge for your own projects.
