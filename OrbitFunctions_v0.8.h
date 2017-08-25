//--------------------------------
//Preprocessor Commands and Macros
//--------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define G 6.67408e-11
#define Bvalues B[i].name, &B[i].mass, &B[i].p.x, &B[i].p.y, &B[i].p.z, &B[i].v.x, &B[i].v.y, &B[i].v.z
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

//vector functions
vector VectorAdd(vector, vector);
vector VectorSubtract(vector, vector);
vector VectorMult(vector, double);
vector VectorMultVector(vector, vector);
vector VectorDivideBy(vector, double);
double VectorMagnitudeSquared(vector);

//Simulation functions
vector AccelerationSum(body *, vector, int, int);
int Simulate(config *);


//--------------------
//Function Definitions
//File IO Functions
//--------------------

//This function reads the user input settings from the csv file
//Also allocates memory for body list
void GetConfig(config *settings)
{
	FILE *in = fopen("InitialConditions.csv", "r");
	(in != NULL) ? FileFound() : FileNotFound();
	
	fscanf(in, "days, %hu,", &settings->days);
	fprintf(stderr, "\nSimulating orbits for %hu days.", settings->days);
	
	settings->totalbodies = CountList(in);
	fclose(in);
	
	if (settings->totalbodies < 2)
	{
		InsufficientObjects();
	}
	
	in = fopen("InitialConditions.csv", "r");
	settings->list = malloc(sizeof(body) * settings->totalbodies);
	GetList(in, settings->list);
	fclose(in);
}

//This function counts how many objects are stored in the list
int CountList(void *file)
{
	int n = 0;
	while (fscanf(file, "%*95s mass, %*lg position, %*lg, %*lg, %*lg velocity, %*lg, %*lg, %*lg") != EOF)
	{
		n++;
	}
	fprintf(stderr, "\nThe number of objects is %hu.", n);
	return n;
}

