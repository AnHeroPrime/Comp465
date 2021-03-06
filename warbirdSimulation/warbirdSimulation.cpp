/*
warbirdSimulation.cpp

465 utility include files:  shader465.hpp, triModel465.hpp  

This program is constructed from the manyModelsStatic example the models are indexed
using arrays.

Keys: 
'v' moves to the next camera 'x' moves to the previous camera
'w' warps the ship
's' changes ship speed
'f' fires missle
arrow keys move
ctrl movement modifier
't' change TQ
light toggles: 'p' point, 'h' headlight, 'd' debug color

Steven Blachowiak, Aaron Scott
9/29/2016
*/

// define your target operating system: __Windows__  __Linux__  __Mac__
# define __Windows__ 
# include "../includes465/include465.hpp"

const int X = 0, Y = 1, Z = 2, START = 0, STOP = 1, ruber = 0, unum = 1, duo = 2, primus = 3, secundus = 4, ship = 5, missile_1 = 6, missile_2 = 7, missileBase_1 = 8, missileBase_2 = 9, missile_3 = 10, skybox = 11, gravityMax = 90000000;
int currentWarp = 1; // Warp set to Unum
int currentCam = 1; // start in ship view
int TQ = 5;
int speed = 10;
int gravity = 0;
int accelerate = 0; //variables for movement keys
int yaw = 0;
int pitch = 0;
int roll = 0;
bool fire = false;
bool unumBaseFire = false;
bool moonBaseFire = false;
bool nextCam = false;
bool previousCam = false;
bool warp = false;
bool canFire = true;
bool playerCollision = false;
bool missileBase1Collision = false;
bool missileBase2Collision = false;
bool initialUpdate = true;
bool pointLightSetOn = true;
bool headLightSetOn = true;
bool debugSetOn = false;
bool ambientOn = true;
// constants for models:  file names, vertex count, model display size
const int nModels = 12;  // number of models in this scene
const int nTextures = 6; // number of textures
char * textureFile = { "skybox.raw" };
char * modelFile[nModels] = { "Ruber.tri", "Unum.tri", "Duo.tri", "Primus.tri", "Secundus.tri", "Warbird.tri", "Missile.tri", "Missile.tri", "Missilebase.tri", "Missilebase.tri", "Missile.tri","" };
float modelBR[nModels];       // model's bounding radius
float scaleValue[nModels];    // model's scaling "size" value
float gravityForce;
const int nVertices[nModels] = { 264 * 3, 264 * 3, 264 * 3, 264 * 3, 264 * 3, 996 * 3, 252 * 3, 252 * 3, 12 * 3, 12 * 3, 252 * 3, 2 * 3 };
char * vertexShaderFile   = "phase3Vertex.glsl";     
char * fragmentShaderFile = "phase3Fragment.glsl"; 
GLuint shaderProgram;
GLuint textures;
GLuint VAO[nModels];      // Vertex Array Objects
GLuint buffer[nModels];   // Vertex Buffer Objects
GLuint ibo, vTexCoord; //indexBufferObject

// Shader handles, matrices, etc
GLuint MVP ;  // Model View Projection matrix's handle
GLuint MV; // ModelView handle
GLuint NM; // NormalMatrix handle
GLuint TEX; //Texture handle
GLuint POINTLOCATION;
GLuint POINTINTENSITY;
GLuint POINTON;
GLuint HEADON;
GLuint HEADLOCATION;
GLuint HEADINTENSITY;
GLuint DEBUGON;
GLuint AMBIENTON;


