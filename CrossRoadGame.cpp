#include <stdlib.h>
#include <Windows.h>
#include <glut.h>
#include <string>
#include <vector>
#include <iostream>
#include <math.h>

//TODO implement winning phase(if reaches some point or reaches 1 hour)
//TODO implement statistics of program for debugging purposes
using namespace std;
using std::string;

/* globals */

GLsizei width = 520, height = 700; /* initial window size */

const double PI = 3.141592653589793238463;

int score = 0;
int time = 0;

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

GLint AGENT_WIDTH = 10;
GLint AGENT_HEIGHT = 20;

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

typedef struct {  // for collision detection
	GLint x;
	GLint y;
	GLint width;
	GLint height;
} Rect;

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
void checkCollisions();
int getLineNumberOfAgent();
void drawGameOver();
void timeCounter(int id);
void drawWonTheGame();


void timeCounter(int id) {
	if (isPaused) {
		return;
	}
	if (!isPaused || !isGameOver) {
		time++;
		glutTimerFunc(1000, timeCounter, 0);
	}
	if (time == 3599) {
		isGameOver = true;
		isPaused = true;
		glutPostRedisplay();
	}
}

vector<Car> getCarsFromGivenLineNumber(int lineNumber) {
	vector<Car> carsInGivenLine;
	for (int i = 0; i < carVector.size(); i++) {
		if (carVector[i].lineNumber == lineNumber) {
			carsInGivenLine.push_back(carVector[i]);
		}
	}
	return carsInGivenLine;
}

vector<Truck> getTrucksFromGivenLineNumber(int lineNumber) {
	vector<Truck> trucksInGivenLine;
	for (int i = 0; i < truckVector.size(); i++) {
		if (truckVector[i].lineNumber == lineNumber) {
			trucksInGivenLine.push_back(truckVector[i]);
		}
	}
	return trucksInGivenLine;
}

vector<Lane> getLanesFromGivenLineNumber(int lineNumber) {
	vector<Lane> lanesInGivenLine;
	for (int i = 0; i < lanes.size(); i++) {
		if (lanes[i].lineNumber == lineNumber) {
			lanesInGivenLine.push_back(lanes[i]);
		}
	}
	return lanesInGivenLine;
}

int getHeightOfGivenLineNumber(int lineNumber) {
	for (int i = 0; i < lanes.size(); i++) {
		if (lanes[i].lineNumber == lineNumber) {
			return lanes[i].start.y;
		}
	}
	return -1;
}

char getDirectionOfLine(int lineNumber) {
	for (int i = 0; i < lanes.size(); i++) {
		if (lanes[i].lineNumber == lineNumber) {
			return lanes[i].direction;
		}
	}
	return 'E'; // indicates error
}

GLboolean isThereAnyCarOrTruckInThatLine(GLint y, GLint lineNumber) {
	vector<Car> cars = getCarsFromGivenLineNumber(lineNumber);
	vector<Truck> trucks = getTrucksFromGivenLineNumber(lineNumber);

	for (int i = 0; i < cars.size(); i++) {
		if (cars[i].start.y == y) {
			if (cars[i].start.x < CAR_HALF_SIZE * 4 || cars[i].end.x > width - CAR_HALF_SIZE * 2) {
				return true;
			}
		}
	}

	for (int i = 0; i < trucks.size(); i++) {
		if (trucks[i].start.y == y) {
			if (trucks[i].start.x < TRUCK_HALF_SIZE * 6 || trucks[i].end.x > width - CAR_HALF_SIZE * 4) {
				return true;
			}
		}
	}
	return false;
}

void fillSideWalksVector() {
	for (GLint i = 0; i < NUMBER_OF_SIDEWALKS; i++) {

		/* setting side walk color to black */
		Color color = { 0,0,0 };

		/* define the starting point of side walk */
		Point2D start;
		start.x = 0;
		start.y = (i * (SIDEWALK_WIDTH + ROAD_WIDTH));

		/* define the ending point of side walk */
		Point2D end;
		end.x = width;
		end.y = (i * (SIDEWALK_WIDTH + ROAD_WIDTH)) + SIDEWALK_WIDTH;

		/* putting values into side walk struct and adding them to sidewalk vector which holds all sidewalks */
		SideWalk sideWalk;
		sideWalk.color = color;
		sideWalk.start = start;
		sideWalk.end = end;

		sideWalks.push_back(sideWalk);
	}
}

