//--------------------------------
//Preprocessor Commands and Macros
//--------------------------------
#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>

#define GRAV_CONST 6.67408e-11
#define BVALUES B[i].name, &B[i].mass, &B[i].p.x, &B[i].p.y, &B[i].p.z, &B[i].v.x, &B[i].v.y, &B[i].v.z
#define Bposition B[i].p.x, B[i].p.y, B[i].p.z
#define Bvelocity B[i].v.x, B[i].v.y, B[i].v.z
#include <time.h>

//Global variable for thread synchronizer
pthread_barrier_t synchronizer;

//------------------
//Custom data types
//------------------

typedef struct
{
	double x;
	double y;
	double z;
} vector;

typedef enum {x = 0, y = 1, z = 2, end = 3} direction;

typedef struct
{
	char name[96];
	double mass;
	vector p;
	vector v;
} body;

typedef struct
{
	int days;
	int totalbodies;
	body *list;
} config;

typedef struct
{
	body *list;
	int totalbodies;
	int num;
	int* sim;
	FILE *writefile;
} ThreadData;

typedef struct
{
	int days;
	int* sim;
} TimeData;

//-------------------
//Function Prototypes
//-------------------

//File IO functions
void GetConfig(config *);
int CountList(void *);
void GetList(void *, body *);
void Print(body *, int);

//Error handling functions
void FileFound();
void FileNotFound();
void GenerateSampleFile();
void InsufficientObjects();
void InvalidMass(body *);
void CheckDays(int);
void ObjectsTooClose();
void BadMalloc();
void ThreadError();

//vector functions
vector VectorAdd(vector, vector);
vector VectorSubtract(vector, vector);
vector VectorMult(vector, double);
vector VectorMultVector(vector, vector);
vector VectorDivideBy(vector, double);
double VectorMagnitudeSquared(vector);

//Simulation functions
vector AccelerationSum(body *, vector, int, int);
void Simulate(config *);
void SimulateMultithread(config *);
void* TimekeeperThread(void *);
void* SimThread(void *);


//--------------------
//Function Definitions
//File IO Functions
//--------------------

//This function reads the user input settings from the csv file
//Also allocates memory for body list
void GetConfig(config *settings)
{
	//Set input file for reading
	FILE *in = fopen("InitialConditions.ini", "r");
	
	//Go to FileFound or FileNotFound depending on result of fopen
	(in != NULL) ? FileFound() : FileNotFound();
	
	//Scan for number of days to simulate and store the result in settings
	fscanf(in, "days, %d,", &settings->days);
	
	//Check to make sure days is a valid number
	CheckDays(settings->days);
	
	//Print message listing days read
	fprintf(stderr, "\nSimulating orbits for %d days.", settings->days);
	
	//Run CountList function to determine number of objects in list
	//This number is stored in settings
	settings->totalbodies = CountList(in);
	
	//Then close file
	fclose(in);
	
	//If less than two bodies were counted, go to appropriate function
	if (settings->totalbodies < 2)
	{
		InsufficientObjects();
	}
	
	//Otherwise continue by opening file again
	in = fopen("InitialConditions.ini", "r");
	
	//Allocate memory for the number of bodies counted previously
	settings->list = malloc(sizeof(body) * settings->totalbodies);
	
	//If allocation failed, go to BadMalloc function
	if (settings->list == NULL)
		BadMalloc();
	
	//Populate the memory with the data from each body
	GetList(in, settings->list);
	
	//Then close file
	fclose(in);
}

//This function counts how many objects are stored in the list
int CountList(void *file)
{
	//Start counter at zero
	int n = 0;
	
	//Increment with each read until end of file
	while (fscanf(file, "%*s mass, %*g position, %*g, %*g, %*g velocity, %*g, %*g, %*g") != EOF)
	{
		n++;
	}
	
	//Print number of objects counted and return this number
	fprintf(stderr, "\nThe number of objects is %d.", n);
	return n;
}

//This function reads the data stored on each body
void GetList(void *in, body B[])
{
	//Read number of days again to go through buffer
	fscanf(in, "days, %*d");
	
	//Index used for object number in the array
	int i = 0;
	
	//Reads data into the allocated memory location until end of file
	while (fscanf(in, "%95s mass, %lg position, %lg, %lg, %lg velocity, %lg, %lg, %lg", BVALUES) != EOF)
	{
		//Check for valid mass
		if (0 > B[i].mass)
		{
			InvalidMass(&B[i]);
		}
		i++;
	}
	//When complete, print message to user
	fprintf(stderr, "\nListed object properties have been read. Read complete.");
}

