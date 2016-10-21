/*
warbirdSimulation.cpp

465 utility include files:  shader465.hpp, triModel465.hpp  

This program is constructed from the manyModelsStatic example the models are indexed
using arrays.

Steven Blachowiak, Aaron Scott
9/29/2016
*/

// define your target operating system: __Windows__  __Linux__  __Mac__
# define __Windows__ 
# include "../includes465/include465.hpp"

const int X = 0, Y = 1, Z = 2, START = 0, STOP = 1;
int dynamicCam = 0;
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
	glutTimerFunc(5, update, 1);
	for (int m = 0; m < nModels; m++) {
		rotation[m] = glm::rotate(rotation[m], modelRadians[m], glm::vec3(0, 1, 0));
		orientation[m] = rotation[m] * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(scale[m]));
		if (m == 3 || m == 4){	// lunar orbits
			orientation[m] = glm::translate(identity, getPosition(orientation[2])) * glm::rotate(rotation[m], modelRadians[m], glm::vec3(0, 1, 0)) * glm::translate(identity, translate[m]) * glm::scale(identity, glm::vec3(scale[m]));
		}
	}
	//Update dynamic cameras
	if (dynamicCam == 1){ // ship
		viewMatrix = glm::lookAt(getPosition(orientation[5]) - getIn(rotation[5]) * 1000.0f + getUp(rotation[5]) * 300.0f, getPosition(orientation[5] * glm::translate(identity, glm::vec3(0.0f, 300.0f, 0.0f))), glm::vec3(0.0f, 1.0f, 0.0f));
	} else if (dynamicCam == 2){ //Unum
		viewMatrix = glm::lookAt(getPosition(orientation[1]) + getIn(rotation[1]) * 8000.0f, getPosition(orientation[1]), glm::vec3(0.0f, 1.0f, 0.0f));
	} else if (dynamicCam == 3){ //Duo
		viewMatrix = glm::lookAt(getPosition(orientation[2]) + getIn(rotation[2]) * 8000.0f, getPosition(orientation[2]), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 033: case 'q':  case 'Q': exit(EXIT_SUCCESS); break;
		case '1':  // top view
			dynamicCam = 0;
			viewMatrix = glm::lookAt(
				glm::vec3(0.0f, 20000.0f, 0.0f),		// pos
				glm::vec3(0.0f, 0.0f, 0.0f),			// look at 
				glm::vec3(0.0f, 0.0f, -1.0f));			// up
			//glutPostRedisplay(); 
			break;
		case '2':  // front view
			dynamicCam = 0;
			viewMatrix = glm::lookAt(
				glm::vec3(0.0f, 10000.0f, 20000.0f),   // pos
				glm::vec3(0.0f, 0.0f, 0.0f),			// look at 
				glm::vec3(0.0f, 1.0f, 0.0f));			// up
			//glutPostRedisplay(); 
			break;
		case '3':  // ship view
			dynamicCam = 1;
			break;
		case '4':  // Unum view
			dynamicCam = 2; 
			break;
		case '5':  // Duo view
			dynamicCam = 3;
			break;
		}
	}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // update model matrix
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
  glutTimerFunc(5, update, 1);
  glutMainLoop();
  printf("done\n");
  return 0;
  }
  

