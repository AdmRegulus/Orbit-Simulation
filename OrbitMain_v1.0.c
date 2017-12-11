#include "OrbitFunctions_v1.0.h"
#include <string.h>

int main(int argc, char *argv[])
{
	//Create configuration and get user settings from file
	config settings = {.list = NULL};
	GetConfig(&settings);

	//Set objects to their relative position
	SetRelative(settings.list, settings.totalbodies);
	
	//Change masses to masses times GRAV_CONST
	ComputeMG(settings.list, settings.totalbodies);
	
	//Determine if command-line argument is applied
	if ((argc >= 2) && (strcmp(argv[1], "-m") == 0))
	{
		//Start simulation on multiple threads
		fprintf(stderr, "\nRunning simulation on %d threads.", (settings.totalbodies + 1));
		SimulateMultithread(&settings);	
	}
	else
	{
		//Start simulation on a single thread
		fprintf(stderr, "\nRunning simulation on single thread.");
		Simulate(&settings);
	}
	
	//Print message to user
	fprintf(stderr, "\nSimulation complete.\n");
	
	//Free unused memory and quit
	free(settings.list);
	return 0;
}