//Displays the conditions of each body
void Print(body B[], int n)
{
	printf("\n\n\t=============== Object Properties ===============");
	int i = 0;
	while (i < n)
	{ 
		printf("\n\t%s's position is: (%.3lg, %.3lg, %.3lg) m", B[i].name, Bposition);
		printf("\n\t%s's velocity is: (%.3lg, %.3lg, %.3lg) m/s\n", B[i].name, Bvelocity);
		i++;
	}
	printf("\t=================================================\n");
}


//--------------------
//Function Definitions
//Error Handling Functions
//--------------------

//Action taken if file is read as intended.
void FileFound()
{
	fprintf(stderr, "\nFile Found. Reading data...");
}

//Action taken if file is not read as intended.
void FileNotFound()
{
	fprintf(stderr, "\nError: file \"InitialConditions.ini\" not found. Could not read data.");
	fprintf(stderr, "\nWould you like to generate a sample file? (Y/N)\n");
	
	//Give user option to generate a sample initial conditions file
	char UserChoice;
	scanf("%c", &UserChoice);
	switch (UserChoice)
	{
		case 'Y':
		case 'y':
			GenerateSampleFile();
			printf("\nTerminating program.");
			exit(0);
			break;
		case 'N':
		case 'n':
		default:
			printf("\nTerminating program.");
			exit(0);
			break;
	}		
}

void BadMalloc()
{
	fprintf(stderr, "\nError: Unable to allocate memory.");
	fprintf(stderr, "\nTerminating program.");
	exit(0);
}

void CheckDays(int d)
{
	//If days is more than zero, do nothing
	if (d > 0)
	{
		return;
	}
	//Otherwise print error message and exit
	else
	{
		fprintf(stderr, "\nError: Days to simulate is not a positive integer.");
		fprintf(stderr, "\nChange the days to a nonzero positive value and restart.");
		fprintf(stderr, "\nTerminating program.");
		exit(0);
	}
}

//This function creates a sample input file
void GenerateSampleFile()
{
	FILE *sample = fopen("InitialConditions.ini", "w");
	
	fprintf(sample, "days, 27\n\n");
	
	fprintf(sample, "Earth\n");
	fprintf(sample, "mass, 5.97e24\n");
	fprintf(sample, "position, 0, 0, 0\n");
	fprintf(sample, "velocity, 0, 0, 0\n\n");
	
	fprintf(sample, "Moon\n");
	fprintf(sample, "mass, 7.34e22\n");
	fprintf(sample, "position, 3.84e8, 0, 0\n");
	fprintf(sample, "velocity, 0, 1000, 0\n\n");
	
	fprintf(sample, "GeostationarySatellite\n");
	fprintf(sample, "mass, 1200\n");
	fprintf(sample, "position, 3.58e7, 0, 0\n");
	fprintf(sample, "velocity, 0, 3070, 0");
	
	fprintf(sample, "InternationalSpaceStation\n");
	fprintf(sample, "mass, 419455\n");
	fprintf(sample, "position, 740626.73, -6644976.48, 1151109.69\n");
	fprintf(sample, "velocity, 4724.433862, 1545.169511, 5838.010655\n\n");
	
	fprintf(sample, "LunarReconOrbiter\n");
	fprintf(sample, "mass, 1000\n");
	fprintf(sample, "position, 3.84e8, 0, 1.787e6\n");
	fprintf(sample, "velocity, -1600, 1000, 0\n\n");
	
	fclose(sample);
	printf("\nSample file \"InitialConditions.ini\" has been created.");
}

//This function ends the program if there are not enough objects
void InsufficientObjects()
{
	fprintf(stderr, "\n\nThe number of objects listed is less than two. This simulation requires at least two bodies.");
	fprintf(stderr, "\nAdd more objects to the list in \"InitialConditions.ini\" and restart the program.");
	fprintf(stderr, "\nTerminating program.");
	exit(0);
}

//This function ends the program if an invalid mass is detected
void InvalidMass(body *B)
{
	fprintf(stderr, "\nError: the mass of %s is zero or negative. Mass is %.3lg kg.", B->name, B->mass);
	fprintf(stderr, "\nChange the mass to a nonzero positve value and restart the program.");
	fprintf(stderr, "\nTerminating program.");
	exit(0);
}