GLuint vPosition[nModels], vColor[nModels], vNormal[nModels];   // vPosition, vColor, vNormal handles for models
// model, view, projection matrices and values to create modelMatrix.
float modelSize[nModels] = { 2000.0f, 200.0f, 400.0f, 100.0f, 150.0f, 100.0f, 45.0f, 45.0f, 30.0f, 30.0f, 45.0f, 50000.0f};   // size of model
float modelRadians[nModels] = { 0.0f, 0.004f, 0.002f, 0.004f, 0.002f, 0.0f, 0.0f, 0.0f, 0.004f, 0.002f, 0.0f, 0.0f };
float textureRadians = 1.57079632679489661923f;
glm::vec3 scale[nModels];       // set in init()
glm::vec3 translate[nModels] = { glm::vec3(0, 0, 0), glm::vec3(4000, 0, 0), glm::vec3(9000, 0, 0), glm::vec3(-900, 0, 0), glm::vec3(-1750, 0, 0), glm::vec3(5000, 1000, 5000), glm::vec3(4900, 1000, 4850), glm::vec3(4900, 1050, 4850), glm::vec3(4000, 225, 0), glm::vec3(-1750, 175, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) };
glm::mat4 rotation[nModels];
glm::mat4 texRotation; // used for rotating texture
glm::mat4 orientation[nModels];

static const GLfloat skyboxPoints[] = {
	-1.0f, -1.0f,  1.0f, 1.0f, // 0 bottom left back corner
	-1.0f, -1.0f, -1.0f, 1.0f, // 1 bottom left forward corner
	-1.0f,  1.0f,  1.0f, 1.0f, // 2 top left back corner
	-1.0f,  1.0f, -1.0f, 1.0f, // 3 top left forward corner
};

static const unsigned int indices[] = {
	0, 1, 2, // 0 left square bottom
	1, 2, 3, // 1 left square top
};


static const GLfloat texCoords[] = {
	0.0f, 0.0f,     // 0 bottom left
	1.0f, 0.0f,     // 1 bottom right
	0.0f, 1.0f,     // 2 top left
	1.0f, 1.0f,     // 3 top right
};

glm::mat4 modelMatrix;          // set in display()
glm::mat4 viewMatrix;           // set in init()
glm::mat4 modelViewMatrix;		// set in display()
glm::mat3 normalMatrix;
glm::mat4 projectionMatrix;     // set in reshape()
glm::mat4 ModelViewProjectionMatrix; // set in display();

// window title strings
char fpsStr[15];
char missileStr[50];
char viewStr[50] = "Camera: Ship View | ";
char baseStr[50] = "Warbird Simulation | ";
char base1Str[50] = "Unum base: Active | ";
char base2Str[50] = "Moon base: Active | ";
char gameStr[50] = "";
char titleStr[500];
int playerMissileCount = 9; // Player Missile Count
int base1MissileCount = 5;
int base2MissileCount = 5;
int frameCount = 0;
int missileTimerCount = 0;
int missile2TimerCount = 0;
int missile3TimerCount = 0;
double currentTime, lastTime, timeInterval;
glm::mat4 identity(1.0f);

//method identifyers
void warpShip();
void missileTracking(int missile);
bool orientAt(int originObject, int targetObject);
glm::mat4 cameraUpdate(int cam);

void reshape(int width, int height) {
  float aspectRatio = (float) width / (float) height;
  float FOVY = glm::radians(60.0f);
  glViewport(0, 0, width, height);
  printf("reshape: FOVY = %5.2f, width = %4d height = %4d aspect = %5.2f \n", 
    FOVY, width, height, aspectRatio);
  projectionMatrix = glm::perspective(FOVY, aspectRatio, 1.0f, 100000.0f); 
  }

// update and display animation state in window title
void updateTitle() {
	strcpy(titleStr, baseStr);
	strcat(titleStr, viewStr);
	strcat(titleStr, fpsStr);
	strcat(titleStr, missileStr);
	if (missileBase1Collision){
		 strcpy(base1Str,"Unum base: Destroyed | ");
	}
	if (missileBase2Collision){
		strcpy(base2Str,"Moon base: Destroyed | ");
	}
	if (missileBase1Collision && missileBase2Collision){
		strcpy(gameStr, "***YOU WIN*** | ");
	}
	if (playerCollision){
		strcpy(gameStr, "***YOU DIED*** | ");
	}
	strcat(titleStr, base1Str);
	strcat(titleStr, base2Str);
	strcat(titleStr, gameStr);
	glutSetWindowTitle(titleStr);
}

