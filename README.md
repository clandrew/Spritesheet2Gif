# Spritesheet2Gif

This is a utility for converting a spritesheet into an animated GIF. 

I used to use random web apps to get this functionality, but became put off by how they felt weirdly bloated and littered with ads. Locally-installed programs were not much better. Seems like over time it's become harder to find freeware you can trust. 

Since the functionality I needed was simple I made this. And even better, I can add a bunch of extra features (e.g., frame scrubbing) that I had wanted which were not present in the web apps.

![Example image](https://raw.githubusercontent.com/clandrew/Spritesheet2Gif/master/Demo/Video.gif "Example image")

Features:
* Flexible sprite size
* Ability to set number of loop iterations, or loop forever
* Ability to scrub through frames
* Zoom in / out
* Option for configuring the animation speed
* Option to reload spritesheet from disk
* Ability to save GIF at a different size than the source sprite size

The frame scrubbing can be handy for creating animated sprites even if you're not saving GIFs.

## Build
The code is organized as a Visual Studio 2019 solution. The solution consists of two projects.
* Spritesheet2Gif - This is a .NET exe, a Windows Forms program written in C#. It's responsible for the UI.
* Native - This is a DLL written in C++. It's responsible for image encoding and decoding using WIC.

The solution is built for x86/x64 architecture. Tested on Windows 10 May 2019 Update.
