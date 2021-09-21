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

The program runs standalone without an installer.

## Supported input formats
For the source image, the program supports all the formats supported by WIC: png, bmp, jpg, etc. This does not currently include SVG.
Because the program uses hardware bitmaps, images must be under the Direct3D GPU texture limit. Generally, this means having neither width nor height exceeding 16384.

## Release
You don't need to build the code to run it. Download the release from the [releases page](https://github.com/clandrew/Spritesheet2Gif/releases).

The program runs on x86-64-based Windows environments. Tested on Windows 10.

## Build
The code is organized as a Visual Studio 2019 solution. The solution consists of two projects.
* Spritesheet2Gif - This is a .NET exe, a Windows Forms program written in C#. It has the UI.
* Native - This is a DLL written in C++. It handles image encoding and decoding using WIC.

The solution is built for x86 architecture. Tested on Windows 10 May 2019 Update.