void ObjectsTooClose()
{
	static int MessageGiven = 0;
	if (!MessageGiven)
	{
		fprintf(stderr, "\nWarning: two objects are separated by a distance less than 1000 m.");
		fprintf(stderr, "\nCollision is anticipated. Accuracy of results is compromised.");
		MessageGiven = 1;
	}
}

void ThreadError()
{
	fprintf(stderr, "\nError: worker thread could not be created or joined.");
	fprintf(stderr, "\nTerminating program.");
	exit(0);
}


//--------------------
//Function Definitions
//Coordinate Substitution Functions
//--------------------

//This function sets the coordinates to the center of mass of the system
//Also sets relative velocities
void SetRelative(body objects[], int n)
{
	//Initialize used quantities to zero
	double ThisObjectMass = 0;
	double TotalMass = 0;
	
	vector ThisObjectMomentum = {0, 0, 0};
	vector TotalMomentum = {0, 0, 0};
	
	vector ThisObjectMoment = {0, 0, 0};
	vector TotalMoment = {0, 0, 0};
	
	vector SystemVelocity = {0, 0, 0};
	vector SystemCenter = {0, 0, 0};
	
	//Print message to user
	fprintf(stderr, "\nSetting positions and velocities relative to center of system...");
	for (int i = 0; i < n; i++)
	{
		//Increment through object list to find total mass
		ThisObjectMass = objects[i].mass;
		TotalMass = TotalMass + ThisObjectMass;
		
		//Also total momentum of system for relative velocity
		ThisObjectMomentum = VectorMult(objects[i].v, ThisObjectMass);
		TotalMomentum = VectorAdd(TotalMomentum, ThisObjectMomentum);
		
		//And Moment of system for center of mass
		ThisObjectMoment = VectorMult(objects[i].p, ThisObjectMass);
		TotalMoment = VectorAdd(TotalMoment, ThisObjectMoment);
	}
	
	//Get values for entire system by dividing total quantities by total system mass
	SystemVelocity = VectorDivideBy(TotalMomentum, TotalMass);
	SystemCenter = VectorDivideBy(TotalMoment, TotalMass);
	
	//Print results to user
	fprintf(stderr, "\nNet velocity of system found to be (%.3lg, %.3lg, %.3lg).", SystemVelocity.x, SystemVelocity.y, SystemVelocity.z);
	fprintf(stderr, "\nCenter of mass of system found to be (%.3lg, %.3lg, %.3lg).", SystemCenter.x, SystemCenter.y, SystemCenter.z);
	
	//Set each object's coordinates to a new coordinate relative to the entire system
	for (int i = 0; i < n; i++)
	{
		objects[i].p = VectorSubtract(objects[i].p, SystemCenter);
		objects[i].v = VectorSubtract(objects[i].v, SystemVelocity);
	}	
}

//Function to multiply each body's mass by G
void ComputeMG(body objects[], int n)
{
	for (int i = 0; i < n; i++)
	{
		objects[i].mass = objects[i].mass * GRAV_CONST;
	}
}


//--------------------
//Function Definitions
//vector Functions
//--------------------

//Function for a vector divided by a scalar
vector VectorDivideBy(vector V, double C)
{
	double C2 = 1/C;
	V.x = V.x * C2;
	V.y = V.y * C2;
	V.z = V.z * C2;
	return V;
}

//Function for a vector added to another vector
vector VectorAdd(vector V, vector W)
{
	V.x = V.x + W.x;
	V.y = V.y + W.y;
	V.z = V.z + W.z;
	return V;
}

//Function for a vector multiplied by a scalar
vector VectorMult(vector V, double C)
{
	V.x = V.x * C;
	V.y = V.y * C;
	V.z = V.z * C;
	return V;
}

//Function for a vector subtracted by another vector
vector VectorSubtract(vector V, vector W)
{
	V.x = V.x - W.x;
	V.y = V.y - W.y;
	V.z = V.z - W.z;
	return V;
}

//Function for the magnitude (a scalar) squared
double VectorMagnitudeSquared(vector V)
{
	V.x = V.x * V.x;
	V.y = V.y * V.y;
	V.z = V.z * V.z;
	return (V.x + V.y + V.z);
}


//--------------------
//Function Definitions
//Simulation Functions
//--------------------

