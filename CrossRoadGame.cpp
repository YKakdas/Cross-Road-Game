#include "CrossRoadGame.h" // includes global variables,constant variables and function prototypes

void timeCounter(int id) { // for counting time
	if (isPaused) {
		return;
	}
	if (!isPaused || !isGameOver) {
		time++;
		glutTimerFunc(1000, timeCounter, 0);
	}
	if (time == 120) { // If game reaches 120. seconds, player is won. Stop the game and call redisplay to show winning info panel
		isGameWon = true;
		isPaused = true;
		glutPostRedisplay();
	}
}

vector<Car> getCarsFromGivenLineNumber(int lineNumber) { // To find respective cars from the given line number
	vector<Car> carsInGivenLine;
	for (int i = 0; i < carVector.size(); i++) {
		if (carVector[i].lineNumber == lineNumber) {
			carsInGivenLine.push_back(carVector[i]);
		}
	}
	return carsInGivenLine;
}

vector<Truck> getTrucksFromGivenLineNumber(int lineNumber) { // To find respective trucks from the given line number
	vector<Truck> trucksInGivenLine;
	for (int i = 0; i < truckVector.size(); i++) {
		if (truckVector[i].lineNumber == lineNumber) {
			trucksInGivenLine.push_back(truckVector[i]);
		}
	}
	return trucksInGivenLine;
}

vector<Lane> getLanesFromGivenLineNumber(int lineNumber) { // To find respective lanes from the given line number
	vector<Lane> lanesInGivenLine;
	for (int i = 0; i < lanes.size(); i++) {
		if (lanes[i].lineNumber == lineNumber) {
			lanesInGivenLine.push_back(lanes[i]);
		}
	}
	return lanesInGivenLine;
}

int getHeightOfGivenLineNumber(int lineNumber) { // returns given line's height
	for (int i = 0; i < lanes.size(); i++) {
		if (lanes[i].lineNumber == lineNumber) {
			return lanes[i].start.y;
		}
	}
	return -1;
}

char getDirectionOfLine(int lineNumber) { // returns direction of given line ( left or right )
	for (int i = 0; i < lanes.size(); i++) {
		if (lanes[i].lineNumber == lineNumber) {
			return lanes[i].direction;
		}
	}
	return 'E'; // indicates error
}

/* this method checks is there any car or truck at the start or end position of line. If I don't check this,
two vehicle may be generated one on top of another and collision may occur. To prevent collision of car and truck, this function is implemented.*/
GLboolean isThereAnyCarOrTruckInThatLineEdges(GLint y, GLint lineNumber) {
	vector<Car> cars = getCarsFromGivenLineNumber(lineNumber);
	vector<Truck> trucks = getTrucksFromGivenLineNumber(lineNumber);

	for (int i = 0; i < cars.size(); i++) {
		if (cars[i].start.y == y) {
			if (cars[i].start.x < CAR_HALF_SIZE * 4 || cars[i].end.x > width - CAR_HALF_SIZE * 2) { // checking bounds of line
				return true;
			}
		}
	}

	for (int i = 0; i < trucks.size(); i++) {
		if (trucks[i].start.y == y) {
			if (trucks[i].start.x < TRUCK_HALF_SIZE * 6 || trucks[i].end.x > width - CAR_HALF_SIZE * 4) { // checking bounds of line
				return true;
			}
		}
	}
	return false;
}