void fillLanesVector() {
	GLint lineNumber = 0;
	for (GLint i = 0; i < NUMBER_OF_ROADS; i++) { // traverse each road
		for (GLint j = 0; j < NUMBER_OF_LINES_PER_ROAD; j++) {  // traverse each line for each road
			for (GLint k = 0; k < NUMBER_OF_LANES_PER_LINE; k++) {  // inside the lines of roads, define the lanes for them

				/* setting lane color to black */
				Color color = { 0,0,0 };

				/* define the starting point of lane line */
				Point2D start;
				start.x = k * (LANE_LENGTH + GAP_BETWEEN_LANES_HORIZONTALLY) + GAP_FROM_WINDOW;
				start.y = (ROAD_WIDTH * i) + (SIDEWALK_WIDTH * (i + 1)) + (GAP_BETWEEN_LANES_VERTICALLY * (j));

				/* define the ending point of lane line */
				Point2D end;
				end.x = start.x + LANE_LENGTH;
				end.y = (ROAD_WIDTH * i) + (SIDEWALK_WIDTH * (i + 1)) + (GAP_BETWEEN_LANES_VERTICALLY * (j));

				/* putting values into lane struct and adding them to lane vector which holds all lanes */
				Lane lane;
				lane.lineNumber = lineNumber;
				lane.color = color;
				lane.start = start;
				lane.end = end;

				/* for even lines, make vehicles go from left to right, for odds from right to left*/
				if (((i + j) % 2) == 0) {
					lane.direction = 'R';
				}
				else {
					lane.direction = 'L';
				}

				lanes.push_back(lane);
			}
			lineNumber++;
		}
	}
}

void agentInit() {
	SideWalk agentSideWalk = sideWalks[0];
	GLint agentLeftX = ((agentSideWalk.start.x) + (agentSideWalk.end.x)) / 2 - 5;
	GLint agentRightX = agentLeftX + 10;
	GLint agentUpX = agentLeftX + 5;
	GLint agentLeftAndRightY = agentSideWalk.start.y;
	GLint agentUpY = agentLeftAndRightY + GAP_BETWEEN_LANES_VERTICALLY;

	Point2D leftVertex = { agentLeftX ,agentLeftAndRightY };
	Point2D rightVertex = { agentRightX ,agentLeftAndRightY };
	Point2D upVertex = { agentUpX ,agentUpY };

	Color color = { 255,0,0 };

	agent.leftVertex = leftVertex;
	agent.rightVertex = rightVertex;
	agent.upVertex = upVertex;
	agent.direction = 'U';
	agent.color = color;
}
void randomVehicleGenerator(int id) {

	if (isPaused) {
		return;
	}
	int carType = rand() % 2;
	int direction = rand() % 2;
	int lineNumber = rand() % TOTAL_NUMBER_OF_LINES;
	int x;
	int y = getHeightOfGivenLineNumber(lineNumber) + 2; // y coordiante of bottom of vehicle i.e. corresponding line's y value

	int r = rand() % 200;
	int g = (rand() % 200) + 56;
	int b = (rand() % 200) + 56;

	Color color = { r, g, b };
	int velocity = 6;

	if (isThereAnyCarOrTruckInThatLine(y, lineNumber)) {
		glutTimerFunc(randomVehicleGeneratorPeriod, randomVehicleGenerator, 0);
		return;
	}

	if (getCarsFromGivenLineNumber(lineNumber).size() != 0 || getTrucksFromGivenLineNumber(lineNumber).size() != 0) {
		velocity = 3;
	}
	if (carType == 0) { // generate car

		Car car;
		if (direction == 0) {
			x = 0;
		}
		else {
			x = width - CAR_HALF_SIZE * 2;
		}

		Point2D start = { x,y };
		Point2D end = { x + CAR_HALF_SIZE * 2 ,y + CAR_HALF_SIZE * 2 };

		car.start = start;
		car.end = end;
		car.color = color;
		car.velocity = velocity;
		car.lineNumber = lineNumber;

		carVector.push_back(car);
	}
	else { // generate truck

		Truck truck;

		if (direction == 0) {
			x = 0;
		}
		else {
			x = width - CAR_HALF_SIZE * 2;
		}
		Point2D start = { x,y };
		Point2D end = { x + TRUCK_HALF_SIZE * 4 ,y + TRUCK_HALF_SIZE * 2 };

		truck.start = start;
		truck.end = end;
		truck.color = color;
		truck.velocity = velocity;
		truck.lineNumber = lineNumber;

		truckVector.push_back(truck);
	}
	glutTimerFunc(randomVehicleGeneratorPeriod, randomVehicleGenerator, 0);
	glutPostRedisplay();
}