// Animate scene objects by updating their transformation matrices
void update(int i) {
	glutTimerFunc(TQ, update, 1);

	if (warp == true){ // warp ship
		warp = false;
		warpShip();
	}
	for (int m = 0; m < nModels; m++) {
		if (m == ship){
			rotation[m] = glm::rotate(rotation[m], float(pitch * 0.02), glm::vec3(1, 0, 0));
			rotation[m] = glm::rotate(rotation[m], float(yaw * 0.02), glm::vec3(0, 1, 0));
			rotation[m] = glm::rotate(rotation[m], float(roll * 0.02), glm::vec3(0, 0, 1));

			if (!initialUpdate) {
				translate[m] = getPosition(orientation[m]) + getIn(rotation[m]) * float(accelerate * speed);
			}
			else { //makes sure ship has initial position set
				initialUpdate = false;
			}
			orientation[m] = glm::translate(identity, translate[m]) * rotation[m] * glm::scale(identity, glm::vec3(scale[m]));

			if (gravity == 1) {
				gravityForce = gravityMax / pow(glm::distance(translate[ship], translate[ruber]),2);
				glm::vec3 gravityVector = getPosition(orientation[ruber]) - getPosition(orientation[ship]);
				gravityVector = glm::normalize(gravityVector);
				translate[m] = getPosition(orientation[m]) + gravityVector * gravityForce;
				orientation[m] = glm::translate(identity, translate[m]) * rotation[m] * glm::scale(identity, glm::vec3(scale[m]));
			}
			if(playerCollision){
				orientation[m] = rotation[m] * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(0, 0, 0));
			}
		}
		else{ // orbits
			rotation[m] = glm::rotate(rotation[m], modelRadians[m], glm::vec3(0, 1, 0));
			orientation[m] = rotation[m] * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(scale[m]));
		
			if (m == missileBase_1){
				if (missileBase1Collision){
					orientation[m] = rotation[m] * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(0,0,0));
				}
			}
			if (m == primus || m == secundus || m == missileBase_2){	// lunar orbits
				orientation[m] = glm::translate(identity, getPosition(orientation[duo])) * glm::rotate(rotation[m], modelRadians[m], glm::vec3(0, 1, 0)) * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(scale[m]));
				if (m == missileBase_2){
					if (missileBase2Collision){
						orientation[m] = rotation[m] * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(0, 0, 0));
					}
				}
			}
		}

		// missile updates
		if (m == missile_1){
			if (fire == false){
				translate[m] = getPosition(orientation[ship]);
				rotation[m] = rotation[ship];
				orientation[m] = glm::translate(identity, translate[m]) * rotation[m] * glm::scale(identity, glm::vec3(0,0,0));
			}
			if (fire == true) {
				missileTimerCount++; // player missle timer

				translate[m] = (translate[m] + getIn(orientation[m]) * 800.0f); // move missile forward
				orientation[m] = glm::translate(identity, translate[m]) * rotation[m] * glm::scale(identity, glm::vec3(scale[m]));

				if (missileTimerCount > 200){ // start missile tracking after 200 frames
					missileTracking(missile_1);
				}

				if (missileTimerCount >= 2000){ // kill missle after 2000 frames
					missileTimerCount = 0;
					fire = false;
					if (playerMissileCount == 0){
						canFire = false;
					}
				}
			}
		}
		else if (m == missile_2 && !missileBase1Collision){ // unum missle updates

			if (unumBaseFire == false || missileBase1Collision){ // set missles translation and rotation matrix to that of the base
				translate[m] = getPosition(orientation[missileBase_1]);
				rotation[m] = rotation[missileBase_1];
				orientation[m] = glm::translate(identity, translate[m]) * rotation[m] * glm::scale(identity, glm::vec3(0, 0, 0));
			}

			if (distance(getPosition(orientation[missileBase_1]),getPosition(orientation[ship])) < 5000 && missile2TimerCount == 0){ // proximity check
				unumBaseFire = true;
			}

			if (unumBaseFire == true){
				missile2TimerCount++;

				translate[m] = translate[m] + getIn(orientation[m]) * 400.0f; // move missile forward
				orientation[m] = glm::translate(identity, translate[m]) * rotation[m] * glm::scale(identity, scale[m]);

				//if (missile2TimerCount > 200){ // start tracking after 200 updates
					missileTracking(missile_2);
				//}
				if (missile2TimerCount > 2000){ // kill missle after 2000 updates
					unumBaseFire = false;
					missile2TimerCount = 0;
				}
			}
		}
		else if (m == missile_3 && !missileBase2Collision){ // moon missle updates
		
			if (moonBaseFire == false || missileBase2Collision){ // set missles translation and rotation matrix to that of the base
				translate[m] = getPosition(orientation[missileBase_2]);
				rotation[m] = rotation[missileBase_2];
				orientation[m] = glm::translate(identity, translate[m]) * rotation[m] * glm::scale(identity, glm::vec3(0, 0, 0));
			}

			if (distance(getPosition(orientation[missileBase_2]), getPosition(orientation[ship])) < 5000 && missile3TimerCount == 0){ // proximity check
				moonBaseFire = true;
			}

			if (moonBaseFire == true){
				missile3TimerCount++;
				
				translate[m] = translate[m] + getIn(orientation[m]) * 400.0f; // move missile forward
				orientation[m] = glm::translate(identity, translate[m]) * rotation[m] * glm::scale(identity, scale[m]);
				
				//if (missile3TimerCount > 200){ // start tracking after 200 updates
					missileTracking(missile_3);
				//}
				if (missile3TimerCount > 2000){ // kill missle after 2000 updates
					moonBaseFire = false;
					missile3TimerCount = 0;
				}
			
			}
		}
	}

	if (glm::distance(getPosition(orientation[ship]), getPosition(orientation[ruber])) < (modelSize[ruber] + modelSize[ship])) {
		playerCollision = true;
		printf("Sun Collision");
	}
	else if (glm::distance(getPosition(orientation[ship]), getPosition(orientation[unum])) < (modelSize[unum] + modelSize[ship])) {
		playerCollision = true;
		printf("Unum Collision");
	}
	else if (glm::distance(getPosition(orientation[ship]), getPosition(orientation[duo])) < (modelSize[duo] + modelSize[ship])) {
		playerCollision = true;
		printf("Duo Collision");
	}
	else if (glm::distance(getPosition(orientation[ship]), getPosition(orientation[primus])) < (modelSize[primus] + modelSize[ship])) {
		playerCollision = true;
		printf("Primus Collision ");
	}
	else if (glm::distance(getPosition(orientation[ship]), getPosition(orientation[secundus])) < (modelSize[secundus] + modelSize[ship])) {
		playerCollision = true;
		printf("Secundus Collision ");
	}
	else if (glm::distance(getPosition(orientation[ship]), getPosition(orientation[missile_2])) < (modelSize[missile_2] + modelSize[ship])) {
		playerCollision = true;
		unumBaseFire = false;
		printf("missile_2 HIT");
	}
	else if (glm::distance(getPosition(orientation[ship]), getPosition(orientation[missile_3])) < (modelSize[missile_3] + modelSize[ship])) {
		playerCollision = true;
		moonBaseFire = false;
		printf("missile_3 HIT");
	}
	if (glm::distance(getPosition(orientation[missile_1]), getPosition(orientation[missileBase_1])) < (modelSize[missileBase_1] + modelSize[missile_1] + 100)) {
		missileBase1Collision = true;
		fire = false;
		translate[missile_2] = glm::vec3(0, 0, 0);
		printf("Base1 HIT ");
	}
	if (glm::distance(getPosition(orientation[missile_1]), getPosition(orientation[missileBase_2])) < (modelSize[missileBase_2] + modelSize[missile_1] + 100)) {
		missileBase2Collision = true;
		fire = false;
		translate[missile_3] = glm::vec3(0, 0, 0);
		printf("Base2 HIT ");
	}

	pitch = yaw = roll = accelerate = 0; //stop rotations if key is let go of

	viewMatrix = cameraUpdate(0); //Update dynamic cameras
	sprintf(missileStr, "Missiles: %d | ", playerMissileCount);
	glutPostRedisplay();
}

