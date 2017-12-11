# Orbit-Simulation

![Example system of 200 objects simulated for 60 days](https://i.imgur.com/ZukMWrZ.png)

<sub><sup>Example of a system plotted in MATLAB</sup></sub>

**Summary**

This program is used to predict the paths of objects in multi-body orbital systems via Newton's laws of motion. This is accomplished through the use of a nonadaptive 4th-order RK method with a small timestep of one second. This simulation DOES NOT account for relativistic effects.

**How to input data**

This program looks for a file titled "IntialConditions.ini", and loads the properties of the system from this file. It is only necessary to enter the number of days for the simulation to calculate, and the parameters of each body. These parameters include a name, a mass, a position in x, y, z coordinates, and a velocity in x, y, z coordinates. 

If the program does not detect the file when it runs, it will ask to create a sample file with five objects as a template. Open this sample file with any text editor (notepad, nano, gedit, etc) to view the specific format required for data entry. Unknown bugs may be encountered if the specific format is not followed. 

**How to read data**

The x, y, and z positions of each object are listed in their own files. Each line in the output file corresponds to the new conditions after one minute of time, so the files may be quite large (>1MB each) for long runs. These file can be opened in any spreadsheet for plotting and analysis.

The OrbitPlot.m file is included as a quick script for plotting in Matlab or Octave. Copy this code into Matlab, and simply adjust the example file path to the location of each of the csv files.

**How to compile**

This program is intended to be compiled via gcc using the following command:

>gcc -std=c11 -O3 OrbitMain_v1.0.c OrbitFunctions_v1.0.h -lm -lpthread -o Orbit.exe

For greater performance if necessary, this compilation can be modified with the following command:

>gcc -std=c11 -Ofast -march=native OrbitMain_v1.0.c OrbitFunctions_v1.0.h -lm -lpthread -o Orbit.exe

On newer PCs, it may be necessary to download and install a later version of gcc for the -march=native command to have any effect.

It may be possible to compile with a compiler other than gcc, but I have not tried this.

**How to run**

If compiled using either of the two instructions above, run the program via command line with either of the two commands: Orbit.exe on Windows, or ./Orbit.exe on Mac/Linux. Use the -m option (e.g. "./Orbit.exe -m") to enable multithreaded processing.

Unless the number of bodies to simulate is quite large, multithreaded processing is likely to be slower than the default of singlethreaded processing.

**Known bugs**

If InitialConditions.ini is not formatted correctly, the input will not be read as intended. This can occur if the file is edited in Excel or similar software.

**Version history**

v1.0 (December 11, 2017): 
* Fixed bugs with multithreaded version, and added "-m" command-line option to enable multithreading
* Changed input file from InitialConditions.csv to InitialConditions.ini. 
* Changed output file from SimulationResult.csv, to multiple output files for each object, [objectname].csv. 
* Updated Matlab script to compensate for this change
* Removed processing time message
* Removed list of object properties before and after simulation
* Removed many other console messages

v0.9 (December 5, 2017): 
* Updated Matlab script to generate 3D plot
* Fixed Matlab bug that prevented more than 3 objects from being plotted
* Fixed bug that allowed a divide by zero NaN error to occur
* Edited sample file generated to include 2 more objects: ISS and MoonReconOrbiter

v0.8 (August 25, 2017):
* First shared version of Orbit Sim, uploaded only single-threaded variant
* Included all necessary files to compile and run
* Edited parts of code from v0.61 for readibility and ease of use

v0.7 (August 24, 2017):
* Configured processing of RK method to share work across threads
* Assigned appropriate number of threads based on objects and CPUs available
* Introduced new, strange bugs

v0.61 (August 15, 2017):
* Converted float data types from long double to double 
* Improved performance by ~5x

v0.6 (August 1, 2017):
* Experimented with compiler optimizations, improved performance by up to ~10x
* Tested code across platforms and OSes

v0.51 (July 28, 2017):
* Fixed bugs in RK method

v0.5 (June 9th, 2017):
* Wrote RK method
* Wrote Matlab code to plot output and test for bugs

v0.4 (June 6, 2017):
* Wrote and tested coordinate substitution functions

v0.3 (May 30, 2017):
* Moved some functions to header file, clearing up space in main

v0.2 (May 24, 2017):
* Implemented File I/O
* Enabled program to read data

v0.1 (May 18, 2017):
* Started project, began development of data structures