//Function to find net acceleration on a body by iterating through list of other bodies
//Needs list, current position of body, number of itself, and number of total bodies
vector AccelerationSum(body *list, vector position, int i, int n)
{
	//Initialize variables to zero
	vector a_sum = {0, 0, 0};
	vector q = {0, 0, 0};
	
	double MagSquared;
	double MagNegCubed;
	double scalar;
	
	//Iterate through list of bodies, for each one not itself
	for (int j = 0; j < n; j++)
	{
		if (i != j)
		{
			//q is the distance vector from the jth body to the current one
			q = VectorSubtract(list[j].p, position);
			
			//If the distance <1000m, print error before continuing
			if ((MagSquared = VectorMagnitudeSquared(q)) < 1e6)
			{
				ObjectsTooClose();
				MagSquared = 1e6;
			}
			
			//Convert quantity from ^2 to ^-3, then multiply by mass and G
			MagNegCubed = pow(MagSquared, -1.5);
			scalar = list[j].mass * MagNegCubed;
			
			//The net acceleration will be the sum of all these vectors times their scalar
			a_sum = VectorAdd(a_sum, VectorMult(q, scalar));
		}
	}
	//Return the sum of accelerations
	return a_sum;
}

//Function to begin simulation
void Simulate(config *settings)
{
	//Alert user to start of simulation
	fprintf(stderr, "\nBeginning Simulation...\n");
	fprintf(stderr, "This may take some time. Please wait.");
	
	//Create a very big integer to store the number of hours to simulate
	unsigned long sim_end_hours = settings->days * 24;
	
	//Create integers to store the time currently simulated
	unsigned long simtime_hours = 0;
	int simtime_minutes = 0;
	int simtime_seconds = 0;
	
	//Get the number of bodies for faster iteration
	int n = settings->totalbodies;
	int i;
	
	//Allocate space for 10 vectors for each object
	vector *VectorSpace = malloc(sizeof(vector) * n * 10);
	
	//Go to malloc error if vector space is not created
	if (VectorSpace == NULL)
	{
		BadMalloc();
	}
	
	//Define vector lists within allocated space using pointer arithmetic
	vector *K1V = VectorSpace + 0 * n;
	vector *K2V = VectorSpace + 1 * n;
	vector *K3V = VectorSpace + 2 * n;
	vector *K4V = VectorSpace + 3 * n;
	
	vector *K1R = VectorSpace + 4 * n;
	vector *K2R = VectorSpace + 5 * n;
	vector *K3R = VectorSpace + 6 * n;
	vector *K4R = VectorSpace + 7 * n;
	
	vector *NewP = VectorSpace + 8 * n;
	vector *NewV = VectorSpace + 9 * n;
	
	//Vectors for altered positions for each coefficient
	vector K2V_pos = {0, 0, 0};
	vector K3V_pos = {0, 0, 0};
	vector K4V_pos= {0, 0, 0};
	
	//Set multipliers for RK method
	double h = 1.0;
	double half_h = h / 2.0;
	double C = h / 6.0;
	
	//Initial position and initial velocity vectors
	vector pi;
	vector vi;
	
	//Vector for sum of K coefficients
	vector sum_k;
	
	//Create space for array of file out pointers
	FILE **out = malloc(sizeof(FILE*) * n);
	
	//Output file
	for (int i = 0; i < n; i++)
	{
		out[i] = fopen(strcat(settings->list[i].name, ".csv"), "w");
	}
	
	//Loop until time reaches end
	while (simtime_hours < sim_end_hours)
	{
		
		//Loop for each body in the list
		for (i = 0; i < n; i++)
		{
			pi = settings->list[i].p;
			vi = settings->list[i].v;
		
			K1V[i] = AccelerationSum(settings->list, pi, i, n);
			K1R[i] = vi;
		
			K2V_pos = VectorAdd(pi, (VectorMult(K1R[i], half_h)));
			K2V[i] = AccelerationSum(settings->list, K2V_pos, i, n);
			K2R[i] = VectorAdd(vi, VectorMult(K1V[i], half_h));
		
			K3V_pos = VectorAdd(pi, (VectorMult(K2R[i], half_h)));
			K3V[i] = AccelerationSum(settings->list, K3V_pos, i, n);
			K3R[i] = VectorAdd(vi, VectorMult(K2V[i], half_h));
		
			K4V_pos = VectorAdd(pi, VectorMult(K3R[i], h));
			K4V[i] = AccelerationSum(settings->list, K4V_pos, i, n);
			K4R[i] = VectorAdd(vi, VectorMult(K3V[i], h));
		
			sum_k = VectorAdd(VectorAdd(K1V[i], VectorMult(K2V[i], 2.0)), VectorAdd(VectorMult(K3V[i], 2.0), K4V[i]));
			NewV[i] = VectorAdd(vi, VectorMult(sum_k, C));
		
			sum_k = VectorAdd(VectorAdd(K1R[i], VectorMult(K2R[i], 2.0)), VectorAdd(VectorMult(K3R[i], 2.0), K4R[i])); 
			NewP[i] = VectorAdd(pi, VectorMult(sum_k, C));
			
		}
		
		//For each object in the list, update positions and velocities
		for (i = 0; i < n; i++)
		{
			settings->list[i].p = NewP[i];
			settings->list[i].v = NewV[i];
		}
		//Increment seconds
		simtime_seconds++;
		
		//If one minute has passed...
		if (simtime_seconds == 60)
		{
			//Print each object's position to its out file
			for (int k = 0; k < n; k++)
			{
				fprintf(out[k], "%.10lg, %.10lg, %.10lg,\n", settings->list[k].p.x, settings->list[k].p.y, settings->list[k].p.z);
			}

			//Increment time, reset seconds counter
			simtime_minutes++;
			simtime_seconds = 0;
			
			if (simtime_minutes == 60)
			{
				simtime_hours++;
				simtime_minutes = 0;
			}	
		}		
	}
	
	//Free the space used by the vectors
	free(VectorSpace);

	//Close the writing files
	for (int k = 0; k < n; k++)
	{
		fclose(out[k]);
	};
	free(out);

	return;
}

