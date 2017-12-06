# Orbit-Simulation

**Summary**

This program is used to predict the paths of objects in multi-body orbital systems via Newton's laws of motion. This is accomplished through the use of a nonadaptive 4th-order RK method with a small timestep of one second. This simulation DOES NOT account for relativistic effects.

**How to input data**

This program looks for a file titled "IntialConditions.csv", and loads the properties of the system from this file. It is only necessary to enter the number of days for the simulation to calculate, and the parameters of each body. These parameters include a name, a mass, a position in x, y, z coordinates, and a velocity in x, y, z coordinates. 

If the program does not detect the file when it runs, it will ask to create a sample file with three objects: Earth, Moon, and GeoSat. Open this sample file with any text editor (notepad, nano, gedit, etc) to view the specific format required for data entry. Unknown bugs may be encountered if the specific format is not followed. 

**How to read data**

The x, y, and z positions of each object in the listed order are given in an output file under the name of SimulationResult.csv. Each line in the output file corresponds to the new conditions after one minute of time, so the file may be quite large for long runs. This file can be opened in any spreadsheet for plotting and analysis.

The PlotOrbits.m file is included as a quick script for plotting in Matlab or Octave. Simply adjust the example file path to the location of SimulationResult.csv.

**Compilation**

This program is intended to be compiled via gcc using the following command:

>gcc -std=c11 -O1 OrbitMain_v0.9.c OrbitFunctions_v0.9.h -lm -o Orbit.exe

For greater performance if necessary, this compilation can be modified with the following command:

>gcc -std=c11 -Ofast -march=native OrbitMain_v0.9.c OrbitFunctions_v0.9.h -lm -o Orbit.exe

On newer hardware, it may be necessary to download and install a later version of gcc for the -march=native command to have any effect.

It may be possible to compile with a compiler other than gcc, but I have not tried this.

**Known bugs**

If InitialConditions.csv is not formatted correctly, the input will not be read as intended. This can occur if the file is edited in Excel or similar software.
