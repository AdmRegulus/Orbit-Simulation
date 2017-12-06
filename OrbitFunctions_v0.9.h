//--------------------------------
//Preprocessor Commands and Macros
//--------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define GRAV_CONST 6.67408e-11
#define BVALUES B[i].name, &B[i].mass, &B[i].p.x, &B[i].p.y, &B[i].p.z, &B[i].v.x, &B[i].v.y, &B[i].v.z
#define Bposition B[i].p.x, B[i].p.y, B[i].p.z
#define Bvelocity B[i].v.x, B[i].v.y, B[i].v.z


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


//--------------------
//Function Definitions
//File IO Functions
//--------------------

//This function reads the user input settings from the csv file
//Also allocates memory for body list
void GetConfig(config *settings)
{
	//Set input file for reading
	FILE *in = fopen("InitialConditions.csv", "r");
	
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
	in = fopen("InitialConditions.csv", "r");
	
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
	fprintf(stderr, "\nError: file \"InitialConditions.csv\" not found. Could not read data.");
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
	FILE *sample = fopen("InitialConditions.csv", "w");
	
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
	fprintf(sample, "velocity, 4724.433862, 1545.169511, 5838.010655");
	
	fprintf(sample, "LunarReconOrbiter\n");
	fprintf(sample, "mass, 1000\n");
	fprintf(sample, "position, 3.84e8, 0, 1.787e6\n");
	fprintf(sample, "velocity, -1600, 1000, 0");
	
	fclose(sample);
	printf("\nSample file \"InitialConditions.csv\" has been created.");
}

//This function ends the program if there are not enough objects
void InsufficientObjects()
{
	fprintf(stderr, "\n\nThe number of objects listed is less than two. This simulation requires at least two bodies.");
	fprintf(stderr, "\nAdd more objects to the list in \"InitialConditions.csv\" and restart the program.");
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
	fprintf(stderr, "\nNet velocity of system found to be (%.3lg, %.3lg, %.3lg)", SystemVelocity.x, SystemVelocity.y, SystemVelocity.z);
	fprintf(stderr, "\nCenter of mass of system found to be (%.3lg, %.3lg, %.3lg)", SystemCenter.x, SystemCenter.y, SystemCenter.z);
	
	//Set each object's coordinates to a new coordinate relative to the entire system
	for (int i = 0; i < n; i++)
	{
		objects[i].p = VectorSubtract(objects[i].p, SystemCenter);
		objects[i].v = VectorSubtract(objects[i].v, SystemVelocity);
		fprintf(stderr, "\nProperties of object \"%s\" set based on new coordinates.", objects[i].name);
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
	fprintf(stderr, "\nBeginning Simulation...\n\n");
	
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
	
	//Output file
	FILE *out = fopen("SimulationResult.csv", "w");
	
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
			//Print each object's position to the out file
			for (int k = 0; k < n; k++)
			{
				fprintf(out, "%.10lg, %.10lg, %.10lg,", settings->list[k].p.x, settings->list[k].p.y, settings->list[k].p.z);
			}
			//Start a new line after all objects were printed
			fprintf(out, "\n");

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

	//Close the writing file
	fclose(out);

	return;
}
