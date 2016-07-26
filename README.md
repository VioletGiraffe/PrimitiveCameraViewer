# Primitive Camera Viewer

[![Build status](https://ci.appveyor.com/api/projects/status/4u7nh6ed0vxn34ad?svg=true)](https://ci.appveyor.com/project/VioletGiraffe/primitivecameraviewer)

Connects to a standard Windows camera device (web camera, external USB camera, video capture card etc.) and displays the live image from it. 

Features and functions:
* automatically goes to full screen when image is detected and hides when the image becomes invalid (blank);
* has settings to mirror the image horizontally and/or vertically;
* doesn't stay connected to the camera if there is no valid image (e. g. darkness). Instead, the camera is probed for an image every so often, and the app only stays connected when a valid image is detected (validity threshold is user-configurable).