void randomCoinGenerator(int id) {

	if (isPaused) {
		return;
	}

	if (coinVector.size() > 5) {
		coinVector.erase(coinVector.begin());
	}
	Coin coin;
	Color color = { 204,204,0 };
	Point2D center;
	coin.radius = 5;
	center.x = (rand() % (width - coin.radius * 4 + 1)) + coin.radius * 2;
	center.y = (rand() % (gameWindowHeight - SIDEWALK_WIDTH - coin.radius * 6 + 1)) + SIDEWALK_WIDTH + coin.radius * 2;

	coin.center = center;
	coin.color = color;
	coinVector.push_back(coin);
	glutTimerFunc(2000, randomCoinGenerator, 0);
}
void updateVehicleLocation(int id) {

	if (isPaused) {
		return;
	}
	for (int i = 0; i < carVector.size(); i++) {
		char direction = getDirectionOfLine(carVector[i].lineNumber);
		if (direction == 'L') {
			carVector[i].start.x = carVector[i].start.x - carVector[i].velocity;
			carVector[i].end.x = carVector[i].end.x - carVector[i].velocity;
			if (carVector[i].start.x < -10) {
				carVector.erase(carVector.begin() + i);
			}
		}
		else if (direction == 'R') {
			carVector[i].start.x = carVector[i].start.x + carVector[i].velocity;
			carVector[i].end.x = carVector[i].end.x + carVector[i].velocity;
			if (carVector[i].start.x > width + 10) {
				carVector.erase(carVector.begin() + i);
			}
		}
	}

	for (int i = 0; i < truckVector.size(); i++) {
		char direction = getDirectionOfLine(truckVector[i].lineNumber);
		if (direction == 'L') {
			truckVector[i].start.x = truckVector[i].start.x - truckVector[i].velocity;
			truckVector[i].end.x = truckVector[i].end.x - truckVector[i].velocity;
			if (truckVector[i].start.x < -10) {
				truckVector.erase(truckVector.begin() + i);
			}
		}
		else if (direction == 'R') {
			truckVector[i].start.x = truckVector[i].start.x + truckVector[i].velocity;
			truckVector[i].end.x = truckVector[i].end.x + truckVector[i].velocity;
			if (truckVector[i].start.x > width + 10) {
				truckVector.erase(truckVector.begin() + i);
			}
		}
	}
	checkCollisions();
	glutTimerFunc(updateVehiclePeriod, updateVehicleLocation, 0);
	glutPostRedisplay();
}

