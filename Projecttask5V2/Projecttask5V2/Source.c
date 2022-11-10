#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include <math.h>
#include <Windows.h>

#define MAXTAXIS 4
#define ROADL 6
#define CAR 254 //black square symbol
#define NBSP 255 //space
#define XX 1
#define YY 100

#define NUL	0x00
#define ESC	0x1b
#define XXX	2
#define YYY	99
/*characters for printing*/
#define UL		218
#define LL		192
#define UR		191
#define LR		217
#define T		194
#define iT		193
#define H		196
#define S		32
#define V		179
#define VR		195
#define VL		180

/*defining directions*/
#define NORTH	1000
#define SOUTH	2000
#define EAST	3000
#define	WEST	4000

/*defining type*/
#define TOPLEFT		10000	
#define TOP			20000
#define TOPRIGHT	30000
#define LEFT		40000
#define MIDDLE		50000
#define RIGHT		60000
#define BOTTOMLEFT	70000
#define BOTTOM		80000
#define BOTTOMRIGHT 90000
/* DISPput character */

#define	DISP(x)	putc((x),stdout)
HANDLE Console;

bool getxy(int* mapxsize, int* mapysize);

struct building* initmap(int mapxsize, int mapysize);

void printmapcoords(struct building* buildingmap, int mapxsize, int mapysize);

void printmap(struct building* buildingmap, int mapxsize, int mapysize);

void printbuilding(int x, int y, int type, int charging, int parked);

void printroad(int xr, int yr, int direction);

void move_cursor(int row, int col);

int checkfreeparking(struct building* buildingmap, int xcoord, int ycoord);

int checkfreecharging(struct building* buildingmap, int xcoord, int ycoord);

void parktaxi(struct building* buildingmap, int xcoord, int ycoord);

void unparktaxi(struct building* buildingmap, int xcoord, int ycoord);

void chargetaxi(struct building* buildingmap, int xcoord, int ycoord);

void unchargetaxi(struct building* buildingmap, int xcoord, int ycoord);


struct building
{
	int x;
	int y;
	int parked;
	int parkedmax;
	int charging;
	int chargingmax;
	int xmax;
	int ymax;
};


typedef struct alltaxis
{
	int currentX, currentY;
	int endX, endY;
	bool arrive;
	int taxi_num; //just like taxi 1, 2, 3, 4...
}taxi;


taxi* init_taxi(int numtaxis);
bool moveCar(int* curX, int* curY, int* endX, int* endY, int counter, bool* skip);
void moveXright(int cX, int cY, int counter);
void moveXleft(int cX, int cY, int counter);
void moveYup(int cX, int cY, int counter);
void moveYdown(int cX, int cY, int counter);
void move_cursor(int row, int col);


int main(void)
{
	COORD scrsize;
	int row;
	int col;
	unsigned char ch;

	/* For console output via Windows */
	Console = GetStdHandle(STD_OUTPUT_HANDLE);
	/* Determine screen size */
	scrsize = GetLargestConsoleWindowSize(Console);

#ifdef DEBUG
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r); //stores the console's current dimensions

	MoveWindow(console, r.left, r.top, 1920, 1080, TRUE);
