#include <vector>
#include <stdlib.h>
#include <Windows.h>
#include <glut.h>
#include <string>
#include <iostream>
#include <math.h>

using namespace std;
using std::string;

/* struct definitions for holding drawing data */

typedef struct { // to hold x and y coordinates of agent,car,truck and coin
	GLint x;
	GLint y;
} Point2D;

typedef struct { // to hold color of drawing components
	GLint r;
	GLint g;
	GLint b;
} Color;

typedef struct { // to define cars
	Point2D start;
	Point2D end;
	Color color;
	int velocity;
	int lineNumber;
} Car;

typedef struct { // to define trucks
	Point2D start;
	Point2D end;
	Color color;
	int velocity;
	int lineNumber;
} Truck;


typedef struct { // to define lanes
	int lineNumber;
	char direction;
	Point2D start;
	Point2D end;
	Color color;
} Lane;

typedef struct { // to define sidewalks
	Point2D start;
	Point2D end;
	Color color;
} SideWalk;

typedef struct { // to define agent
	Point2D leftVertex;
	Point2D rightVertex;
	Point2D upVertex;
	char direction; // U for UP and D for DOWN
	Color color;
} Agent;

typedef struct { // to define coins
	GLint radius;
	Point2D center;
	Color color;
} Coin;

typedef struct {  // for collision detection
	GLint x;
	GLint y;
	GLint width;
	GLint height;
} Rect;

/* globals */

GLsizei width = 520, height = 700; /* initial window size */

const double PI = 3.141592653589793238463;

int score = 0;
int time = 0;

/* variables for holding game states s*/
GLboolean isPaused = false;
GLboolean isGameOver = false;
GLboolean isGameWon = false;
GLboolean isOneStepMode = false;


/* periods for timer functions */

GLint randomVehicleGeneratorPeriod = 100;
GLint updateVehiclePeriod = 20;
GLint randomCoinGeneratorPeriod = 1000;

/* constants for drawing components' size*/

const GLint SIDEWALK_WIDTH = 40;
const GLint ROAD_WIDTH = 80;
const GLint LANE_LENGTH = 20;

const GLint GAP_BETWEEN_LANES_HORIZONTALLY = 15;
const GLint GAP_BETWEEN_LANES_VERTICALLY = 20;

const GLint SCOREBOARD_SIZE = 64;
const GLint gameWindowHeight = height - SCOREBOARD_SIZE;

const GLint AGENT_WIDTH = 10;
const GLint AGENT_HEIGHT = 20;

const GLint CAR_HALF_SIZE = (GAP_BETWEEN_LANES_VERTICALLY - GAP_BETWEEN_LANES_VERTICALLY / 5) / 2;
const GLint TRUCK_HALF_SIZE = (GAP_BETWEEN_LANES_VERTICALLY - GAP_BETWEEN_LANES_VERTICALLY / 5) / 2;

const GLint NUMBER_OF_SIDEWALKS = 6;
const GLint NUMBER_OF_ROADS = 5;
const GLint NUMBER_OF_LINES_PER_ROAD = 4;
const GLint TOTAL_NUMBER_OF_LINES = NUMBER_OF_ROADS * NUMBER_OF_LINES_PER_ROAD;
const GLint NUMBER_OF_LANES_PER_LINE = 15;

const GLint GAP_FROM_WINDOW = 5;

/* vector definitions for keeping multiple objects of same type */

vector<SideWalk> sideWalks; // holds sidewalks
vector<Lane> lanes; // holds lanes
vector<Car> carVector; // holds generated cars
vector<Truck> truckVector; // holds generated trucks
vector<Coin> coinVector; // holds generated coins
vector<int> keyboardHistory; // holds keyboard history for taking last key input when program is in the one step mode(right click)

Agent agent;

/* function prototypes */

void fillSideWalksVector();
void fillLanesVector();
vector<Car> getCarsFromGivenLineNumber(int lineNumber);
vector<Truck> getTrucksFromGivenLineNumber(int lineNumber);
vector<Lane> getLanesFromGivenLineNumber(int lineNumber);
int getHeightOfGivenLineNumber(int lineNumber);
char getDirectionOfLine(int lineNumber);
GLboolean isThereAnyCarOrTruckInThatLineEdges(GLint y, GLint lineNumber);
void randomVehicleGenerator(int id);
void randomCoinGenerator(int id);
void updateVehicleLocation(int id);
void agentInit();
void gameOver();
void gameRestart();
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
void drawSidewalks();
void drawLanes();
void drawCars();
void drawTrucks();
void drawAgent();
void drawCoins();
void drawScoreBoard();
void checkCollisions();
int getLineNumberOfAgent();
void drawGameOver();
void timeCounter(int id);
void drawWonTheGame();
void gameInfo();