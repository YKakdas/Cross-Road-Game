#include <time.h>
#include <stdlib.h>
#include <Windows.h>
#include <glut.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <math.h>

using namespace std;

/* globals */

GLsizei width = 520, height = 700; /* initial window size */

const double PI = 3.141592653589793238463;

int score = 0;

GLboolean isPaused = false;
GLboolean isGameOver = false;

GLint randomVehicleGeneratorPeriod = 500;
GLint updateVehiclePeriod = 500;
GLint randomCoinGeneratorPeriod = 2000;

/* these 6 variables are calculated from the initial window size ( 520 x 700 ) Their values are dependent to window size. So, with using this initial values,
   in the resize callback function, some dividers are calculated. Using these dividers with new window size gives us these variables' new values to make
   nice-looking user interface.
*/

GLint SIDEWALK_WIDTH = 40;
GLint ROAD_WIDTH = 80;
GLint LANE_LENGTH = 20;

GLint GAP_BETWEEN_LANES_HORIZONTALLY = 15;
GLint GAP_BETWEEN_LANES_VERTICALLY = 20;

GLint SCOREBOARD_SIZE = ceil((GLdouble)height / (GLdouble)11);
GLint gameWindowHeight = height - SCOREBOARD_SIZE;

/* constants for drawing components' size*/

const GLint CAR_HALF_SIZE = (GAP_BETWEEN_LANES_VERTICALLY - 4) / 2;
const GLint TRUCK_HALF_SIZE = (GAP_BETWEEN_LANES_VERTICALLY - 4) / 2;

const GLint NUMBER_OF_SIDEWALKS = 6;
const GLint NUMBER_OF_ROADS = 5;
const GLint NUMBER_OF_LINES_PER_ROAD = 4;
const GLint TOTAL_NUMBER_OF_LINES = NUMBER_OF_ROADS * NUMBER_OF_LINES_PER_ROAD;
const GLint NUMBER_OF_LANES_PER_LINE = 15;

const GLint GAP_FROM_WINDOW = 5;

/* struct definitions for holding drawing data */

typedef struct {
	GLint x;
	GLint y;
} Point2D;

typedef struct {
	GLint r;
	GLint g;
	GLint b;
} Color;

typedef struct {
	Point2D start;
	Point2D end;
	Color color;
	int velocity;
	int lineNumber;
} Car;

typedef struct {
	Point2D start;
	Point2D end;
	Color color;
	int velocity;
	int lineNumber;
} Truck;


typedef struct {
	int lineNumber;
	char direction;
	Point2D start;
	Point2D end;
	Color color;
} Lane;

typedef struct {
	Point2D start;
	Point2D end;
	Color color;
} SideWalk;

typedef struct {
	Point2D leftVertex;
	Point2D rightVertex;
	Point2D upVertex;
	char direction; // U for UP and D for DOWN
	Color color;
} Agent;

typedef struct {
	GLint radius;
	Point2D center;
	Color color;
} Coin;

vector<SideWalk> sideWalks;
vector<Lane> lanes;
vector<Car> carVector;
vector<Truck> truckVector;
vector<Coin> coinVector;
Agent agent;

/* function prototypes */

void fillSideWalksVector();
void fillLanesVector();
vector<Car> getCarsFromGivenLineNumber(int lineNumber);
vector<Truck> getTrucksFromGivenLineNumber(int lineNumber);
vector<Lane> getLanesFromGivenLineNumber(int lineNumber);
int getHeightOfGivenLineNumber(int lineNumber);
char getDirectionOfLine(int lineNumber);
GLboolean isThereAnyCarOrTruckInThatLine(GLint y, GLint lineNumber);
void randomVehicleGenerator(int id);
void randomCoinGenerator(int id);
void updateVehicleLocation(int id);
void agentInit();
void gameOver();
void turnAgentDown();
void turnAgentUp();
void agentMoveUp();
void agentMoveDown();
void agentMoveLeft();
void agentMoveRight();
void myReshape(GLsizei w, GLsizei h);
void myinit(void);
void myKeyboard(unsigned char key, int x, int y);
void myKeyboardSpecial(int key, int x, int y);
void myMouse(int btn, int state, int x, int y);
void myDisplay(void);

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Cross Road");
	myinit();
	glutReshapeFunc(myReshape);
	glutMouseFunc(myMouse);
	glutDisplayFunc(myDisplay);
	glutKeyboardFunc(myKeyboard);
	glutSpecialFunc(myKeyboardSpecial);
	glutTimerFunc(randomVehicleGeneratorPeriod, randomVehicleGenerator, 0);
	glutTimerFunc(updateVehiclePeriod, updateVehicleLocation, 0);
	glutTimerFunc(randomCoinGeneratorPeriod, randomCoinGenerator, 0);

	srand(time(NULL));

	glutMainLoop();
}