int getLineNumberOfAgent() {
	int lineNumber = -1; // indicates that agent is in SideWalk

	GLint coordinateY;

	if (agent.direction == 'U') {
		coordinateY = agent.leftVertex.y;
	}
	else {
		coordinateY = agent.upVertex.y;
	}

	for (int i = 0; i < lanes.size(); i = i + NUMBER_OF_LANES_PER_LINE) {
		if (lanes[i].start.y == coordinateY) {
			lineNumber = lanes[i].lineNumber;
			break;
		}
	}
	return lineNumber;
}
void checkCollisions() {
	GLint lineNumber = getLineNumberOfAgent();

	Rect agentRect;

	if (agent.direction == 'U') {
		agentRect = { agent.leftVertex.x,agent.leftVertex.y,AGENT_WIDTH,AGENT_HEIGHT };
	}
	else {
		agentRect = { agent.upVertex.x - AGENT_WIDTH / 2,agent.upVertex.y,AGENT_WIDTH,AGENT_HEIGHT };
	}

	for (int i = 0; i < coinVector.size(); i++) {
		Coin coin = coinVector[i];
		Rect coinRect = { coin.center.x - coin.radius,coin.center.y - coin.radius, coin.radius * 2,coin.radius * 2 };

		if (coinRect.x < agentRect.x + agentRect.width &&
			coinRect.x + coinRect.width > agentRect.x &&
			coinRect.y < agentRect.y + agentRect.height &&
			coinRect.y + coinRect.height > agentRect.y) {
			PlaySound(TEXT("coin"), NULL, SND_ASYNC);
			score += 5;
			coinVector.erase(coinVector.begin() + i);
			break;
		}
	}

	if (lineNumber != -1) {
		vector<Car> carsInThatLine = getCarsFromGivenLineNumber(lineNumber);
		vector<Truck> trucksInThatLine = getTrucksFromGivenLineNumber(lineNumber);

		for (int i = 0; i < carsInThatLine.size(); i++) {
			Car car = carsInThatLine[i];
			Rect carRect = { car.start.x,car.start.y,CAR_HALF_SIZE * 2,CAR_HALF_SIZE * 2 };

			if (carRect.x < agentRect.x + agentRect.width &&
				carRect.x + carRect.width > agentRect.x &&
				carRect.y < agentRect.y + agentRect.height &&
				carRect.y + carRect.height > agentRect.y) {
				PlaySound(TEXT("car-crash"), NULL, SND_ASYNC);
				gameOver();
				break;
			}
		}

		for (int i = 0; i < trucksInThatLine.size(); i++) {
			Truck truck = trucksInThatLine[i];
			Rect truckRect = { truck.start.x,truck.start.y,TRUCK_HALF_SIZE * 4,TRUCK_HALF_SIZE * 2 };

			if (truckRect.x < agentRect.x + agentRect.width &&
				truckRect.x + truckRect.width > agentRect.x &&
				truckRect.y < agentRect.y + agentRect.height &&
				truckRect.y + truckRect.height > agentRect.y) {
				PlaySound(TEXT("car-crash"), NULL, SND_ASYNC);
				gameOver();
				break;
			}
		}
	}
}
void turnAgentDown() {

	agent.leftVertex.y = agent.upVertex.y;
	agent.upVertex.y = agent.rightVertex.y;
	agent.rightVertex.y = agent.leftVertex.y;
	agent.direction = 'D';
	glutPostRedisplay();
}

void turnAgentUp() {

	agent.leftVertex.y = agent.upVertex.y;
	agent.upVertex.y = agent.rightVertex.y;
	agent.rightVertex.y = agent.leftVertex.y;
	agent.direction = 'U';
	glutPostRedisplay();
}
void agentMoveUp() {

	if (isPaused) {
		return;
	}
	if (agent.direction == 'D') {
		gameOver();
		return;
	}
	agent.leftVertex.y = agent.leftVertex.y + GAP_BETWEEN_LANES_VERTICALLY;
	agent.rightVertex.y = agent.leftVertex.y;
	agent.upVertex.y = agent.upVertex.y + GAP_BETWEEN_LANES_VERTICALLY;

	if (agent.upVertex.y >= sideWalks[sideWalks.size() - 1].start.y + GAP_BETWEEN_LANES_VERTICALLY) {
		turnAgentDown();
	}

	score++;
	checkCollisions();
	glutPostRedisplay();
}

