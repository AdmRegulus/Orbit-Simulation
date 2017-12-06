#include "OrbitFunctions_v0.9.h"
#include <time.h>

//Compile and run using the following command:
/*
gcc-7 -std=c11 -Ofast -march=native OrbitMain_v0.9.c OrbitFunctions_v0.9.h -lm -o Orbit.exe && ./Orbit.exe
*/

int main()
{
	//Create configuration and get user settings from file
	config settings = {.list = NULL};
	GetConfig(&settings);
	
	//Start clock
	clock_t CPU_time_1 = clock();

	//Set objects to their relative position
	SetRelative(settings.list, settings.totalbodies);
	
	//Print new object properties
	Print(settings.list, settings.totalbodies);
	
	//Change masses to masses times GRAV_CONST
	ComputeMG(settings.list, settings.totalbodies);
	
	//Start Simulation
	Simulate(&settings);
	
	//Print end results
	Print(settings.list, settings.totalbodies);
	
	//End clock
	clock_t CPU_time_2 = clock();
	
	//Print processing time
	fprintf(stderr, "\nSimulation complete. Processing time is %.3f seconds.\n", (CPU_time_2 - CPU_time_1) / 1e6);
	
	//Free unused memory and quit
	free(settings.list);
	return 0;
}