#endif

	scrsize = GetLargestConsoleWindowSize(Console);
	printf("Size: r: %d c: %d\n", scrsize.Y, scrsize.X);

	int mapxsize;
	int mapysize;
	if (getxy(&mapxsize, &mapysize))
	{
		printf("Size of map is %dx%d\n", mapxsize, mapysize);
	}
	else
	{
		printf("Size of map is invalid.");
	}
	struct building* buildingmap = initmap(mapxsize, mapysize);

	printmapcoords(buildingmap, mapxsize, mapysize);

	printmap(buildingmap, mapxsize, mapysize);
	move_cursor(XXX, YYY + 1);

	///////////////////////////////////////////////////////////////
	int numtaxis;
	printf("Enter number of taxis to be added (maximum of 4): ");
	bool stop = false;
	while (!stop)
	{
		scanf_s("%d", &numtaxis);
		if (numtaxis > MAXTAXIS)
			printf("Number of taxis entered is too large. Please enter a number between 1 - %d: ", MAXTAXIS);
		else if (numtaxis < 1)
			printf("Number of taxis entered is too small. Please enter a number between 1 - %d: ", MAXTAXIS);
		else stop = true;
	}

	taxi* taxis;
	taxis = init_taxi(numtaxis);

	//lets user input start and end coordinates for taxis;
	for (int i = 0; i < numtaxis; i++)
	{
		printf("Enter start coordinates for taxi %d in the form x y: ", taxis[i].taxi_num);
		scanf_s("%d %d", &taxis[i].currentX, &taxis[i].currentY);
		printf("Enter end coordinates for taxi %d in the form x y: ", taxis[i].taxi_num);
		scanf_s("%d %d", &taxis[i].endX, &taxis[i].endY);
	}

	bool stopall = false;
	bool* skip = false;
	while (!stopall) //everyloop is esentially a clock cycle
	{
		for (int j = 0; j <= ROADL; j++)
		{
			for (int i = 0; i < numtaxis; i++) //calls the move function for each taxi
			{
				taxis[i].arrive = moveCar(&taxis[i].currentX, &taxis[i].currentY, &taxis[i].endX, &taxis[i].endY, j, &skip);
				if ((checkfreeparking(buildingmap, taxis[i].endX, taxis[i].endY) > 0) && taxis[i].arrive && j == ROADL)
				{
					parktaxi(buildingmap, taxis[i].endX, taxis[i].endY);
					printmap(buildingmap, mapxsize, mapysize);
				}
			}

			if (!skip)
				Sleep(500);
		}
		skip = false;
		for (int j = 0; j < numtaxis; j++) //checks if each taxi arrived and asks user for new destination or quit
		{
			if (taxis[j].arrive == true)
			{
				move_cursor(1, 10);
				printf("\nTaxi %d has reached its destination.", taxis[j].taxi_num);
				printf("\nEnter new destination coordinates to continue or 0 0 to quit: ");
				scanf_s("%d %d", &taxis[j].endX, &taxis[j].endY);
				unparktaxi(buildingmap, taxis[j].currentX, taxis[j].currentY);
				printmap(buildingmap, mapxsize, mapysize);
			}
			if (taxis[j].endX == 0 && taxis[j].endY == 0) //quit if user enters (0,0)
				stopall = true;
		}
	}
	return 0;
}

taxi* init_taxi(int numtaxis)
{
	taxi* taxis;
	int tsize = sizeof(taxi);
	taxis = malloc(tsize * numtaxis);
	for (int i = 0; i < numtaxis; i++)
	{
		taxis[i].arrive = false;
		taxis[i].taxi_num = i + 1;
	}
	return taxis;
}

bool moveCar(int* curX, int* curY, int* endX, int* endY, int counter, bool* skip)
{
	int difX = *endX - *curX;
	int difY = *endY - *curY;
	//notice that the below structure first moves x and then moves y
	if (difY == 0 && difX == 0)
	{
		*skip = true;
		return true;
	}
	if (difX > 0) //need to move right
	{
		moveXright(*curX, *curY, counter); //passing by value
		if (counter == ROADL)
			(*curX)++;
		return false;
	}
	else if (difX < 0) //need to move left
	{
		moveXleft(*curX, *curY, counter); //passing by value
		if (counter == ROADL)
			(*curX)--;
		return false;
	}
	else if (difY > 0) //need to move up
	{
		moveYup(*curX, *curY, counter); //passing by value
		if (counter == ROADL)
			(*curY)++;
		return false;
	}
	else if (difY < 0) //need to move down
	{
		moveYdown(*curX, *curY, counter); //passing by value
		if (counter == ROADL)
			(*curY)--;
		return false;
	}
	else //both x and y are at the destination position
		return true;
}

void moveXright(int cX, int cY, int counter) //does not change values, only displays movement on screen
{
	int x = 8 + (cX - 1) * 13; //converts from building coordinates to character coordinates
	int y = 3 + (cY - 1) * 13;
	int j = x + counter;
	if (counter > 0)
	{
		move_cursor(XX + j - 1, YY - y);
		DISP(NBSP);
	}
	if (counter < 6)
	{
		move_cursor(XX + j, YY - y);
		DISP(CAR);
	}
	return;
}