void agentMoveDown() {

	if (isPaused) {
		return;
	}
	if (agent.direction == 'U') {
		gameOver();
		return;
	}
	agent.leftVertex.y = agent.leftVertex.y - GAP_BETWEEN_LANES_VERTICALLY;
	agent.rightVertex.y = agent.leftVertex.y;
	agent.upVertex.y = agent.upVertex.y - GAP_BETWEEN_LANES_VERTICALLY;

	if (agent.upVertex.y <= sideWalks[0].end.y - GAP_BETWEEN_LANES_VERTICALLY) {
		turnAgentUp();
	}
	score++;
	checkCollisions();
	glutPostRedisplay();
}
void agentMoveLeft() {
	if (isPaused) {
		return;
	}
	if (agent.leftVertex.x - 5 >= 0) {
		agent.leftVertex.x = agent.leftVertex.x - 5;
		agent.rightVertex.x = agent.rightVertex.x - 5;
		agent.upVertex.x = agent.upVertex.x - 5;
		checkCollisions();
		glutPostRedisplay();
	}
}

void agentMoveRight() {
	if (isPaused) {
		return;
	}
	if (agent.rightVertex.x + 5 <= width) {
		agent.leftVertex.x = agent.leftVertex.x + 5;
		agent.rightVertex.x = agent.rightVertex.x + 5;
		agent.upVertex.x = agent.upVertex.x + 5;
		checkCollisions();
		glutPostRedisplay();
	}
}

void gameOver() {
	isGameOver = true;
	isPaused = true;
	glutPostRedisplay();
	cout << "Game Over \n";
}

void myReshape(GLsizei w, GLsizei h) {

	/* adjust clipping box */

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* adjust viewport and clear */

	glViewport(0, 0, w, h);

	/* set global size for use by drawing routine */

	agent.leftVertex.x = agent.leftVertex.x * w / width;
	agent.rightVertex.x = agent.leftVertex.x + 10;
	agent.upVertex.x = agent.leftVertex.x + 5;
	agent.leftVertex.y = agent.leftVertex.y * h / height;
	agent.rightVertex.y = agent.leftVertex.y;
	agent.upVertex.y = agent.leftVertex.y + GAP_BETWEEN_LANES_VERTICALLY;
	width = w;
	height = h;

	/* these dividers are derivated from the initial values of window */

	SCOREBOARD_SIZE = ceil((GLdouble)height / (GLdouble)11);
	gameWindowHeight = height - SCOREBOARD_SIZE;

	SIDEWALK_WIDTH = ceil(((GLdouble)gameWindowHeight / (GLdouble)16));
	ROAD_WIDTH = ceil((GLdouble)gameWindowHeight / (GLdouble)8);
	LANE_LENGTH = ceil((GLdouble)width / (GLdouble)26);

	GAP_BETWEEN_LANES_HORIZONTALLY = ceil((GLdouble)width / (GLdouble)34);
	GAP_BETWEEN_LANES_VERTICALLY = ceil((GLdouble)gameWindowHeight / (GLdouble)32);

	sideWalks.clear();
	fillSideWalksVector();
	lanes.clear();
	fillLanesVector();

	/* Previous Cars and Trucks are cleared otherwise when screen size
	   changes, shape and coordinates of vehicles would change and this is not desired case
	   So, all components will be drawn again when resize is happened and vehicles will be created
	   from scratch */

	carVector.clear();
	truckVector.clear();
	
}

void myinit(void) {
	fillSideWalksVector();
	fillLanesVector();
	agentInit();

	/* setting view port that fills whole screen */
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble)width, 0.0, (GLdouble)height);

	/* set clear color to white and clear window */

	glClearColor(0.5, 0.5, 0.5, 1.0);
}

void myKeyboard(unsigned char key, int x, int y) {
	if ((key == 'Q') || (key == 'q'))
		exit(0);
}

void myKeyboardSpecial(int key, int x, int y) {
	if (key == GLUT_KEY_UP) {
		agentMoveUp();
	}
	else if (key == GLUT_KEY_LEFT) {
		agentMoveLeft();
	}
	else if (key == GLUT_KEY_RIGHT) {
		agentMoveRight();
	}
	else if (key == GLUT_KEY_DOWN) {
		agentMoveDown();
	}
}

