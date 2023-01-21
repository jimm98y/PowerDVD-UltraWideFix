#PowerDVD UltraWide Fix
Older verisons of PowerDVD were not able to stretch the movie to fill an ultra wide screen with 21:9 aspect ratio. This application was created to workaround the problem by changing the size of the video renderer window to fill the entire screen. Designed for PowerDVD 11, but it might work with other versions as well. Originally this project was published on Codeplex.

##Installation
###Launcher
Place PowerDVDLoader.exe into the PowerDVD installation folder. Change the target of PowerDVD shortcut to point to PowerDVDLoader.exe instead. Run PowerDVDLoader.exe, it will start PowerDVD and exit automatically when PowerDVD is closed.
###Standalone
Place into any other folder. Run PowerDVDLoader, it will run in the background. To close it, you must use the Task Manager and kill the process.

##Usage
Play any bluray movie with 21:9 aspect ratio on 21:9 screen. Press Ctrl + Alt + F to stretch the movie and hide black borders. Press Ctrl + Alt + F again to deactivate.