void SimulateMultithread(config *settings)
{
	//Set simulate to 1, or true
	int simulate = 1;
	fprintf(stderr,"\nBeginning simulation...\n");
	fprintf(stderr, "This may take some time. Please wait.");
	
	//Initialize thread barrier for synchronization
	//Total threads is the number of bodies, plus one for the time thread
	pthread_barrier_init(&synchronizer, NULL, settings->totalbodies + 1);
	
	//Set ThreadArray to dynamically allocated space
	pthread_t *ThreadArray = NULL;
	ThreadArray = (pthread_t*) malloc(sizeof(pthread_t) * settings->totalbodies);
	
	//Call badmalloc function if thread pointers could not be set
	if (ThreadArray == NULL)
	{
		BadMalloc();
	}
	
	//Create reference to arguments for sim threads
	ThreadData *ThreadArg;
	
	//Allocate space for write files
	FILE **WriteFiles = NULL;
	WriteFiles = (FILE**) malloc(sizeof(FILE**) * settings->totalbodies);
	
	//Check for malloc error
	if (WriteFiles == NULL)
	{
		BadMalloc();
	}
	
	//for each body in the list, assign a thread to a body to simulate
	for (int i = 0; i < settings->totalbodies; i++)
	{
		//Allocate memory for thread data arguments for current thread
		ThreadArg = NULL;
		ThreadArg = (ThreadData*) malloc(sizeof(ThreadData));
		
		//Call badmalloc function if thread arg pointers could not be set
		if (ThreadArg == NULL)
		{
			BadMalloc();
		}
		
		//Otherwise, continue by populating ThreadArg struct with appropriate data
		ThreadArg->sim = &simulate;
		ThreadArg->list = settings->list;
		ThreadArg->totalbodies = settings->totalbodies;
		ThreadArg->num = i;
		
		//Open file
		WriteFiles[i] = fopen(strcat(settings->list[i].name, ".csv"), "w");
		ThreadArg->writefile = WriteFiles[i];
		
		//Create the pthread and assign it to SimThread, with ThreadArg cast as void pointer
		if (pthread_create(&ThreadArray[i], NULL, SimThread, (void*) ThreadArg) == -1)
		{
			//If there was an error, go to appropriate function
			ThreadError();
		}
	}
	
	//Create a time thread
	pthread_t TimeThread;
	
	//Create time thread argument
	TimeData TimeArg = {.days = settings->days, .sim = &simulate};
	
	//Assign time thread to timekeeper task
	if (pthread_create(&TimeThread, NULL, TimekeeperThread, (void*) &TimeArg) == -1)
	{
		//If there was an error, go to appropriate function
		ThreadError();
	}
	
	//Pointer used only for joining threads
	void* end;
	
	//Iteratively join threads here
	for (int i = 0; i < settings->totalbodies; i++)
	{
		if (pthread_join(ThreadArray[i], &end) == -1)
		{
			//If there was an error, go to appropriate function
			ThreadError();
		}
	}
	
	//Join time thread too
	if (pthread_join(TimeThread, &end) == -1)
	{
		//If there was an error, go to appropriate function
		ThreadError();
	}
	
	pthread_barrier_destroy(&synchronizer);
	free(WriteFiles);
}

