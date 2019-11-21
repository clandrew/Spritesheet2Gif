# Spritesheet2Gif

This is a utility for converting a spritesheet into an animated GIF.

![Example image](https://raw.githubusercontent.com/clandrew/Spritesheet2Gif/master/Demo/Video.gif "Example image")

Features:
* Flexible sprite size
* Ability to set number of loop iterations, or loop forever
* Ability to scrub through frames
* Zoom in / out
* Option for configuring the animation speed

## Build
The code is organized as a Visual Studio 2019 solution. The solution consists of two projects.
* Spritesheet2Gif - This is a .NET exe, a Windows Forms program written in C#. It's responsible for the UI.
* Native - This is a DLL written in C++. It's responsible for image encoding and decoding using WIC.