void myMouse(int btn, int state, int x, int y) {
	if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if (!isPaused) {
			isPaused = !isPaused;
		}
		else {
			if (!isGameOver) {
				isPaused = false;
				updateVehicleLocation(0);
				randomVehicleGenerator(0);
				randomCoinGenerator(0);
				timeCounter(0);
				isPaused = true;
			}
		}
	}
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (isGameOver) {
			return;
		}
		if (isPaused) {
			glutTimerFunc(updateVehiclePeriod, updateVehicleLocation, 0);
			glutTimerFunc(randomVehicleGeneratorPeriod, randomVehicleGenerator, 0);
			glutTimerFunc(1000, timeCounter, 0);
		}
		isPaused = !isPaused;
	}
}

void drawScoreBoard() {
	glColor3ub(190, 190, 190);
	glRecti(0, gameWindowHeight, width, height);
	int minute = time / 60;
	string timeStr = "TIME : " +to_string(minute / 10) + to_string((minute % 10)) + ":" + to_string((time / 10) % 6) + to_string((time % 10));
	glColor3ub(51, 0, 102);
	glRasterPos2i(10, height - 25);
	for (int i = 0; i < timeStr.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, timeStr[i]);
	}

	string scoreStr = "SCORE : " + to_string(score);
	glRasterPos2i(10, height - 55);
	for (int i = 0; i < scoreStr.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, scoreStr[i]);
	}
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

void drawGameOver() {
	glColor3ub(0, 0, 0);
	GLint x = width / 10;
	GLint y = height / 3;
	GLint widthGameOver = 9 * width / 10;
	GLint heightGameOver = 2 * height / 3;
	glRecti(x, y, widthGameOver, heightGameOver);
	glColor3ub(255, 0, 0);
	string gameOver = "GAME OVER";
	string scoreStr = "YOUR SCORE IS : " + to_string(score);

	glRasterPos2i((x + widthGameOver) / 3, (y + heightGameOver) * 3 / 5);
	for (int i = 0; i < gameOver.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, gameOver[i]);
	}

	int minute = time / 60;
	string timeStr = "PLAYING TIME : " + to_string(minute / 10) + to_string((minute % 10)) + ":" + to_string((time / 10) % 6) + to_string((time % 10));
	glRasterPos2i((x + 10), (y + heightGameOver) * 6 / 11);
	for (int i = 0; i < timeStr.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, timeStr[i]);
	}

	glRasterPos2i((x + 10), (y + heightGameOver) * 1 / 2);
	for (int i = 0; i < scoreStr.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, scoreStr[i]);
	}

	string exit = "Press q for exit the game";
	glRasterPos2i((x + 10), (y + heightGameOver) * 2 / 5);
	for (int i = 0; i < exit.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, exit[i]);
	}

	glRasterPos2i((x + widthGameOver) * 2 / 3, (y + heightGameOver) * 3 / 5);
	glBegin(GL_LINE_LOOP);
	for (int j = 0; j < 30; j++)
	{
		float angle = 2 * PI * j / 30;
		glVertex2f((x + widthGameOver) * 2 / 3 + cos(angle) * 40, (y + heightGameOver) * 2 / 5 + sin(angle) * 40 + 10);
	}
	glEnd();

	glBegin(GL_LINES);
	glVertex2i((x + widthGameOver) * 2 / 3 - 26, (y + heightGameOver) * 42 / 100+10);
	glVertex2i((x + widthGameOver) * 2 / 3 - 6, (y + heightGameOver) * 42 / 100 + 10);
	glVertex2i((x + widthGameOver) * 2 / 3 + 4, (y + heightGameOver) * 42 / 100 + 10);
	glVertex2i((x + widthGameOver) * 2 / 3 + 24, (y + heightGameOver) * 42 / 100 + 10);
	glEnd();

	glBegin(GL_LINES);
	glVertex2i((x + widthGameOver) * 2 / 3 - 4, (y + heightGameOver) * 40 / 100 + 10);
	glVertex2i((x + widthGameOver) * 2 / 3 - 1, (y + heightGameOver) * 41 / 100 + 10);
	glVertex2i((x + widthGameOver) * 2 / 3 - 1, (y + heightGameOver) * 41 / 100 + 10);
	glVertex2i((x + widthGameOver) * 2 / 3 + 2, (y + heightGameOver) * 40 / 100 + 10);
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int j = 2; j < 14; j++)
	{
		float angle = 2 * PI * j / 30;
		glVertex2f((x + widthGameOver) * 2 / 3 + cos(angle) * 20, (y + heightGameOver) * 36 / 100 + sin(angle) * 20 + 10);
	}
	glEnd();
}

