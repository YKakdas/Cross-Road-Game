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
void drawSidewalks();
void drawLanes();
void drawCars();
void drawTrucks();
void drawAgent();
void drawCoins();
void drawScoreBoard();

void drawScoreBoard() {
	glColor3ub(125, 125, 125);
	glRecti(0, gameWindowHeight, width, height);
}

void drawSidewalks() {
	for (int i = 0; i < sideWalks.size(); i++) {
		SideWalk sideWalk = sideWalks[i];
		glColor3ub(sideWalk.color.r, sideWalk.color.g, sideWalk.color.b);
		glRecti(sideWalk.start.x, sideWalk.start.y, sideWalk.end.x, sideWalk.end.y);
	}
}

void drawLanes() {
	for (int i = 0; i < lanes.size(); i++) {
		Lane lane = lanes[i];
		glColor3ub(lane.color.r, lane.color.g, lane.color.b);
		glBegin(GL_LINES);
		glVertex2d(lane.start.x, lane.start.y);
		glVertex2d(lane.end.x, lane.end.y);
		glEnd();
	}
}

void drawCars() {
	for (int i = 0; i < carVector.size(); i++) {
		glColor3ub(carVector[i].color.r, carVector[i].color.g, carVector[i].color.b);
		glRecti(carVector[i].start.x, carVector[i].start.y, carVector[i].end.x, carVector[i].end.y);
	}
}

void drawTrucks() {
	for (int i = 0; i < truckVector.size(); i++) {
		glColor3ub(truckVector[i].color.r, truckVector[i].color.g, truckVector[i].color.b);
		glRecti(truckVector[i].start.x, truckVector[i].start.y, truckVector[i].end.x, truckVector[i].end.y);
	}
}

void drawAgent() {
	glBegin(GL_POLYGON);
	glColor3ub(agent.color.r, agent.color.g, agent.color.b);
	glVertex2d(agent.leftVertex.x, agent.leftVertex.y);
	glVertex2d(agent.rightVertex.x, agent.rightVertex.y);
	glVertex2d(agent.upVertex.x, agent.upVertex.y);
	glEnd();
}

void drawCoins() {
	for (int i = 0; i < coinVector.size(); i++) {
		glBegin(GL_POLYGON);
		for (int j = 0; j < 30; j++)
		{
			glColor3ub(coinVector[i].color.r, coinVector[i].color.g, coinVector[i].color.b);
			float angle = 2 * PI * j / 30;
			glVertex2f(coinVector[i].center.x + cos(angle) * coinVector[i].radius, coinVector[i].center.y + sin(angle) * coinVector[i].radius);
		}
		glEnd();
	}
}

void myDisplay(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	drawScoreBoard();
	drawSidewalks();
	drawLanes();
	drawCars();
	drawTrucks();
	drawAgent();
	drawCoins();

	glFlush();
}

int main(int argc, char** argv) {
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