void moveXleft(int cX, int cY, int counter) //does not change values, only displays movement on screen
{
	int x = (cX - 1) * 13; //converts from building coordinates to character coordinates
	int y = 5 + (cY - 1) * 13;
	int j = x - counter;
	if (counter > 0)
	{
		move_cursor(XX + j + 1, YY - y);
		DISP(NBSP);
	}
	if (counter < 6)
	{
		move_cursor(XX + j, YY - y);
		DISP(CAR);
	}
	return;
}

void moveYup(int cX, int cY, int counter) //does not change values, only displays movement on screen
{
	int x = 5 + (cX - 1) * 13; //converts from building coordinates to character coordinates
	int y = 8 + (cY - 1) * 13;
	int j = y + counter;
	if (counter > 0)
	{
		move_cursor(XX + x, YY - j + 1);
		DISP(NBSP);
	}
	if (counter < 6)
	{
		move_cursor(XX + x, YY - j);
		DISP(CAR);
	}
	return;
}

void moveYdown(int cX, int cY, int counter) //does not change values, only displays movement on screen
{
	int x = 3 + (cX - 1) * 13; //converts from building coordinates to character coordinates
	int y = (cY - 1) * 13;
	int j = y - counter;
	if (counter > 0)
	{
		move_cursor(XX + x, YY - j - 1);
		DISP(NBSP);
	}
	if (counter < 6)
	{
		move_cursor(XX + x, YY - j);
		DISP(CAR);
	}
	return;
}

void move_cursor(int col, int row)
{
	/* Move cursor to specified row and column */
	COORD coord;
	coord.X = col;
	coord.Y = row;
	/* Windows function to position cursor */
	SetConsoleCursorPosition(Console, coord);
}

int checkfreeparking(struct building* buildingmap, int xcoord, int ycoord)
{
	return buildingmap[(xcoord - 1) + (ycoord - 1) * (buildingmap[0].xmax)].parkedmax - buildingmap[(xcoord - 1) + (ycoord - 1) * (buildingmap[0].xmax)].parked;
}

int checkfreecharging(struct building* buildingmap, int xcoord, int ycoord)
{
	return buildingmap[(xcoord - 1) + (ycoord - 1) * (buildingmap[0].xmax)].chargingmax - buildingmap[(xcoord - 1) + (ycoord - 1) * (buildingmap[0].xmax)].charging;
}

void parktaxi(struct building* buildingmap, int xcoord, int ycoord)
{
	buildingmap[(xcoord - 1) + (ycoord - 1) * (buildingmap[0].xmax)].parked++;
	return;
}

void unparktaxi(struct building* buildingmap, int xcoord, int ycoord)
{
	buildingmap[(xcoord - 1) + (ycoord - 1) * (buildingmap[0].xmax)].parked--;
	return;
}

void chargetaxi(struct building* buildingmap, int xcoord, int ycoord)
{
	buildingmap[(xcoord - 1) + (ycoord - 1) * (buildingmap[0].xmax)].charging++;
	return;
}

void unchargetaxi(struct building* buildingmap, int xcoord, int ycoord)
{
	buildingmap[(xcoord - 1) + (ycoord - 1) * (buildingmap[0].xmax)].charging--;
	return;
}