void missileTracking(int missile){
	
	int target = -1;

	if (missile == missile_1){ // missile is fired from the ship, pick closest base
		if (distance(getPosition(orientation[missile]), getPosition(orientation[missileBase_1])) < distance(getPosition(orientation[missile]), getPosition(orientation[missileBase_2]))){
			if (!missileBase1Collision){
				target = missileBase_1; // target unum base
			}
		}
		else{ 
			if (!missileBase2Collision){
				target = missileBase_2; // target moon base
			}
		}
	}
	else { // missle is fired from a base, its target is the ship
		target = ship;
	}

	if (distance(getPosition(orientation[missile]), getPosition(orientation[target])) > 5000){ // if no target is found within 5000 units set target to -1
		target = -1;
	}

	if (target > 0){ // if missile has a target, track target
		orientAt(missile, target);
	}
}

void warpShip(){
	int warpPoint;
	if (currentWarp == 1){
		currentWarp = 2;
		warpPoint = unum;
	}
	else{
		currentWarp = 1;
		warpPoint = duo;
	}
	translate[ship] = getPosition(orientation[warpPoint]) + getIn(rotation[warpPoint]) * 8000.0f; // warps ship to position. No rotation.
	rotation[ship] = identity;
	orientation[ship] = glm::translate(identity, translate[ship]) * rotation[ship] * glm::scale(identity, glm::vec3(scale[ship]));
	orientAt(ship, warpPoint);
}