void fillSideWalksVector() { // Creates sidewalks according to generic global variables, thus when resize, they will scale.
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

void fillLanesVector() { // Creates lanes according to generic global variables, thus when resize, they will scale.
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

				/* for even lines, make vehicles go from left to right, for odds from right to left( design choice ) */
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

void agentInit() { // first values of agent is assigned in this function

	SideWalk agentSideWalk = sideWalks[0]; // initial position of agent is bottom sidewalk

	// calculations for making proper isosceles triangle
	GLint agentLeftX = ((agentSideWalk.start.x) + (agentSideWalk.end.x)) / 2 - GAP_BETWEEN_LANES_VERTICALLY / 4;
	GLint agentRightX = agentLeftX + GAP_BETWEEN_LANES_VERTICALLY / 2;
	GLint agentUpX = agentLeftX + GAP_BETWEEN_LANES_VERTICALLY / 4;
	GLint agentLeftAndRightY = agentSideWalk.start.y;
	GLint agentUpY = agentLeftAndRightY + GAP_BETWEEN_LANES_VERTICALLY;

	Point2D leftVertex = { agentLeftX ,agentLeftAndRightY };
	Point2D rightVertex = { agentRightX ,agentLeftAndRightY };
	Point2D upVertex = { agentUpX ,agentUpY };

	// set agent's color to red
	Color color = { 255,0,0 };

	agent.leftVertex = leftVertex;
	agent.rightVertex = rightVertex;
	agent.upVertex = upVertex;
	agent.direction = 'U'; // initial direction of agent is from bottom to top( UP )
	agent.color = color;
}
void randomVehicleGenerator(int id) { // function to generate random car or trucks

	if (isPaused) { // if game is paused, do not generate vehicles
		return;
	}
	int carType = rand() % 2; // if == 0 generate car else truck
	int direction = rand() % 2; // deciding where to generate car(start of the line or end of the lane)
	int lineNumber = rand() % TOTAL_NUMBER_OF_LINES; // which line to be generated on
	int x; // will be determined according to direction
	int y = getHeightOfGivenLineNumber(lineNumber) + 2; // y coordiante of bottom of vehicle i.e. corresponding line's y value

	// to generate different colors of vehicles but not tone of red to not confuse with agent
	int r = rand() % 200;
	int g = (rand() % 200) + 56;
	int b = (rand() % 200) + 56;

	Color color = { r, g, b };
	int velocity = 6; // Initial speed of vehicle by pixels

	if (isThereAnyCarOrTruckInThatLineEdges(y, lineNumber)) { // if there is vehicle to be collide, do not generate, reset the timer
		glutTimerFunc(randomVehicleGeneratorPeriod, randomVehicleGenerator, 0);
		return;
	}

	// If there is no vehicle in that line, give velocity as 6 but if there is, give velocity slightly less(5) to prevent collision
	if (getCarsFromGivenLineNumber(lineNumber).size() != 0 || getTrucksFromGivenLineNumber(lineNumber).size() != 0) {
		velocity = 5;
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

void randomCoinGenerator(int id) { // generates random coins

	if (isPaused) {
		return;
	}

	if (coinVector.size() > 5) { // every 6th generation of coin, remove the oldest one
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
	glutTimerFunc(randomCoinGeneratorPeriod, randomCoinGenerator, 0);
}
void updateVehicleLocation(int id) { // updates locations of vehicles by their velocity

	if (isPaused) { // If game is paused, do not update vehicle location
		return;
	}
	for (int i = 0; i < carVector.size(); i++) {
		char direction = getDirectionOfLine(carVector[i].lineNumber);
		if (direction == 'L') {
			carVector[i].start.x = carVector[i].start.x - carVector[i].velocity;
			carVector[i].end.x = carVector[i].end.x - carVector[i].velocity;
			if (carVector[i].start.x < -10) { // if car passes the boundry of window, remove it from game
				carVector.erase(carVector.begin() + i);
			}
		}
		else if (direction == 'R') {
			carVector[i].start.x = carVector[i].start.x + carVector[i].velocity;
			carVector[i].end.x = carVector[i].end.x + carVector[i].velocity;
			if (carVector[i].start.x > width + 10) { // if car passes the boundry of window, remove it from game
				carVector.erase(carVector.begin() + i);
			}
		}
	}

	for (int i = 0; i < truckVector.size(); i++) {
		char direction = getDirectionOfLine(truckVector[i].lineNumber);
		if (direction == 'L') {
			truckVector[i].start.x = truckVector[i].start.x - truckVector[i].velocity;
			truckVector[i].end.x = truckVector[i].end.x - truckVector[i].velocity;
			if (truckVector[i].start.x < -10) {// if truck passes the boundry of window, remove it from game
				truckVector.erase(truckVector.begin() + i);
			}
		}
		else if (direction == 'R') {
			truckVector[i].start.x = truckVector[i].start.x + truckVector[i].velocity;
			truckVector[i].end.x = truckVector[i].end.x + truckVector[i].velocity;
			if (truckVector[i].start.x > width + 10) {// if truck passes the boundry of window, remove it from game
				truckVector.erase(truckVector.begin() + i);
			}
		}
	}
	checkCollisions(); // when update the location of vehicle, check if there ay collision with agent
	glutTimerFunc(updateVehiclePeriod, updateVehicleLocation, 0);
	glutPostRedisplay(); // call redisplay function to update locations
}

int getLineNumberOfAgent() { // returns the agent's line number
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
void checkCollisions() { // checks collisions of vehicles and coins with agent using rectangle bounding method
	GLint lineNumber = getLineNumberOfAgent();

	Rect agentRect;

	// make rectangle box from agent
	if (agent.direction == 'U') {
		agentRect = { agent.leftVertex.x,agent.leftVertex.y,AGENT_WIDTH,AGENT_HEIGHT };
	}
	else {
		agentRect = { agent.upVertex.x - AGENT_WIDTH / 2,agent.upVertex.y,AGENT_WIDTH,AGENT_HEIGHT };
	}

	for (int i = 0; i < coinVector.size(); i++) {
		Coin coin = coinVector[i];
		Rect coinRect = { coin.center.x - coin.radius,coin.center.y - coin.radius, coin.radius * 2,coin.radius * 2 }; // make rectangle box of coin

		if (coinRect.x < agentRect.x + agentRect.width &&
			coinRect.x + coinRect.width > agentRect.x &&
			coinRect.y < agentRect.y + agentRect.height &&
			coinRect.y + coinRect.height > agentRect.y) {
			PlaySound(TEXT("coin"), NULL, SND_ASYNC); // collecting coin sound
			score += 5; // if coin and agent collides, add 5 to score and remove that coin from game
			coinVector.erase(coinVector.begin() + i);
			break;
		}
	}

	if (lineNumber != -1) { // means that agent is not in the sidewalk thus collision may come up
		vector<Car> carsInThatLine = getCarsFromGivenLineNumber(lineNumber); // get cars in the line where agent stands
		vector<Truck> trucksInThatLine = getTrucksFromGivenLineNumber(lineNumber); // get trucks in the line where agent stands

		for (int i = 0; i < carsInThatLine.size(); i++) {
			Car car = carsInThatLine[i];
			Rect carRect = { car.start.x,car.start.y,CAR_HALF_SIZE * 2,CAR_HALF_SIZE * 2 }; // rectangle box of car

			if (carRect.x < agentRect.x + agentRect.width &&
				carRect.x + carRect.width > agentRect.x &&
				carRect.y < agentRect.y + agentRect.height &&
				carRect.y + carRect.height > agentRect.y) {
				PlaySound(TEXT("car-crash"), NULL, SND_ASYNC); // crash sound
				gameOver(); // if car and agent collides, game over
				break;
			}
		}

		for (int i = 0; i < trucksInThatLine.size(); i++) {
			Truck truck = trucksInThatLine[i];
			Rect truckRect = { truck.start.x,truck.start.y,TRUCK_HALF_SIZE * 4,TRUCK_HALF_SIZE * 2 }; // rectangle box of truck

			if (truckRect.x < agentRect.x + agentRect.width &&
				truckRect.x + truckRect.width > agentRect.x &&
				truckRect.y < agentRect.y + agentRect.height &&
				truckRect.y + truckRect.height > agentRect.y) {
				PlaySound(TEXT("car-crash"), NULL, SND_ASYNC); // crash sound
				gameOver(); // if truck and agent collides, game over
				break;
			}
		}
	}
}
void turnAgentDown() { // If agent reaches the top sidewalk, turn it down

	agent.leftVertex.y = agent.upVertex.y;
	agent.upVertex.y = agent.rightVertex.y;
	agent.rightVertex.y = agent.leftVertex.y;
	agent.direction = 'D';
	glutPostRedisplay();
}

void turnAgentUp() { // If agent reaches the bottom sidewalk, turn it up

	agent.leftVertex.y = agent.upVertex.y;
	agent.upVertex.y = agent.rightVertex.y;
	agent.rightVertex.y = agent.leftVertex.y;
	agent.direction = 'U';
	glutPostRedisplay();
}
void agentMoveUp() { // If user presses up arrow key

	if (agent.direction == 'D') { // If it's direction is down, game over, it is not allowed.
		gameOver();
		return;
	}
	agent.leftVertex.y = agent.leftVertex.y + GAP_BETWEEN_LANES_VERTICALLY;
	agent.rightVertex.y = agent.leftVertex.y;
	agent.upVertex.y = agent.upVertex.y + GAP_BETWEEN_LANES_VERTICALLY;

	if (agent.upVertex.y >= sideWalks[sideWalks.size() - 1].start.y + GAP_BETWEEN_LANES_VERTICALLY) {
		turnAgentDown(); // Check if agent reaches top sidewalk
	}

	score++; // If everything is ok, then add 1 to score
	checkCollisions(); // Agent is moved, so check the collisions
	glutPostRedisplay();
}

void agentMoveDown() { // If user presses down arrow key

	if (agent.direction == 'U') { // If it's direction is up, game over, it is not allowed.
		gameOver();
		return;
	}
	agent.leftVertex.y = agent.leftVertex.y - GAP_BETWEEN_LANES_VERTICALLY;
	agent.rightVertex.y = agent.leftVertex.y;
	agent.upVertex.y = agent.upVertex.y - GAP_BETWEEN_LANES_VERTICALLY;

	if (agent.upVertex.y <= sideWalks[0].end.y - GAP_BETWEEN_LANES_VERTICALLY) {
		turnAgentUp(); // Check if agent reaches bottom sidewalk
	}
	score++; // If everything is ok, then add 1 to score
	checkCollisions(); // Agent is moved, so check the collisions
	glutPostRedisplay();
	isOneStepMode = false;
}
void agentMoveLeft() { // If user presses left arrow key

	if (agent.leftVertex.x - 5 >= 0) { // If agent exceeds window bounds, do not move
		agent.leftVertex.x = agent.leftVertex.x - 5;
		agent.rightVertex.x = agent.rightVertex.x - 5;
		agent.upVertex.x = agent.upVertex.x - 5;
		checkCollisions(); // Agent is moved, so check the collisions
		glutPostRedisplay();
	}
}

void agentMoveRight() { // If user presses right arrow key

	if (agent.rightVertex.x + 5 <= width) { // If agent exceeds window bounds, do not move
		agent.leftVertex.x = agent.leftVertex.x + 5;
		agent.rightVertex.x = agent.rightVertex.x + 5;
		agent.upVertex.x = agent.upVertex.x + 5;
		checkCollisions(); // Agent is moved, so check the collisions
		glutPostRedisplay();
	}
	isOneStepMode = false;
}

void gameOver() { // If game is over, stop the game and redisplay to show game over info panel
	isGameOver = true;
	isPaused = true;
	glutPostRedisplay();
}

void gameRestart() { // If key 'r' is pressed to restart, reset everything to their initial states
	isPaused = false;
	isGameOver = false;
	isGameWon = false;
	score = 0;
	time = 0;
	agentInit();
	carVector.clear();
	truckVector.clear();
	coinVector.clear();
	glutTimerFunc(randomVehicleGeneratorPeriod, randomVehicleGenerator, 0);
	glutTimerFunc(updateVehiclePeriod, updateVehicleLocation, 0);
	glutTimerFunc(randomCoinGeneratorPeriod, randomCoinGenerator, 0);
	glutTimerFunc(1000, timeCounter, 0);
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

	/* calculate new locations of agent for new window size */

	agent.leftVertex.x = agent.leftVertex.x * w / width;
	agent.rightVertex.x = agent.leftVertex.x + GAP_BETWEEN_LANES_VERTICALLY / 2;
	agent.upVertex.x = agent.leftVertex.x + GAP_BETWEEN_LANES_VERTICALLY / 4;
	agent.leftVertex.y = agent.leftVertex.y * h / height;
	agent.rightVertex.y = agent.leftVertex.y;
	agent.upVertex.y = agent.leftVertex.y + GAP_BETWEEN_LANES_VERTICALLY;
	width = w;
	height = h;

	/* calculate global variables according to new size to scale drawing components to new size */

	SCOREBOARD_SIZE = floor((GLdouble)height / (GLdouble)11);
	gameWindowHeight = height - SCOREBOARD_SIZE;

	SIDEWALK_WIDTH = ceil(((GLdouble)gameWindowHeight / (GLdouble)16));
	ROAD_WIDTH = ceil((GLdouble)gameWindowHeight / (GLdouble)8);
	LANE_LENGTH = ceil((GLdouble)width / (GLdouble)26);

	GAP_BETWEEN_LANES_HORIZONTALLY = ceil((GLdouble)width / (GLdouble)34);
	GAP_BETWEEN_LANES_VERTICALLY = ceil((GLdouble)gameWindowHeight / (GLdouble)32);

	CAR_HALF_SIZE = ceil((GLdouble)(GAP_BETWEEN_LANES_VERTICALLY - GAP_BETWEEN_LANES_VERTICALLY / 5) / (GLdouble)2);
	TRUCK_HALF_SIZE = ceil((GLdouble)(GAP_BETWEEN_LANES_VERTICALLY - GAP_BETWEEN_LANES_VERTICALLY / 5) / (GLdouble)2);

	sideWalks.clear();
	fillSideWalksVector();
	lanes.clear();
	fillLanesVector();

	/* Previous Cars and Trucks are cleared otherwise when screen size
	   changes, shape and coordinates of vehicles would change and this is not desired case. If I try to keep
	   shapes of vehicles, then collisions may occur or they may enter the sidewalks,
	   if I try to scale them, their shape will be broken. For example
	   car should be square but after scaling they may become rectangle which is definition of truck. So trucks may look
	   like car and likewise cars may look like trucks. Hence, as a design choice, I prefer to clear all cars and trucks and
	   generate them from scratch
	   */

	carVector.clear();
	truckVector.clear();
}

void myinit(void) { // Filling vectors and initializing agent
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
	if ((key == 'Q') || (key == 'q')) // if 'q' is pressed, quit the game
		exit(0);
	else if ((key == 'R') || (key == 'r')) { // if game is over or game is won, if 'r' is pressed, restart the game
		if (isGameOver || isGameWon)
			gameRestart();
	}

	/* key 1-2-3 are used to define hardness level of the game.(2 is default one) 
	   1 is the easiest mode, frequency of generating vehicles are less and coins dissapear lately
	   3 is the hardest mode, frequency of generating vehicles are more than other modes and coins dissappear quickly.
	*/
	else if (key == '1') {
		randomVehicleGeneratorPeriod = 200;
		randomCoinGeneratorPeriod = 2000;
	}
	else if (key == '2') {
		randomVehicleGeneratorPeriod = 100;
		randomCoinGeneratorPeriod = 1000;
	}
	else if (key == '3') {
		randomVehicleGeneratorPeriod = 20;
		randomCoinGeneratorPeriod = 500;
	}
}

void myKeyboardSpecial(int key, int x, int y) {
	keyboardHistory.push_back(key); // fill keys into vector for one step mode
	if (isPaused || isOneStepMode) { // if game is paused or in one step mode do not move agent
		return;
	}
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
		if (!isPaused) { // if game is not paused,first pause it
			isPaused = !isPaused;
		}
		else { // if game is paused
			if (!isGameOver) { // if game is not over
				isPaused = false;
				isOneStepMode = true; // enable one step mode
				if (keyboardHistory.size() != 0) { // check the keyboard history, if it is not empty
					int key = keyboardHistory.back(); // get last pressed keyboard key
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
					keyboardHistory.clear(); // after one step is completed, clear keyboard history
				} 
				// call each timer functions for just 1 time in one step mode
				updateVehicleLocation(0);
				randomVehicleGenerator(0);
				randomCoinGenerator(0);
				timeCounter(0);
				isPaused = true;
			}
		}
		gameInfo(); // at every one step mode is applied, write game informations on console for debugging purposes
	}
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (isGameOver) { // if game is over do nothing
			return;
		}
		if (isPaused) { // if game is paused, reset the timers and resume the game
			isOneStepMode = false;
			glutTimerFunc(updateVehiclePeriod, updateVehicleLocation, 0);
			glutTimerFunc(randomVehicleGeneratorPeriod, randomVehicleGenerator, 0);
			glutTimerFunc(randomCoinGeneratorPeriod, randomCoinGenerator, 0);
			glutTimerFunc(1000, timeCounter, 0);
		}
		isPaused = !isPaused;
	}
}

void drawScoreBoard() { // draws scoreboard at the top of the window
	glColor3ub(190, 190, 190);
	glRecti(0, gameWindowHeight, width, height);
	int minute = time / 60;
	string timeStr = "TIME : " + to_string(minute / 10) + to_string((minute % 10)) + ":" + to_string((time / 10) % 6) + to_string((time % 10));
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
	glRasterPos2i((x + 10), (y + heightGameOver) * 21 / 50);
	for (int i = 0; i < exit.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, exit[i]);
	}

	string restart = "Press r for exit the game";
	glRasterPos2i((x + 10), (y + heightGameOver) * 19 / 50);
	for (int i = 0; i < restart.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, restart[i]);
	}

	glRasterPos2i((x + widthGameOver) * 2 / 3, (y + heightGameOver) * 3 / 5);
	glBegin(GL_LINE_LOOP);
	for (int j = 0; j < 30; j++)
	{
		float angle = 2 * PI * j / 30;
		glVertex2f((x + widthGameOver) * 2 / 3 + cos(angle) * GAP_BETWEEN_LANES_VERTICALLY * 2,
			(y + heightGameOver) * 2 / 5 + sin(angle) * GAP_BETWEEN_LANES_VERTICALLY * 2 + GAP_BETWEEN_LANES_VERTICALLY / 2);
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
	for (int j = 2; j < 14; j++)
	{
		float angle = 2 * PI * j / 30;
		glVertex2f((x + widthGameOver) * 2 / 3 + cos(angle) * 20, (y + heightGameOver) * 36 / 100 + sin(angle) * 20 + 10);
	}
	glEnd();
}

void drawWonTheGame() {

	isGameWon = true;

	glColor3ub(0, 0, 0);
	GLint x = width / 10;
	GLint y = height / 3;
	GLint widthGameOver = 9 * width / 10;
	GLint heightGameOver = 2 * height / 3;
	glRecti(x, y, widthGameOver, heightGameOver);
	glColor3ub(255, 0, 0);
	string gameWon = "YOU WON THE GAME";
	string scoreStr = "YOUR SCORE IS : " + to_string(score);

	glRasterPos2i((x + widthGameOver) / 4, (y + heightGameOver) * 3 / 5);
	for (int i = 0; i < gameWon.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, gameWon[i]);
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
	glRasterPos2i((x + 10), (y + heightGameOver) * 21 / 50);
	for (int i = 0; i < exit.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, exit[i]);
	}

	string restart = "Press r for exit the game";
	glRasterPos2i((x + 10), (y + heightGameOver) * 19 / 50);
	for (int i = 0; i < restart.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, restart[i]);
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

void gameInfo() {

	cout << "<----------------------------------------------------------------------->\n";
	cout << "Informations about Agent \n\n";
	cout << "Direction of agent is : " << agent.direction << "\n";
	cout << "Left vertex of agent is : (" << agent.leftVertex.x << " " << agent.leftVertex.y << ") \n";
	cout << "Right vertex of agent is : (" << agent.rightVertex.x << " " << agent.rightVertex.y << ") \n";
	cout << "Up vertex of agent is : (" << agent.upVertex.x << " " << agent.upVertex.y << ") \n";
	cout << "<----------------------------------------------------------------------->\n";

	cout << "Informations about Vehicles \n\n";

	for (int i = 0; i < NUMBER_OF_LINES_PER_ROAD*NUMBER_OF_ROADS; i++) {
		vector<Car> cars = getCarsFromGivenLineNumber(i);
		vector<Truck> trucks = getTrucksFromGivenLineNumber(i);

		if (cars.size() > 0) {
			cout << "Cars in the line " << i << ":\n\n";
			cout << "Direction of cars is : " << getDirectionOfLine(i) << "\n";
			for (int j = 0; j < cars.size(); j++) {
				cout << "Left bottom vertex of car is : (" << cars[j].start.x << " " << cars[j].start.y << ") \n";
				cout << "Right top vertex of car is : (" << cars[j].end.x << " " << cars[j].end.y << ") \n";
				cout << "Width of the car is : " << CAR_HALF_SIZE * 2 << "\n";
				cout << "Height of the car is : " << CAR_HALF_SIZE * 2 << "\n";
				cout << "Velocity of the car is : " << cars[j].velocity << "\n";
				cout << "<------------------------------------>\n";
			}
		}

		if (trucks.size() > 0) {
			cout << "Trucks in the line " << i << "\n\n";
			cout << "Direction of trucks is : " << getDirectionOfLine(i) << "\n";
			for (int j = 0; j < trucks.size(); j++) {
				cout << "Left bottom vertex of truck is : (" << trucks[j].start.x << " " << trucks[j].start.y << ") \n";
				cout << "Right top vertex of car is : (" << trucks[j].end.x << " " << trucks[j].end.y << ") \n";
				cout << "Width of the truck is : " << TRUCK_HALF_SIZE * 4 << "\n";
				cout << "Height of the truck is : " << TRUCK_HALF_SIZE * 2 << "\n";
				cout << "Velocity of the truck is : " << trucks[j].velocity << "\n";
				cout << "<------------------------------------>\n";
			}
		}
	}
}
void myDisplay(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	/* draw each component by order */
	drawScoreBoard();
	drawSidewalks();
	drawLanes();
	drawCars();
	drawTrucks();
	drawAgent();
	drawCoins();

	if (isGameOver && !isGameWon) { // if game is over draw game over board
		drawGameOver();
	}

	if (time == 120 || score >= 100 && !isGameOver) { // if time is reached to 120 or score passes 100, draw the game won board
		isPaused = true;
		drawWonTheGame();
	}
	glFlush();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(0, 0);
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