bool getxy(int* mapxsize, int* mapysize)
{
	printf("Enter x dimension of map: ");
	scanf_s("%d", &*mapxsize);
	printf("Enter y dimension of map: ");
	scanf_s("%d", &*mapysize);
	if (mapxsize > 1 && mapysize > 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

struct building* initmap(int mapxsize, int mapysize)
{
	struct building* buildingmap = malloc(sizeof(struct building) * (mapxsize * mapysize));
	/*building index = (x-1)+(y-1)x(mapxsize)*/
	for (int y = 1; y <= mapysize; y++)
	{
		for (int x = 1; x <= mapxsize; x++)
		{
			buildingmap[(x - 1) + (y - 1) * (mapxsize)].x = x;
			buildingmap[(x - 1) + (y - 1) * (mapxsize)].y = y;
			buildingmap[(x - 1) + (y - 1) * (mapxsize)].parked = 0;
			buildingmap[(x - 1) + (y - 1) * (mapxsize)].charging = 0;
			buildingmap[(x - 1) + (y - 1) * (mapxsize)].parkedmax = 3;
			buildingmap[(x - 1) + (y - 1) * (mapxsize)].chargingmax = 3;
			buildingmap[(x - 1) + (y - 1) * (mapxsize)].xmax = mapxsize;
			buildingmap[(x - 1) + (y - 1) * (mapxsize)].ymax = mapysize;
		}
	}
	return buildingmap;
}

void printmapcoords(struct building* buildingmap, int mapxsize, int mapysize)
{
	/*building index = (x-1)+(y-1)x(mapxsize)*/
	for (int y = mapysize; y > 0; y--)
	{
		for (int x = 1; x <= mapxsize; x++)
		{
			printf("(%d,%d) ", buildingmap[(x - 1) + (y - 1) * (mapxsize)].x, buildingmap[(x - 1) + (y - 1) * (mapxsize)].y);
		}
		printf("\n");
	}
}

void printmap(struct building* buildingmap, int mapxsize, int mapysize)
{
	/*calls printbuilding at every x,y coordinate that is required*/
	/*type encodes wether it is a:	+-----------------------------------------------+
									|	topleft--------top-------------topright		|
									|	  |				 |				 |			|
									|	left-----------middle----------right		|
									|	  |				 |				 |			|
									|	bottomleft-----bottom----------bottomright	|	building.
									+-----------------------------------------------+

	notice that the roads between the different types of roads will be drawn according to this diagram*/

	/*printbuilding will use this information to call printroad appropriately*/

	int type;
	int charging;
	int parked;

	for (int y = mapysize; y > 0; y--)
	{
		for (int x = mapxsize; x > 0; x--)
		{
			/*(1,1) = bottomleft*/
			if (buildingmap[(x - 1) + (y - 1) * (mapxsize)].x == 1 && buildingmap[(x - 1) + (y - 1) * (mapxsize)].y == 1)
			{
				type = BOTTOMLEFT;
				charging = buildingmap[(x - 1) + (y - 1) * (mapxsize)].charging;
				parked = buildingmap[(x - 1) + (y - 1) * (mapxsize)].parked;

				printbuilding(x, y, type, charging, parked);
				//move_cursor(x + XXX, y + YYY);
				//DISP('1');
			}
			/*(mapxsize, 1) = bottomright*/
			else if (buildingmap[(x - 1) + (y - 1) * (mapxsize)].x == mapxsize && buildingmap[(x - 1) + (y - 1) * (mapxsize)].y == 1)
			{
				type = BOTTOMRIGHT;
				charging = buildingmap[(x - 1) + (y - 1) * (mapxsize)].charging;
				parked = buildingmap[(x - 1) + (y - 1) * (mapxsize)].parked;

				printbuilding(x, y, type, charging, parked);
				//move_cursor(x + XXX, y + YYY);
				//DISP('2');
			}
			/*(1, mapysize) = topleft*/
			else if (buildingmap[(x - 1) + (y - 1) * (mapxsize)].x == 1 && buildingmap[(x - 1) + (y - 1) * (mapxsize)].y == mapysize)
			{
				type = TOPLEFT;
				charging = buildingmap[(x - 1) + (y - 1) * (mapxsize)].charging;
				parked = buildingmap[(x - 1) + (y - 1) * (mapxsize)].parked;

				printbuilding(x, y, type, charging, parked);
				//move_cursor(x + XXX, y + YYY);
				//DISP('3');
			}
			/*(mapxsize, mapysize) = topright*/
			else if (buildingmap[(x - 1) + (y - 1) * (mapxsize)].x == mapxsize && buildingmap[(x - 1) + (y - 1) * (mapxsize)].y == mapysize)
			{
				type = TOPRIGHT;
				charging = buildingmap[(x - 1) + (y - 1) * (mapxsize)].charging;
				parked = buildingmap[(x - 1) + (y - 1) * (mapxsize)].parked;

				printbuilding(x, y, type, charging, parked);
				//move_cursor(x + XXX, y + YYY);
				//DISP('4');
			}
			/*(1, ... ) = left*/
			else if (buildingmap[(x - 1) + (y - 1) * (mapxsize)].x == 1)
			{
				type = LEFT;
				charging = buildingmap[(x - 1) + (y - 1) * (mapxsize)].charging;
				parked = buildingmap[(x - 1) + (y - 1) * (mapxsize)].parked;

				printbuilding(x, y, type, charging, parked);
				//move_cursor(x + XXX, y + YYY);
				//DISP('5');
			}
			/*(mapxsize, ... ) = right*/
			else if (buildingmap[(x - 1) + (y - 1) * (mapxsize)].x == mapxsize)
			{
				type = RIGHT;
				charging = buildingmap[(x - 1) + (y - 1) * (mapxsize)].charging;
				parked = buildingmap[(x - 1) + (y - 1) * (mapxsize)].parked;

				printbuilding(x, y, type, charging, parked);
				//move_cursor(x + XXX, y + YYY);
				//DISP('6');
			}
			/*( ... ,1) = bottom*/
			else if (buildingmap[(x - 1) + (y - 1) * (mapxsize)].y == 1)
			{
				type = BOTTOM;
				charging = buildingmap[(x - 1) + (y - 1) * (mapxsize)].charging;
				parked = buildingmap[(x - 1) + (y - 1) * (mapxsize)].parked;

				printbuilding(x, y, type, charging, parked);
				//move_cursor(x + XXX, y + YYY);
				//DISP('7');
			}
			/*( ... ,mapysize) = top*/
			else if (buildingmap[(x - 1) + (y - 1) * (mapxsize)].y == mapysize)
			{
				type = TOP;
				charging = buildingmap[(x - 1) + (y - 1) * (mapxsize)].charging;
				parked = buildingmap[(x - 1) + (y - 1) * (mapxsize)].parked;

				printbuilding(x, y, type, charging, parked);
				//move_cursor(x + XXX, y + YYY);
				//DISP('8');
			}
			/*if none of previous conditions met ( ... , ... ) = middle*/
			else
			{
				type = MIDDLE;
				charging = buildingmap[(x - 1) + (y - 1) * (mapxsize)].charging;
				parked = buildingmap[(x - 1) + (y - 1) * (mapxsize)].parked;

				printbuilding(x, y, type, charging, parked);
				//move_cursor(x + XXX, y + YYY);
				//DISP('9');
			}
		}
	}
}

void printbuilding(int x, int y, int type, int charging, int parked)
{
	/*allocate parking and charging spaces to the array*/
	char* s[] = { S, S, S, S, S, S }; /*array of the spaces in a building*/
	int parktemp = parked;
	int chargetemp = charging;

	for (int i = 0; i < charging + parked; i++)
	{
		if (parktemp > 0)
		{
			s[i] = 'P';
			parktemp--;
		}
		else if (chargetemp > 0)
		{
			s[i] = 'C';
			chargetemp--;
		}
	}


	/*print base building at given coordinates*/
	char* building[] = { UL,	iT,		T,	iT,		T,	iT,		UR,
							VL,	s[0],	V,	s[1],	V,	s[2],	VR,
							V,	S,		S,	S,		S,	S,		V,
							VL,	S,		S,	S,		S,	S,		VR,
							V,	S,		S,	S,		S,	S,		V,
							VL,	s[3],	V,	s[4],	V,	s[5],	VR,
							LL,	T,		iT,	T,		iT,	T,		LR, };

	for (int iy = 0; iy < 7; iy++)
	{
		for (int ix = 0; ix < 7; ix++)
		{
			move_cursor(XXX + (x - 1) * 13 + ix, YYY - (y - 1) * 13 - iy);
			DISP(building[(ix)+(6 - iy) * 7]);
		}
	}



	//move_cursor(XXX + (x - 1) * 13, YYY + (y - 1) * 13);
	//printf("%d%d", x, y);

	/*add roads based on type*/

	if (type == TOPLEFT)
	{
		printroad(x, y, SOUTH);
		printroad(x, y, EAST);
		move_cursor(XXX + (x - 1) * 13 + 2, YYY - (y - 1) * 13 - 3);
		printf("%d%dTL", x, y);
	}
	else if (type == TOP)
	{
		printroad(x, y, SOUTH);
		printroad(x, y, EAST);
		printroad(x, y, WEST);
		move_cursor(XXX + (x - 1) * 13 + 2, YYY - (y - 1) * 13 - 3);
		printf("%d%dT", x, y);
	}
	else if (type == TOPRIGHT)
	{
		printroad(x, y, SOUTH);
		printroad(x, y, WEST);
		move_cursor(XXX + (x - 1) * 13 + 2, YYY - (y - 1) * 13 - 3);
		printf("%d%dTR", x, y);
	}
	else if (type == LEFT)
	{
		printroad(x, y, SOUTH);
		printroad(x, y, EAST);
		printroad(x, y, NORTH);
		move_cursor(XXX + (x - 1) * 13 + 2, YYY - (y - 1) * 13 - 3);
		printf("%d%dL", x, y);
	}
	else if (type == MIDDLE)
	{
		printroad(x, y, SOUTH);
		printroad(x, y, WEST);
		printroad(x, y, NORTH);
		printroad(x, y, EAST);
		move_cursor(XXX + (x - 1) * 13 + 2, YYY - (y - 1) * 13 - 3);
		printf("%d%dM", x, y);
	}
	else if (type == RIGHT)
	{
		printroad(x, y, SOUTH);
		printroad(x, y, NORTH);
		printroad(x, y, WEST);
		move_cursor(XXX + (x - 1) * 13 + 2, YYY - (y - 1) * 13 - 3);
		printf("%d%dR", x, y);
	}
	else if (type == BOTTOMLEFT)
	{
		printroad(x, y, NORTH);
		printroad(x, y, EAST);
		move_cursor(XXX + (x - 1) * 13 + 2, YYY - (y - 1) * 13 - 3);
		printf("%d%dBL", x, y);
	}
	else if (type == BOTTOM)
	{
		printroad(x, y, WEST);
		printroad(x, y, NORTH);
		printroad(x, y, EAST);
		move_cursor(XXX + (x - 1) * 13 + 2, YYY - (y - 1) * 13 - 3);
		printf("%d%dB", x, y);
	}
	else if (type == BOTTOMRIGHT)
	{
		printroad(x, y, WEST);
		printroad(x, y, NORTH);
		move_cursor(XXX + (x - 1) * 13 + 2, YYY - (y - 1) * 13 - 3);
		printf("%d%dBR", x, y);
	}
}

void printroad(int xr, int yr, int direction)
{
	/*base coordinates of building center*/
	int xcoord = XXX + (xr - 1) * 13 + 3;
	int ycoord = YYY - (yr - 1) * 13 - 3;
	/*need to do additional math to print roads at correct location relative to building*/
	if (direction == NORTH)
	{
		for (int i = 0; i < 3; i++)
		{
			move_cursor(xcoord + 2, ycoord - 4 - i);
			DISP(V);
			move_cursor(xcoord - 2, ycoord - 4 - i);
			DISP(V);
			move_cursor(xcoord, ycoord - 4 - i);
			DISP(V);
		}
	}
	else if (direction == SOUTH)
	{
		for (int i = 0; i < 3; i++)
		{
			move_cursor(xcoord + 2, ycoord + 4 + i);
			DISP(V);
			move_cursor(xcoord, ycoord + 4 + i);
			DISP(V);
			move_cursor(xcoord - 2, ycoord + 4 + i);
			DISP(V);
		}
	}
	else if (direction == EAST)
	{
		for (int i = 0; i < 3; i++)
		{
			move_cursor(xcoord + 4 + i, ycoord + 2);
			DISP(H);
			move_cursor(xcoord + 4 + i, ycoord - 2);
			DISP(H);
			move_cursor(xcoord + 4 + i, ycoord);
			DISP(H);
		}
	}
	else if (direction == WEST)
	{
		for (int i = 0; i < 3; i++)
		{
			move_cursor(xcoord - 4 - i, ycoord + 2);
			DISP(H);
			move_cursor(xcoord - 4 - i, ycoord - 2);
			DISP(H);
			move_cursor(xcoord - 4 - i, ycoord);
			DISP(H);
		}
	}
}