bool orientAt(int originObject, int targetObject){
	float radian;
	glm::vec3 originObjectAt = getIn(rotation[originObject]);
	glm::vec3 target = getPosition(orientation[targetObject]) - getPosition(orientation[originObject]);
	glm::vec3 normTarget = glm::normalize(target); // normalized target vector
	glm::vec3 rotationAxis = glm::vec3(0, 0, 0);//
	rotationAxis = glm::cross(normTarget, originObjectAt);
	rotationAxis = glm::normalize(rotationAxis);
	float rotationAxisDirection = rotationAxis.x + rotationAxis.y + rotationAxis.z;
	float rotationRads = glm::dot(normTarget, originObjectAt);
	radian = (2 * PI) - glm::acos(rotationRads);
	if (colinear(originObjectAt, normTarget, .08)){ // check for colinearity
		if (distance(getPosition(orientation[originObject]) + originObjectAt, target) > distance(getPosition(orientation[originObject]), target)){ // check for bad colinear situations
			rotation[originObject] = glm::rotate(rotation[originObject], PI, getUp(rotation[originObject])); // turn missle around
			//printf("BAD_COLINEAR");
		}
		return true;
	}
	else{
		if (distance(normTarget,originObjectAt) < .5){
			//no rotation
		}
		else{
			rotation[originObject] = glm::rotate(rotation[originObject], radian, rotationAxis);
		}
		return false;
	}
}

