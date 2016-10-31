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

const int X = 0, Y = 1, Z = 2, START = 0, STOP = 1,ruber = 0,unum = 1,duo = 2,primus = 3,secundus = 4, ship = 5,missile_1 = 6;
int currentWarp = 1; // Warp set to Unum
int currentCam = 1; // start in ship view
int TQ = 5;
int speed = 10;
int gravity = 0;
int fire = 0;
int accelerate = 0; //variables for movement keys
int yaw = 0;
int pitch = 0;
int roll = 0;
bool nextCam = false;
bool previousCam = false;
bool warp = false;
// constants for models:  file names, vertex count, model display size
const int nModels = 7;  // number of models in this scene
char * modelFile[nModels] = { "Ruber.tri", "Unum.tri", "Duo.tri", "Primus.tri", "Secundus.tri", "Warbird.tri", "Missile.tri" };
float modelBR[nModels];       // model's bounding radius
float scaleValue[nModels];    // model's scaling "size" value
const int nVertices[nModels] = { 264 * 3, 264 * 3, 264 * 3, 264 * 3, 264 * 3, 996 * 3, 252 * 3 };
char * vertexShaderFile   = "simpleVertex.glsl";     
char * fragmentShaderFile = "simpleFragment.glsl";    
GLuint shaderProgram; 
GLuint VAO[nModels];      // Vertex Array Objects
GLuint buffer[nModels];   // Vertex Buffer Objects

// Shader handles, matrices, etc
GLuint MVP ;  // Model View Projection matrix's handle
GLuint vPosition[nModels], vColor[nModels], vNormal[nModels];   // vPosition, vColor, vNormal handles for models
// model, view, projection matrices and values to create modelMatrix.
float modelSize[nModels] = { 2000.0f, 200.0f, 400.0f, 100.0f, 150.0f, 100.0f, 65.0f };   // size of model
float modelRadians[nModels] = { 0.0f, 0.004f, 0.002f, 0.004f, 0.002f, 0.0f, 0.0f };
glm::vec3 scale[nModels];       // set in init()
glm::vec3 translate[nModels] = { glm::vec3(0, 0, 0), glm::vec3(4000, 0, 0), glm::vec3(9000, 0, 0), glm::vec3(-900, 0, 0), glm::vec3(-1750, 0, 0), glm::vec3(5000, 1000, 5000), glm::vec3(4900, 1000, 4850) };
glm::mat4 rotation[nModels];
glm::mat4 orientation[nModels];

glm::mat4 modelMatrix;          // set in display()
glm::mat4 viewMatrix;           // set in init()
glm::mat4 projectionMatrix;     // set in reshape()
glm::mat4 ModelViewProjectionMatrix; // set in display();

glm::mat4 identity(1.0f);

//method identifyers
void warpShip();
void orientAt(int originObject, int targetObject);
glm::mat4 cameraUpdate(int cam);

// window title string
char titleStr [50]= "Warbird Simulation Phase 1 ";

void reshape(int width, int height) {
  float aspectRatio = (float) width / (float) height;
  float FOVY = glm::radians(60.0f);
  glViewport(0, 0, width, height);
  printf("reshape: FOVY = %5.2f, width = %4d height = %4d aspect = %5.2f \n", 
    FOVY, width, height, aspectRatio);
  projectionMatrix = glm::perspective(FOVY, aspectRatio, 1.0f, 100000.0f); 
  }

// Animate scene objects by updating their transformation matrices
void update(int i) {
	glutTimerFunc(TQ, update, 1);

	for (int m = 0; m < nModels; m++) {
		rotation[m] = glm::rotate(rotation[m], modelRadians[m], glm::vec3(0, 1, 0));
		if (m == ship){
			rotation[m] = glm::rotate(rotation[m], float(pitch * 0.02), glm::vec3(1, 0, 0));
			rotation[m] = glm::rotate(rotation[m], float(yaw * 0.02), glm::vec3(0, 1, 0));
			rotation[m] = glm::rotate(rotation[m], float(roll * 0.02), glm::vec3(0, 0, 1));
			orientation[m] = glm::translate(identity, translate[m]) * rotation[m] * glm::scale(identity, glm::vec3(scale[m]));
		}
		else{
			orientation[m] = rotation[m] * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(scale[m]));
		}
		if (m == primus || m == secundus){	// lunar orbits
			orientation[m] = glm::translate(identity, getPosition(orientation[duo])) * glm::rotate(rotation[m], modelRadians[m], glm::vec3(0, 1, 0)) * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(scale[m]));
		}
	}
	pitch = yaw = roll = accelerate = 0; //stop rotations if key is let go of

	if (warp == true){ //warp ship
		warp = false;
		warpShip();
	}

	viewMatrix = cameraUpdate(0); //Update dynamic cameras
	glutPostRedisplay();
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
	translate[ship] = getPosition(orientation[warpPoint]) + getIn(rotation[warpPoint]) * 8000.0f; // warps ship to duo camera position. No rotation.
	orientation[ship] = glm::translate(identity, translate[ship]) * rotation[ship] * glm::scale(identity, glm::vec3(scale[ship]));
	orientAt(ship, warpPoint);
}

void orientAt(int originObject, int targetObject){
	float radian;
	glm::vec3 originObjectAt = getIn(rotation[originObject]);
	glm::vec3 target = getPosition(orientation[targetObject]) - getPosition(orientation[originObject]);
	target = glm::normalize(target);
	glm::vec3 rotationAxis = glm::cross(target, originObjectAt);
	rotationAxis = glm::normalize(rotationAxis);
	//float rotationAxisDirection = rotationAxis.x + rotationAxis.y + rotationAxis.z;
	float rotationRads = glm::dot(target, originObjectAt);
	radian = (2 * PI) - glm::acos(rotationRads);
	rotation[originObject] = glm::rotate(rotation[originObject], radian, rotationAxis);
	orientation[originObject] = glm::translate(identity, translate[originObject]) * rotation[originObject] * glm::scale(identity, glm::vec3(scale[originObject]));
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
		return (glm::lookAt(getPosition(orientation[ship]) - getIn(rotation[ship]) * 1000.0f + getUp(rotation[ship]) * 300.0f, getPosition(orientation[ship] * glm::translate(identity, glm::vec3(0.0f, 300.0f, 0.0f))), getUp(rotation[ship])));
	}
	else if (cam == 2){ //top view
		return 
			(glm::lookAt(glm::vec3(0.0f, 20000.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	}
	else if (cam == 3){ //front view
		return (glm::lookAt(glm::vec3(0.0f, 10000.0f, 20000.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	}
	else if (cam == 4){ //Unum
		return (glm::lookAt(getPosition(orientation[unum]) + getIn(rotation[unum]) * 8000.0f, getPosition(orientation[unum]), glm::vec3(0.0f, 1.0f, 0.0f)));
	}
	else if (cam == 5){ //Duo
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
			if (gravity = 1) {
				gravity = 0;
			}
			else { //gravity = 0
				gravity = 1;
			}
			break;
	}
	if (key == 'f') { //fire
		fire = 1;
	}
	else {
		fire = 0;
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
    scale[i] = glm::vec3( modelSize[i] * 1.0f/modelBR[i]);
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
  glutCreateWindow("Warbird Simulation Phase 1");
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
  

