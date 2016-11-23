/*
warbirdSimulation.cpp

465 utility include files:  shader465.hpp, triModel465.hpp  

This program is constructed from the manyModelsStatic example the models are indexed
using arrays.

Keys: 
'v' moves to the next camera 'x' moves to the previous camera
'w' warps the ship to duo

Steven Blachowiak, Aaron Scott
9/29/2016
*/

// define your target operating system: __Windows__  __Linux__  __Mac__
# define __Windows__ 
# include "../includes465/include465.hpp"

const int X = 0, Y = 1, Z = 2, START = 0, STOP = 1, ruber = 0, unum = 1, duo = 2, primus = 3, secundus = 4, ship = 5, missile_1 = 6, missile_2 = 7, missileBase_1 = 8, missileBase_2 = 9, missile_3 = 10, gravityMax = 90000000;
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
// constants for models:  file names, vertex count, model display size
const int nModels = 11;  // number of models in this scene
char * modelFile[nModels] = { "Ruber.tri", "Unum.tri", "Duo.tri", "Primus.tri", "Secundus.tri", "Warbird.tri", "Missile.tri", "Missile.tri", "Missilebase.tri", "Missilebase.tri", "Missile.tri" };
float modelBR[nModels];       // model's bounding radius
float scaleValue[nModels];    // model's scaling "size" value
float gravityForce;
const int nVertices[nModels] = { 264 * 3, 264 * 3, 264 * 3, 264 * 3, 264 * 3, 996 * 3, 252 * 3, 252 * 3, 12 * 3, 12 * 3, 252 * 3 };
char * vertexShaderFile   = "simpleVertex.glsl";     
char * fragmentShaderFile = "simpleFragment.glsl";    
GLuint shaderProgram; 
GLuint VAO[nModels];      // Vertex Array Objects
GLuint buffer[nModels];   // Vertex Buffer Objects

// Shader handles, matrices, etc
GLuint MVP ;  // Model View Projection matrix's handle
GLuint vPosition[nModels], vColor[nModels], vNormal[nModels];   // vPosition, vColor, vNormal handles for models
// model, view, projection matrices and values to create modelMatrix.
float modelSize[nModels] = { 2000.0f, 200.0f, 400.0f, 100.0f, 150.0f, 100.0f, 500.0f, 25.0f, 30.0f, 30.0f, 25.0f };   // size of model
float modelRadians[nModels] = { 0.0f, 0.004f, 0.002f, 0.004f, 0.002f, 0.0f, 0.0f, 0.0f, 0.004f, 0.002f, 0.0f };
glm::vec3 scale[nModels];       // set in init()
glm::vec3 translate[nModels] = { glm::vec3(0, 0, 0), glm::vec3(4000, 0, 0), glm::vec3(9000, 0, 0), glm::vec3(-900, 0, 0), glm::vec3(-1750, 0, 0), glm::vec3(5000, 1000, 5000), glm::vec3(4900, 1000, 4850), glm::vec3(4900, 1050, 4850), glm::vec3(4000, 225, 0), glm::vec3(-1750, 175, 0), glm::vec3(0, 0, 0) };
glm::mat4 rotation[nModels];
glm::mat4 orientation[nModels];

glm::mat4 modelMatrix;          // set in display()
glm::mat4 viewMatrix;           // set in init()
glm::mat4 projectionMatrix;     // set in reshape()
glm::mat4 ModelViewProjectionMatrix; // set in display();

