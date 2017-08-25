#include "OrbitFunctions_v0.8.h"
#include <time.h>

//Compile and run using the following command:
/*
gcc-7 -std=c11 -Ofast -march=native OrbitMain_v0.8.c OrbitFunctions_v0.8.h -lm -o Orbit.exe && ./Orbit.exe
*/

int main()
{
	config settings = {.days = 90, .totalbodies = 10, .list = NULL};
	GetConfig(&settings);
	
	clock_t CPU_time_1 = clock();

	SetRelative(settings.list, settings.totalbodies);
	Print(settings.list, settings.totalbodies);
	
	
	Simulate(&settings);
	Print(settings.list, settings.totalbodies);
	
	clock_t CPU_time_2 = clock();
	
	fprintf(stderr, "\nSimulation complete. Processing time is %.3f seconds.\n", (CPU_time_2 - CPU_time_1) / 1e6);
	
	free(settings.list);
	return 0;
}