glm::mat4 cameraUpdate(int cam){
	if (nextCam == true){ // switch camera forward
		nextCam = false;
		if (currentCam == 5){
			currentCam = 1;
		}
		else{
			currentCam++;
		}
	}
	if (previousCam == true){ // switch camera back
		previousCam = false;
		if (currentCam == 1){
			currentCam = 5;
		}
		else{ 
			currentCam--; 
		}
	}
	if (cam <= 0){ // return currentCam mat4 if passed 0
		cam = currentCam;
	}
	if (cam == 1){ // ship
		if (!playerCollision){
			sprintf(viewStr, "Camera: Ship View | ");
			return (glm::lookAt(getPosition(orientation[ship]) - getIn(rotation[ship]) * 1000.0f + getUp(rotation[ship]) * 300.0f, getPosition(orientation[ship] * glm::translate(identity, glm::vec3(0.0f, 300.0f, 0.0f))), getUp(rotation[ship])));
		}
		else{
			currentCam++;
			sprintf(viewStr, "Camera: Top View | ");
			return (glm::lookAt(glm::vec3(0.0f, 20000.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		}
	}
	else if (cam == 2){ //top view
		sprintf(viewStr, "Camera: Top View | ");
		return (glm::lookAt(glm::vec3(0.0f, 20000.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	}
	else if (cam == 3){ //front view
		sprintf(viewStr, "Camera: Front View | ");
		return (glm::lookAt(glm::vec3(0.0f, 10000.0f, 20000.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	}
	else if (cam == 4){ //Unum
		sprintf(viewStr, "Camera: Unum View | ");
		return (glm::lookAt(getPosition(orientation[unum]) + getIn(rotation[unum]) * 8000.0f, getPosition(orientation[unum]), glm::vec3(0.0f, 1.0f, 0.0f)));
	}
	else if (cam == 5){ //Duo
		sprintf(viewStr, "Camera: Duo View | ");
		return (glm::lookAt(getPosition(orientation[duo]) + getIn(rotation[duo]) * 8000.0f, getPosition(orientation[duo]), glm::vec3(0.0f, 1.0f, 0.0f)));
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 033: case 'q':  case 'Q': exit(EXIT_SUCCESS); break;
		case 'v':  // next cam
			nextCam = true;
			break;
		case 'x':  // last cam
			previousCam = true;
			break;
		case 'w':  // warp
			if (!playerCollision){
				warp = true;
			}
			break;
		case 't':  // TimeQuantum
			if (TQ == 5){ //trainee
				TQ = 40;
			}
			else if (TQ == 40){ //pilot
				TQ = 100;
			}
			else if (TQ == 100){ //ace
				TQ = 500;
			}
			else if (TQ == 500){ //debug
				TQ = 5;
			}
			break;
		case 's': //speed
			if (speed == 10) {
				speed = 50;
			}
			if (speed == 50) {
				speed = 200;
			}
			else { //speed = 200
				speed = 10;
			}
			break;
		case 'g': //gravity
			if (gravity == 1) {
				gravity = 0;
			}
			else { //gravity = 0
				gravity = 1;
			}
			break;
		case 'f': //fire
			if (fire == false && canFire && !playerCollision){
				fire = true;
				playerMissileCount--;
				if (playerMissileCount == 0){
					canFire = false;
				}
				//missileTimerCount = frameCount;
			}
			break;
		case 'p': // toggle point light
			if (pointLightSetOn){
				pointLightSetOn = false;
			}
			else{
				pointLightSetOn = true;
			}
			break;
		case 'h': // toggle head light
			if (headLightSetOn){
				headLightSetOn = false;
			}
			else{
				headLightSetOn = true;
			}
			break;
		case 'd': // light **Debug**
			if (debugSetOn){
				debugSetOn = false;
			}
			else{
				debugSetOn = true;
			}
			break;
		case 'a': // toggle ambient
			if (ambientOn){
				ambientOn = false;
			}
			else{
				ambientOn = true;
			}
			break;
	}
}

void specialKeys(int key, int x, int y) {
	int modifier = glutGetModifiers(); //checks for control key
	if (!playerCollision){
		if (modifier == GLUT_ACTIVE_CTRL) {
			if (key == GLUT_KEY_UP) {
				pitch = 1;
			}
			else if (key == GLUT_KEY_DOWN) {
				pitch = -1;
			}
			if (key == GLUT_KEY_RIGHT) {
				roll = 1;
			}
			else if (key == GLUT_KEY_LEFT) {
				roll = -1;
			}
		}
		else {
			if (key == GLUT_KEY_UP) {
				accelerate = 1;
			}
			else if (key == GLUT_KEY_DOWN) {
				accelerate = -1;
			}
			if (key == GLUT_KEY_RIGHT) {
				yaw = -1;
			}
			else if (key == GLUT_KEY_LEFT) {
				yaw = 1;
			}
		}
	}
}


void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // update model matrix
  glClearColor(0,0,0,0);
	for (int m = 0; m < nModels; m++) {
		if (m != skybox) {
			modelMatrix = orientation[m];
			modelViewMatrix = viewMatrix * modelMatrix;
			normalMatrix = glm::mat3(modelViewMatrix);
			ModelViewProjectionMatrix = projectionMatrix * modelViewMatrix;


			glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));
			glUniformMatrix4fv(MV, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
			glUniformMatrix3fv(NM, 1, GL_FALSE, glm::value_ptr(normalMatrix));
			glUniform3fv(POINTLOCATION, 1, glm::value_ptr(glm::vec3(0,0,0) * normalMatrix));
			glUniform3fv(POINTINTENSITY, 1, glm::value_ptr(glm::vec3(.2, .2, .2)));
			glUniform3fv(HEADLOCATION, 1, glm::value_ptr(getPosition(viewMatrix)));
			glUniform3fv(HEADINTENSITY, 1, glm::value_ptr(glm::vec3(.5, .5, .5)));
			glUniform1f(AMBIENTON, ambientOn);
			glUniform1f(HEADON, headLightSetOn);
			glUniform1f(POINTON, pointLightSetOn);
			glUniform1f(DEBUGON, debugSetOn);
			glBindVertexArray(VAO[m]);
			glUniform1f(TEX, false);
			glDrawArrays(GL_TRIANGLES, 0, nVertices[m]);
		}
		else {
			for (int n = 0; n < nTextures; n++) { //rotate texture in to cube position
				if (n < 4) {
					texRotation = rotation[m];
					texRotation = glm::rotate(texRotation, textureRadians * n, glm::vec3(0, 1, 0));
					orientation[m] = texRotation * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(scale[m]));
				}
				else if (n == 4){
					texRotation = rotation[m];
					texRotation = glm::rotate(texRotation, textureRadians, glm::vec3(0, 0, 1));
					orientation[m] = texRotation * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(scale[m]));
				}
				else{ //n = 5
					texRotation = rotation[m];
					texRotation = glm::rotate(texRotation, textureRadians * -1, glm::vec3(0, 0, 1));
					orientation[m] = texRotation * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(scale[m]));
				}
				

				modelMatrix = orientation[m];
				modelViewMatrix = viewMatrix * modelMatrix;
				normalMatrix = glm::mat3(modelViewMatrix);
				ModelViewProjectionMatrix = projectionMatrix * modelViewMatrix;


				glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));
				glUniformMatrix4fv(MV, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
				glUniformMatrix3fv(NM, 1, GL_FALSE, glm::value_ptr(normalMatrix));
				glUniform3fv(POINTLOCATION, 1, glm::value_ptr(glm::vec3(0, 0, 0) * normalMatrix));
				glUniform3fv(POINTINTENSITY, 1, glm::value_ptr(glm::vec3(.2, .2, .2)));
				glUniform3fv(HEADLOCATION, 1, glm::value_ptr(getPosition(viewMatrix)));
				glUniform3fv(HEADINTENSITY, 1, glm::value_ptr(glm::vec3(.5, .5, .5)));
				glUniform1f(AMBIENTON, ambientOn);
				glUniform1f(HEADON, headLightSetOn);
				glUniform1f(POINTON, pointLightSetOn);
				glUniform1f(DEBUGON, debugSetOn);
				glBindVertexArray(VAO[m]);
				glUniform1f(TEX, true);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
				glDrawElements(GL_TRIANGLES, nVertices[m], GL_UNSIGNED_INT, BUFFER_OFFSET(0));
			}
		}
	}
  glutSwapBuffers();
  frameCount++;
  // see if a second has passed to set estimated fps information
  currentTime = glutGet(GLUT_ELAPSED_TIME);  // get elapsed system time
  timeInterval = currentTime - lastTime;
  if (timeInterval >= 1000) {
	  sprintf(fpsStr, "FPS: %4d | ", (int)(frameCount / (timeInterval / 1000.0f)));
	  lastTime = currentTime;
	  frameCount = 0;
	  updateTitle();
  }
  }

// load the shader programs, vertex data from model files, create the solids, set initial view
void init() {
	// load the shader programs
	shaderProgram = loadShaders(vertexShaderFile, fragmentShaderFile);
	glUseProgram(shaderProgram);

	//load in texture
	GLuint texture = loadRawTexture(textures, textureFile, 1080, 1080);
	
  // generate VAOs and VBOs
  glGenVertexArrays( nModels, VAO );
  glGenBuffers( nModels, buffer );
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // set up the indexed skybox vertex attributes
  glBindVertexArray(VAO[skybox]);

  // initialize a buffer object
  glEnableVertexAttribArray(buffer[skybox]);
  glBindBuffer(GL_ARRAY_BUFFER, buffer[skybox]);
 
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxPoints) + sizeof(texCoords), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(skyboxPoints), skyboxPoints);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(skyboxPoints), sizeof(texCoords), texCoords);
 

  // set up vertex arrays (after shaders are loaded)
  vPosition[skybox] = glGetAttribLocation(shaderProgram, "vPosition");
  glVertexAttribPointer(vPosition[skybox], 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
  glEnableVertexAttribArray(vPosition[skybox]);

  vTexCoord = glGetAttribLocation(shaderProgram, "vTexCoord");
  glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(skyboxPoints)));
  glEnableVertexAttribArray(vTexCoord);

  // load the buffers from the model files

  for (int i = 0; i < nModels; i++) {
	  if (i != skybox) {
		  modelBR[i] = loadModelBuffer(modelFile[i], nVertices[i], VAO[i], buffer[i], shaderProgram,
			  vPosition[i], vColor[i], vNormal[i], "vPosition", "vColor", "vNormal");
	  }
	  else { //skybox
		  modelBR[i] = 1;
	  }
    // set scale for models given bounding radius  
    scale[i] = glm::vec3(modelSize[i] * 1.0f / modelBR[i]);
	
    }

   viewMatrix = glm::lookAt(
	glm::vec3(0.0f, 20000.0f, 0.0f),  // eye position
    glm::vec3(0),                   // look at position
    glm::vec3(0.0f, 0.0f, -1.0f)); // up vector 

  MVP = glGetUniformLocation(shaderProgram, "ModelViewProjection");
  MV = glGetUniformLocation(shaderProgram, "ModelView");
  NM = glGetUniformLocation(shaderProgram, "NormalMatrix");
  TEX = glGetUniformLocation(shaderProgram, "isTexture");
  POINTLOCATION = glGetUniformLocation(shaderProgram, "PointLightPosition");
  POINTINTENSITY = glGetUniformLocation(shaderProgram, "PointLightIntensity");
  HEADLOCATION = glGetUniformLocation(shaderProgram, "HeadLightPosition");
  HEADINTENSITY = glGetUniformLocation(shaderProgram, "HeadLightIntensity");
  POINTON = glGetUniformLocation(shaderProgram, "PointLightOn");
  HEADON = glGetUniformLocation(shaderProgram, "HeadLightOn");
  AMBIENTON = glGetUniformLocation(shaderProgram, "AmbientOn");
  DEBUGON = glGetUniformLocation(shaderProgram, "DebugOn");


  // set render state values
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
  }

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
# ifdef __Mac__
	// Can't change the version in the GLUT_3_2_CORE_PROFILE
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
# endif
# ifndef __Mac__
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
# endif
  glutInitWindowSize(1000, 600);
  //glutInitWindowPosition(600, 600);
  // set OpenGL and GLSL versions to 3.3 for Comp 465/L, comment to see highest versions
# ifndef __Mac__
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
# endif
  glutCreateWindow(baseStr);
  // initialize and verify glew
  glewExperimental = GL_TRUE;  // needed my home system 
  GLenum err = glewInit();  
  if (GLEW_OK != err) 
      printf("GLEW Error: %s \n", glewGetErrorString(err));      
    else {
      printf("Using GLEW %s \n", glewGetString(GLEW_VERSION));
      printf("OpenGL %s, GLSL %s\n", 
        glGetString(GL_VERSION),
        glGetString(GL_SHADING_LANGUAGE_VERSION));
      }
  // initialize scene
  init();
  // set glut callback functions
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(specialKeys);
  glutTimerFunc(5, update, 1);
  glutMainLoop();
  printf("done\n");
  return 0;
  }