// window title strings
char fpsStr[15];
char missileStr[50];
char viewStr[50] = "Camera: Ship View | ";
char baseStr[50] = "Warbird Simulation | ";
char titleStr[500];
int playerMissileCount = 9; // Player Missile Count
int base1MissileCount = 5;
int base2MissileCount = 5;
int frameCount = 0;
int missileTimerCount = 0;
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
		}
		else{ // orbits
			rotation[m] = glm::rotate(rotation[m], modelRadians[m], glm::vec3(0, 1, 0));
			orientation[m] = rotation[m] * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(scale[m]));
			if (m == primus || m == secundus || m == missileBase_2){	// lunar orbits
				orientation[m] = glm::translate(identity, getPosition(orientation[duo])) * glm::rotate(rotation[m], modelRadians[m], glm::vec3(0, 1, 0)) * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(scale[m]));
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
				translate[m] = translate[m] + getIn(rotation[m]) * 20.0f; // move missile forward
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
		else if (m == missile_2){ // unum base missle updates
			if (distance(getPosition(orientation[missileBase_1]),getPosition(orientation[ship])) > 5000){ // proximity check
				unumBaseFire = true;
			}
			
			if (unumBaseFire == false){
				translate[m] = getPosition(orientation[missileBase_1]);
				rotation[m] = rotation[missileBase_1];
				orientation[m] = glm::translate(identity, translate[m]) * rotation[m] * glm::scale(identity, glm::vec3(0, 0, 0));
			}
			if (unumBaseFire == true){
				translate[m] = translate[m] + getIn(rotation[m]) * 5.0f; // move missile forward
				missileTracking(missile_2);
				orientation[m] = glm::translate(identity, translate[m]) * rotation[m] * glm::scale(identity, scale[m]);
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
		printf("YOU DIED ");
	}
	else if (glm::distance(getPosition(orientation[ship]), getPosition(orientation[secundus])) < (modelSize[secundus] + modelSize[ship])) {
		playerCollision = true;
		printf("YOU DIED ");
	}
	else if (glm::distance(getPosition(orientation[ship]), getPosition(orientation[missile_2])) < (modelSize[missile_2] + modelSize[ship])) {
		playerCollision = true;
		printf("missile_2 HIT");
	}
	if (glm::distance(getPosition(orientation[missile_1]), getPosition(orientation[missileBase_1])) < (modelSize[missileBase_1] + modelSize[missile_1] + 50)) {
		missileBase1Collision = true;
		fire = false;
		printf("Base1 HIT ");
	}
	if (glm::distance(getPosition(orientation[missile_1]), getPosition(orientation[missileBase_2])) < (modelSize[missileBase_2] + modelSize[missile_1] + 50)) {
		missileBase1Collision = true;
		fire = false;
		printf("Base2 HIT ");
	}

	pitch = yaw = roll = accelerate = 0; //stop rotations if key is let go of

	viewMatrix = cameraUpdate(0); //Update dynamic cameras
	sprintf(missileStr, "Missiles: %d | ", playerMissileCount);
	glutPostRedisplay();
}

void missileTracking(int missile){
	int target;

	if (missile == missile_1){ // missile is fired from the ship, pick closest base
		if (distance(getPosition(orientation[missile]), getPosition(orientation[missileBase_1])) < distance(getPosition(orientation[missile]), getPosition(orientation[missileBase_2]))){
			target = missileBase_1; // target unum base
		}
		else{ 
			target = missileBase_2; // target moon base
		}
	}
	else { // missle is fired from a base, its target is the ship
		target = ship;
	}

	//if (distance(getPosition(orientation[missile]), getPosition(orientation[target])) > 5000){ // if no target is found within 5000 units set target to -1
	//	target = -1;
	//}

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
	if (colinear(originObjectAt, normTarget, .1)){ // check for colinearity
		if (distance(getPosition(orientation[originObject]) + originObjectAt, target) > distance(getPosition(orientation[originObject]), target)){ // check for bad colinear situations
			rotation[originObject] = glm::rotate(rotation[originObject], PI, getUp(rotation[originObject])); // turn missle around
			printf("BAD_COLINEAR");
		}
		return true;
	}
	else{
		if (distance(normTarget,originObjectAt) < 1){
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
		sprintf(viewStr,"Camera: Ship View | ");
		return (glm::lookAt(getPosition(orientation[ship]) - getIn(rotation[ship]) * 1000.0f + getUp(rotation[ship]) * 300.0f, getPosition(orientation[ship] * glm::translate(identity, glm::vec3(0.0f, 300.0f, 0.0f))), getUp(rotation[ship])));
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
			warp = true;
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
			if (fire == false && canFire){
				fire = true;
				playerMissileCount--;
				if (playerMissileCount == 0){
					canFire = false;
				}
				//missileTimerCount = frameCount;
			}
			break;
		case '1': //reset missle **Debug**
			fire = false;
			break;
	}
}

void specialKeys(int key, int x, int y) {
	int modifier = glutGetModifiers(); //checks for control key
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


void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // update model matrix
  glClearColor(0,0,0,0);
	for (int m = 0; m < nModels; m++) {
		modelMatrix = orientation[m];
		ModelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
		glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));
		glBindVertexArray(VAO[m]);
		glDrawArrays(GL_TRIANGLES, 0, nVertices[m]);
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
  shaderProgram = loadShaders(vertexShaderFile,fragmentShaderFile);
  glUseProgram(shaderProgram);
  
  // generate VAOs and VBOs
  glGenVertexArrays( nModels, VAO );
  glGenBuffers( nModels, buffer );
  // load the buffers from the model files
  for (int i = 0; i < nModels; i++) {
    modelBR[i] = loadModelBuffer(modelFile[i], nVertices[i], VAO[i], buffer[i], shaderProgram,
      vPosition[i], vColor[i], vNormal[i], "vPosition", "vColor", "vNormal"); 
    // set scale for models given bounding radius  
    scale[i] = glm::vec3(modelSize[i] * 1.0f / modelBR[i]);
	
    }
  
  MVP = glGetUniformLocation(shaderProgram, "ModelViewProjection");

  viewMatrix = glm::lookAt(
	glm::vec3(0.0f, 20000.0f, 0.0f),  // eye position
    glm::vec3(0),                   // look at position
    glm::vec3(0.0f, 0.0f, -1.0f)); // up vect0r

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
  