//This function reads the data stored on each body
void GetList(void *in, body B[])
{
	fscanf(in, "days, %*hu");
	int i = 0;
	while (fscanf(in, "%95s mass, %lg position, %lg, %lg, %lg velocity, %lg, %lg, %lg", Bvalues) != EOF)
	{
		if (0 > B[i].mass)
		{
			InvalidMass(&B[i]);
		}
		i++;
	}
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
	
	fprintf(sample, "GeoSat\n");
	fprintf(sample, "mass, 1200\n");
	fprintf(sample, "position, 3.58e7, 0, 0\n");
	fprintf(sample, "velocity, 0, 3070, 0");
	
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

//--------------------
//Function Definitions
//Coordinate Substitution Functions
//--------------------

//This function sets the coordinates to the center of mass of the system
//Also sets relative velocities
void SetRelative(body objects[], int n)
{
	double ThisObjectMass = 0;
	double TotalMass = 0;
	
	vector ThisObjectMomentum = {0, 0, 0};
	vector TotalMomentum = {0, 0, 0};
	
	vector ThisObjectMoment = {0, 0, 0};
	vector TotalMoment = {0, 0, 0};
	
	vector SystemVelocity = {0, 0, 0};
	vector SystemCenter = {0, 0, 0};
	
	fprintf(stderr, "\nSetting positions and velocities relative to center of system...");
	for (int i = 0; i < n; i++)
	{
		ThisObjectMass = objects[i].mass;
		TotalMass = TotalMass + ThisObjectMass;
		
		ThisObjectMomentum = VectorMult(objects[i].v, ThisObjectMass);
		TotalMomentum = VectorAdd(TotalMomentum, ThisObjectMomentum);
		
		ThisObjectMoment = VectorMult(objects[i].p, ThisObjectMass);
		TotalMoment = VectorAdd(TotalMoment, ThisObjectMoment);
	}
		
	SystemVelocity = VectorDivideBy(TotalMomentum, TotalMass);
	SystemCenter = VectorDivideBy(TotalMoment, TotalMass);
	fprintf(stderr, "\nNet velocity of system found to be (%.3lg, %.3lg, %.3lg)", SystemVelocity.x, SystemVelocity.y, SystemVelocity.z);
	fprintf(stderr, "\nCenter of mass of system found to be (%.3lg, %.3lg, %.3lg)", SystemCenter.x, SystemCenter.y, SystemCenter.z);
	
	for (int i = 0; i < n; i++)
	{
		objects[i].p = VectorSubtract(objects[i].p, SystemCenter);
		objects[i].v = VectorSubtract(objects[i].v, SystemVelocity);
		fprintf(stderr, "\nProperties of object \"%s\" set based on new coordinates.", objects[i].name);
	}	
}


//--------------------
//Function Definitions
//vector Functions
//--------------------

vector VectorDivideBy(vector V, double C)
{
	V.x = V.x / C;
	V.y = V.y / C;
	V.z = V.z / C;
	return V;
}

vector VectorAdd(vector V, vector W)
{
	V.x = V.x + W.x;
	V.y = V.y + W.y;
	V.z = V.z + W.z;
	return V;
}

vector VectorMult(vector V, double C)
{
	V.x = V.x * C;
	V.y = V.y * C;
	V.z = V.z * C;
	return V;
}

vector VectorSubtract(vector V, vector W)
{
	V.x = V.x - W.x;
	V.y = V.y - W.y;
	V.z = V.z - W.z;
	return V;
}

double VectorMagnitudeSquared(vector V)
{
	V.x = V.x * V.x;
	V.y = V.y * V.y;
	V.z = V.z * V.z;
	double Magnitude = V.x + V.y + V.z;
	return Magnitude;
}

//--------------------
//Function Definitions
//Simulation Functions
//--------------------

vector AccelerationSum(body *list, vector position, int i, int n)
{
	vector a_sum = {0, 0, 0};
	vector q = {0, 0, 0};
	double scalar;
	for (int j = 0; j < n; j++)
	{
		if (i != j)
		{
			q = VectorSubtract(list[j].p, position);
			scalar = list[j].mass * G * pow(VectorMagnitudeSquared(q), -1.5);
			a_sum = VectorAdd(a_sum, VectorMult(q, scalar));
		}
	}
	return a_sum;
}

int Simulate(config *settings)
{
	fprintf(stderr, "\nBeginning Simulation...\n\n");
	
	unsigned long sim_end_hours = settings->days * 24;
	
	unsigned long simtime_hours = 0;
	int simtime_minutes = 0;
	int simtime_seconds = 0;
	
	int n = settings->totalbodies;
	int i;
	
	vector *K1V = malloc(sizeof(vector) * n);
	vector *K2V = malloc(sizeof(vector) * n);
	vector *K3V = malloc(sizeof(vector) * n);
	vector *K4V = malloc(sizeof(vector) * n);
	
	vector *K1R = malloc(sizeof(vector) * n);
	vector *K2R = malloc(sizeof(vector) * n);
	vector *K3R = malloc(sizeof(vector) * n);
	vector *K4R = malloc(sizeof(vector) * n);
	
	vector *NewP = malloc(sizeof(vector) * n);
	vector *NewV = malloc(sizeof(vector) * n);
	
	vector K2V_pos = {0, 0, 0};
	vector K3V_pos = {0, 0, 0};
	vector K4V_pos= {0, 0, 0};
	
	double h = 1.0;
	double half_h = h / 2.0;
	double C = h / 6.0;
	
	vector pi;
	vector vi;
	
	vector sum_k;
	
	FILE *out = fopen("SimulationResultDOUBLE.csv", "w");
	
	while (simtime_hours < sim_end_hours)
	{
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
		
		for (i = 0; i < n; i++)
		{
			settings->list[i].p = NewP[i];
			settings->list[i].v = NewV[i];
		}
		simtime_seconds++;
		
		if (simtime_seconds == 60)
		{
			fprintf(out, "%.10lg, %.10lg, %.10lg, %.10lg, %.10lg, %.10lg, %.10lg, %.10lg, %.10lg\n", settings->list[0].p.x, settings->list[0].p.y,
			settings->list[0].p.z, settings->list[1].p.x, settings->list[1].p.y, settings->list[1].p.z, 
			settings->list[2].p.x, settings->list[2].p.y, settings->list[2].p.z);
			simtime_minutes++;
			simtime_seconds = 0;
			
			if (simtime_minutes == 60)
			{
				simtime_hours++;
				simtime_minutes = 0;
			}	
		}		
	}
	free(K1V);
	free(K2V);
	free(K3V);
	free(K4V);
	
	free(K1R);
	free(K2R);
	free(K3R);
	free(K4R);
	
	free(NewP);
	free(NewV);
	
	fclose(out);
	
	return(0);
}