//Function for simulating an object
void* SimThread(void *arg)
{
	//Re-cast passed arguments as ThreadData struct
	ThreadData *argument = (ThreadData*) arg;
	
	//Save important values from ThreadData to thread's stack
	int i = argument->num;
	int n = argument->totalbodies;
	int* sim = argument->sim;
	body *list = argument->list;
	
	//Declare variables used for RK calculation
	vector pi = argument->list[i].p;
	vector vi = argument->list[i].v;
	
	vector K2V_pos;
	vector K3V_pos;
	vector K4V_pos;
	
	vector K1V;
	vector K2V;
	vector K3V;
	vector K4V;
	
	vector K1R;
	vector K2R;
	vector K3R;
	vector K4R;
	
	vector sum_RK;
	
	//Counter to determine how often to print
	int counter = 0;
	
	//Set multipliers for RK method
	double h = 1.0;
	double half_h = h / 2.0;
	double C = h / 6.0;
	
	//Until the value is set to zero, the simulation loop continues
	//Begin the RK iteration loop
	while (*sim)
	{		
		K1V = AccelerationSum(list, pi, i, n);
		K1R = vi;
		
		K2V_pos = VectorAdd(pi, (VectorMult(K1R, half_h)));
		K2V = AccelerationSum(list, K2V_pos, i, n);
		K2R = VectorAdd(vi, VectorMult(K1V, half_h));
			
		K3V_pos = VectorAdd(pi, (VectorMult(K2R, half_h)));
		K3V = AccelerationSum(list, K3V_pos, i, n);
		K3R = VectorAdd(vi, VectorMult(K2V, half_h));
		
		K4V_pos = VectorAdd(pi, VectorMult(K3R, h));
		K4V = AccelerationSum(list, K4V_pos, i, n);
		K4R = VectorAdd(vi, VectorMult(K3V, h));
		
		sum_RK = VectorAdd(VectorAdd(K1V, VectorMult(K2V, 2.0)), VectorAdd(VectorMult(K3V, 2.0), K4V));
		vi = VectorAdd(vi, VectorMult(sum_RK, C));
		
		sum_RK = VectorAdd(VectorAdd(K1R, VectorMult(K2R, 2.0)), VectorAdd(VectorMult(K3R, 2.0), K4R)); 
		pi = VectorAdd(pi, VectorMult(sum_RK, C));
		
		//Assess if 60 seconds have been processed
		if (counter == 60)
		{
			//if so, reset counter and print a line at the current spot
			counter = 0;
			fprintf(argument->writefile, "%.10lg, %.10lg, %.10lg,\n", pi.x, pi.y, pi.z);
		}
		
		//Wait here for synchronization
		pthread_barrier_wait(&synchronizer);
		
		//update new values for other threads to see
		list[i].p = pi;
		list[i].v = vi;
		counter++;
		
		//Then wait again for synchronization
		pthread_barrier_wait(&synchronizer);
	}
	
	//close file once simulation is complete
	fclose(argument->writefile);
	
	//free memory allocated to this thread
	free(arg);
	
	//return nothing useful
	return NULL;
}

void *TimekeeperThread(void *arg)
{
	//cast argument back as TimeData
	TimeData *argument = (TimeData*) arg;
	
	//declare variables needed for timekeeping
	int *simulate = argument->sim;
	unsigned long EndHour = 24 * argument->days;
	unsigned long CurrentHour = 0;
	int CurrentSecond = 0;
	
	//loop for simulation
	while (CurrentHour < EndHour)
	{
	
		//Increment time by one second
		CurrentSecond = CurrentSecond + 1;
		
		//If one hour has passed, increment the hour and reset seconds
		if (CurrentSecond == 3600)
		{
			CurrentSecond = 0;
			CurrentHour = CurrentHour + 1;
		}
		
		//Wait for worker threads to compute new vectors
		pthread_barrier_wait(&synchronizer);	
		
		//Wait for worker threads to update values 
		pthread_barrier_wait(&synchronizer);
	}
	
	//Wait for worker threads to compute new vectors
	pthread_barrier_wait(&synchronizer);
	
	//set simulation condition to end
	*simulate = 0;
	
	//Wait for worker threads to update new values
	pthread_barrier_wait(&synchronizer);
	
	//Return nothing useful when finished, as nothing is needed
	return NULL;
}