void drawWonTheGame() {
	glColor3ub(0, 0, 0);
	GLint x = width / 10;
	GLint y = height / 3;
	GLint widthGameOver = 9 * width / 10;
	GLint heightGameOver = 2 * height / 3;
	glRecti(x, y, widthGameOver, heightGameOver);
	glColor3ub(255, 0, 0);
	string gameOver = "YOU WON THE GAME";
	string scoreStr = "YOUR SCORE IS : " + to_string(score);

	glRasterPos2i((x + widthGameOver) / 3, (y + heightGameOver) * 3 / 5);
	for (int i = 0; i < gameOver.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, gameOver[i]);
	}

	int minute = time / 60;
	string timeStr = "PLAYING TIME : " + to_string(minute / 10) + to_string((minute % 10)) + ":" + to_string((time / 10) % 6) + to_string((time % 10));
	glRasterPos2i((x + 10), (y + heightGameOver) * 6 / 11);
	for (int i = 0; i < timeStr.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, timeStr[i]);
	}

	glRasterPos2i((x + 10), (y + heightGameOver) * 1 / 2);
	for (int i = 0; i < scoreStr.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, scoreStr[i]);
	}

	string exit = "Press q for exit the game";
	glRasterPos2i((x + 10), (y + heightGameOver) * 2 / 5);
	for (int i = 0; i < exit.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, exit[i]);
	}

	glRasterPos2i((x + widthGameOver) * 2 / 3, (y + heightGameOver) * 3 / 5);
	glBegin(GL_LINE_LOOP);
	for (int j = 0; j < 30; j++)
	{
		float angle = 2 * PI * j / 30;
		glVertex2f((x + widthGameOver) * 2 / 3 + cos(angle) * 40, (y + heightGameOver) * 2 / 5 + sin(angle) * 40 + 10);
	}
	glEnd();

	glBegin(GL_LINES);
	glVertex2i((x + widthGameOver) * 2 / 3 - 26, (y + heightGameOver) * 42 / 100 + 10);
	glVertex2i((x + widthGameOver) * 2 / 3 - 6, (y + heightGameOver) * 42 / 100 + 10);
	glVertex2i((x + widthGameOver) * 2 / 3 + 4, (y + heightGameOver) * 42 / 100 + 10);
	glVertex2i((x + widthGameOver) * 2 / 3 + 24, (y + heightGameOver) * 42 / 100 + 10);
	glEnd();

	glBegin(GL_LINES);
	glVertex2i((x + widthGameOver) * 2 / 3 - 4, (y + heightGameOver) * 40 / 100 + 10);
	glVertex2i((x + widthGameOver) * 2 / 3 - 1, (y + heightGameOver) * 41 / 100 + 10);
	glVertex2i((x + widthGameOver) * 2 / 3 - 1, (y + heightGameOver) * 41 / 100 + 10);
	glVertex2i((x + widthGameOver) * 2 / 3 + 2, (y + heightGameOver) * 40 / 100 + 10);
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int j = 16; j < 30; j++)
	{
		float angle = 2 * PI * j / 30;
		glVertex2f((x + widthGameOver) * 2 / 3 + cos(angle) * 20, (y + heightGameOver) * 39 / 100 + sin(angle) * 20 + 10);
	}
	glEnd();
	PlaySound(TEXT("win"), NULL, SND_ASYNC);
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

	if (isGameOver) {
		drawGameOver();
	}

	if (time == 3599 || score >= 1000) {
		isGameOver = true;
		isPaused = true;
		drawWonTheGame();
	}

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
	glutTimerFunc(1000, timeCounter, 0);

	glutMainLoop();
}