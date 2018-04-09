# ImageSearchEngine
Win32 Application for finding relevant photos base on the original one.

This project used OpenCV 3.3.0 as a main framework to develop, and required Visual Studio with C++ plugin to build.

To run this application, make sure to have:
- Visual Studio 2017
Install here: https://www.visualstudio.com/downloads/
- OpenCV 3.0 (If you are using OpenCV 2, you will have to reinstall it to a newer version)
Install here: https://opencv.org/releases.html

## After cloning this project, please follow these steps to set the new environment for OpenCV(If you have already known this process, please skip these steps):

- Step 1: Change the directory for include file in project, which can be found in: Right click to the Projects, choose Properties. In C/C++, change Additional Include Directories point to the include file
For example: C:\...\opencv\build\include

- Step 2: Change the link point to the lib directory: In Properties dialog box, choose Linker (below C/C++ in previous step), change Additional Library Directories to lib file in opencv
For example: C:\...\opencv\build\x64\cv14\lib

- Step 3: Change the Additional Dependencies for lib file in Linker>Input
For example: opencv_world330.lib (It is based on your opencv, find its name in lib file which was set in previous step)

## Compiling and Executing:
  - Make sure to change your Visual Studio to Release Mode and x64 flatforms.
  - Copy opencv_world330.dll to the ImageSearchEngine folder.
  
## Hope you enjoy!
  
# Reference Algorithm
[Image Search Engine using python and opencv](https://www.pyimagesearch.com/2014/12/01/complete-guide-building-image-search-engine-python-opencv/)

### Note:
- This application coudl only work for PNG photos (I will expand to JPG photos in the future)
- Need an image dataset? - Here is your free one (for researching purposes only): at [ETH Zurich](http://www.vision.ee.ethz.ch/datasets_extra/ZuBuD.tar.gz)
