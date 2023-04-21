#include <iostream>
#include <GL/glew.h>
#include <3dgl/3dgl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#include <stdio.h>

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// 3D models
C3dglModel camera;
C3dglModel table;
C3dglModel vase;
C3dglModel teapot;
C3dglModel cat;
C3dglModel lamp;
C3dglModel candle;
C3dglModel livingRoom;
C3dglModel ceilingLamp;

// Textures 
GLuint idTexWood;
GLuint idFabric;
GLuint idTexNone;
GLuint idTexShadowMap;
GLuint idTexSmoke;
GLuint idTexScreen;

GLuint idBufferVelocity;
GLuint idBufferStartTime;

// GLSL Program
C3dglProgram program;
C3dglProgram programParticle;

// The View Matrix
mat4 matrixView;

// Camera & navigation
float maxspeed = 4.f;	// camera max speed
float accel = 4.f;		// camera acceleration
vec3 _acc(0), _vel(0);	// camera acceleration and velocity vectors
float _fov = 60.f;		// field of view (zoom)

// vertex array 
float vertices[] = {
	-4, 0, -4, 4, 0, -4, 0, 7, 0, -4, 0, 4, 4, 0, 4, 0, 7, 0,
	-4, 0, -4, -4, 0, 4, 0, 7, 0, 4, 0, -4, 4, 0, 4, 0, 7, 0,
	-4, 0, -4, -4, 0, 4, 4, 0, -4, 4, 0, 4 
};
float normals[] = {
	0, 4, -7, 0, 4, -7, 0, 4, -7, 0, 4, 7, 0, 4, 7, 0, 4, 7,
	-7, 4, 0, -7, 4, 0, -7, 4, 0, 7, 4, 0, 7, 4, 0, 7, 4, 0,
	0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0
};
unsigned indices[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 13, 14, 15 };

// buffers names
unsigned vertexBuffer = 0;
unsigned normalBuffer = 0;
unsigned indexBuffer = 0;
unsigned idFBO = 0;

int Lpoint1 = 1;

// Particle System Params
const float PERIOD = 0.1f;
const float LIFETIME = 6;
const int NPARTICLES = (int)(LIFETIME / PERIOD);
const float M_PI = 3.141592;


bool init()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);

	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_FLAT);	// smooth shading mode is the default one;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one;
	

	// Initialise Shaders
	C3dglShader vertexShader;
	C3dglShader fragmentShader;

	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/basic.vert")) return false;
	if (!vertexShader.compile()) return false;

	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/basic.frag")) return false;
	if (!fragmentShader.compile()) return false;

	if (!program.create()) return false;
	if (!program.attach(vertexShader)) return false;
	if (!program.attach(fragmentShader)) return false;
	if (!program.link()) return false;
	if (!program.use(true)) return false;

	// Initialise Particle Shader
	C3dglShader VertexShaderParticle;
	C3dglShader FragmentShaderParticle;

	if (!VertexShaderParticle.create(GL_VERTEX_SHADER)) return false;
	if (!VertexShaderParticle.loadFromFile("shaders/basicParticle.vert")) return false;
	if (!VertexShaderParticle.compile()) return false;

	if (!FragmentShaderParticle.create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShaderParticle.loadFromFile("shaders/basicParticle.frag")) return false;
	if (!FragmentShaderParticle.compile()) return false;

	if (!programParticle.create()) return false;

	if (!programParticle.attach(VertexShaderParticle)) return false;
	if (!programParticle.attach(FragmentShaderParticle)) return false;
	if (!programParticle.link()) return false;
	if (!programParticle.use(true)) return false;

	// Setup the particle system
	programParticle.sendUniform("initialPos", vec3(20.25, -1.25, 9.4));
	programParticle.sendUniform("gravity", vec3(0.0, 0.1, 0.0));
	programParticle.sendUniform("particleLifetime", LIFETIME);

	// Reenable main shader
	program.use();

	// glut additional setup
	glutSetVertexAttribCoord3(program.getAttribLocation("aVertex"));
	glutSetVertexAttribNormal(program.getAttribLocation("aNormal"));

	
	// prepare vertex data
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// prepare normal data
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	// prepare indices array
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	// load your 3D models here!
	if (!camera.load("models\\camera.3ds")) return false;
	if (!table.load("models\\table.obj")) return false;
	if (!vase.load("models\\vase.obj")) return false;
	if (!teapot.load("models\\teapot.obj")) return false;
	if (!cat.load("models\\cat.obj")) return false;
	if (!lamp.load("models\\lamp.obj")) return false;
	if (!livingRoom.load("models\\LivingRoomObj\\livingRoom.obj")) return false;	
	livingRoom.loadMaterials("models\\LivingRoomObj");
	if (!ceilingLamp.load("models\\ceilingLamp.3ds")) return false;
	if (!candle.load("models\\candle.obj")) return false;

	
	// Send the texture info to the shaders
	glActiveTexture(GL_TEXTURE0);
	program.sendUniform("texture0", 0);
	programParticle.sendUniform("texture0", 0);

	// Textures
	C3dglBitmap bm;
	glActiveTexture(GL_TEXTURE0);
	bm.load("models/smoke.bmp", GL_RGBA);
	if (!bm.getBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexSmoke);
	glBindTexture(GL_TEXTURE_2D, idTexSmoke);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.getBits());

	//Oak texture
	bm.load("models/oak.bmp", GL_RGBA);
	if (!bm.getBits()) return false;
	glGenTextures(1, &idTexWood);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits());


	// Fabric texture
	bm.load("models/fabric.png", GL_RGBA);
	if (!bm.getBits()) return false;
	glGenTextures(1, &idFabric);
	glBindTexture(GL_TEXTURE_2D, idFabric);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits());

	// Null texture
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);

	
	// Prepare the particle buffers
	std::vector<float> bufferVelocity;
	std::vector<float> bufferStartTime;
	float time = 0;
	for (int i = 0; i < NPARTICLES; i++)
	{
		float theta = M_PI / 4.5f * (float)rand() / (float)RAND_MAX;
		float phi = M_PI * 5.0f * (float)rand() / (float)RAND_MAX;
		float x = sin(theta) * cos(phi);
		float y = cos(theta);
		float z = sin(theta) * sin(phi);
		float v = 0.1 + 0.2f * (float)rand() / (float)RAND_MAX;

		bufferVelocity.push_back(x * v);
		bufferVelocity.push_back(y * v);
		bufferVelocity.push_back(z * v);

		bufferStartTime.push_back(time);
		time += PERIOD;
	}
	glGenBuffers(1, &idBufferVelocity);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0],
		GL_STATIC_DRAW);
	glGenBuffers(1, &idBufferStartTime);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0],
		GL_STATIC_DRAW);


	// Create shadow map texture
	glActiveTexture(GL_TEXTURE7);
	glGenTextures(1, &idTexShadowMap);
	glBindTexture(GL_TEXTURE_2D, idTexShadowMap);

	// Texture parameters - to get nice filtering & avoid artefact on the edges of the shadowmap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);


	// This will associate the texture with the depth component in the Z-buffer
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int w = viewport[2], h = viewport[3];
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w * 2, h * 2, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	// Send the texture info to the shaders
	program.sendUniform("shadowMap", 7);
	// revert to texture unit 0
	glActiveTexture(GL_TEXTURE0);

	// Create a framebuffer object (FBO)
	glGenFramebuffers(1, &idFBO);
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, idFBO);
	// Instruct openGL that we won't bind a color texture with the currently binded FBO
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	// attach the texture to FBO depth attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, idTexShadowMap, 0);
	// switch back to window-system-provided framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	program.sendUniform("lightSpot.on", Lpoint1);
	program.sendUniform("lightSpot.position", vec3(0.f, 30.3f, 0.f));
	program.sendUniform("lightSpot.diffuse", vec3(0.5, 0.5, 0.5));
	program.sendUniform("lightSpot.specular", vec3(.0, .0, .0));
	program.sendUniform("lightSpot.direction", vec3(0., -1., 0.));
	program.sendUniform("lightSpot.cutoff", 35.0);
	program.sendUniform("lightSpot.auttenuation", 3.0);


	//Setup Lights
	//Switches Light On
	program.sendUniform("lightAmbient.on", 1);
	////Setup Light Colour
	program.sendUniform("lightAmbient.color", vec3(0.02, 0.02, 0.02));
	//Setup The Emissive light
	program.sendUniform("lightEmissive.on", 0);
	program.sendUniform("lightEmissive.color", vec3(1.0, 1.0, 1.0));
	program.sendUniform("materialAmbient", vec3(1.f, 1.f, 1.f));

	//Turn the Directional Light On
	program.sendUniform("lightDir.on", 1);
	program.sendUniform("lightDir.direction", vec3(1.0, 0.5, 1.0));
	program.sendUniform("lightDir.diffuse", vec3(0.4, 0.4, 0.4));

	program.sendUniform("materialSpecular", vec3(0, 0, 0));
	program.sendUniform("shininess", 3.0);

	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1), radians(12.f), vec3(1, 0, 0));
	matrixView *= lookAt(
		vec3(-20.0, 20.0, 25.0),
		vec3(0.0, 20.0, 0.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	glClearColor(0.18f, 0.25f, 0.22f, 1.0f);   // deep grey background
	

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Shift to speed up your movement" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}


void renderScene(mat4& matrixView, float time, float deltaTime)
{
	mat4 m;
	glActiveTexture(GL_TEXTURE0);

	// Pendulum mechanics
	static float alpha = 0; // angular position (swing)
	static float omega = 0.6f; // angular velocity
	deltaTime = glm::min(deltaTime, 0.2f); // remove time distortions (longer than 0.2s)
	omega -= alpha * 0.04f * deltaTime; // Hooke's law: acceleration proportional to swing
	alpha += omega * deltaTime * 50; // motion equation: swing += velocity * delta-time
	
	m = matrixView;
	m = translate(m, vec3(.0f, 50.715f, .0f));
	m= rotate(m, radians(alpha), vec3(0.5, 0, 1));
	program.sendUniform("materialDiffuse", vec3(0.9f, 0.8f, 0.4f));
	program.sendUniform("materialAmbient", vec3(0.9f, 0.8f, 0.4f));
	m = translate(m, vec3(0, -9, 0));
	mat4 m1 = m;
	m = translate(m, vec3(0, 9, 0));
	m = scale(m, vec3(.23f, .23f, .23f));
	program.sendUniform("lightSpot.on", Lpoint1);
	ceilingLamp.render(m);
	// light bulb
	m = m1;
	m = translate(m, vec3(0.0f, -14.f, 0.0f));
	m = scale(m, vec3(.7f, .7f,.7f));
	program.sendUniform("materialDiffuse", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("materialAmbient", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("materialSpecular", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("lightSpot.position", vec3(0.0f, 2.f, 0.0f));
	program.sendUniform("lightSpot.matrix", m);
	program.sendUniform("lightEmissive.on", Lpoint1);
	program.sendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);
	m = matrixView;
	program.sendUniform("lightEmissive.on", 0);

	m = matrixView;

	// room
	m = matrixView;
	glActiveTexture(GL_TEXTURE0);
	m = translate(m, vec3(.0f, 0, -30.0f));
	//m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(.15f, .15f, .15f));
	livingRoom.render(m);


	// setup materials - brown
	program.sendUniform("material", vec3(0.784f, 0.545f, 0.317f));

	// table
	m = matrixView;
	m = translate(m, vec3(.0f, 0, 0.0f));
	m = rotate(m, radians(-180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.020f, 0.020f, 0.020f));
	program.sendUniform("materialDiffuse", vec3(0.784f, 0.545f, 0.317f));
	program.sendUniform("materialAmbient", vec3(0.784f, 0.545f, 0.317f));
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	table.render(1, m);

	// setup materials - grey
	program.sendUniform("material", vec3(0.6f, 0.6f, 0.6f));
	program.sendUniform("materialDiffuse", vec3(0.6f, 0.6f, 0.6f));
	program.sendUniform("materialAmbient", vec3(0.6f, 0.6f, 0.6f));
	
	m = rotate(m, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	glBindTexture(GL_TEXTURE_2D, idFabric);
	table.render(0, m);

	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	table.render(0, m);
	
	m = rotate(m, radians(-90.f), vec3(0.0f, 1.0f, 0.0f));
	table.render(0, m);

	m = rotate(m, radians(-180.f), vec3(0.0f, 1.0f, 0.0f));
	table.render(0, m);

	// setup materials - blue
	program.sendUniform("material", vec3(0.2f, 0.2f, 0.8f));
	glBindTexture(GL_TEXTURE_2D, idTexNone);

	// vase
	m = matrixView;
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	m = translate(m, vec3(0.0f, 15.2f, .0f));
	m = rotate(m, radians(0.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.4f, 0.4f, 0.4f));
	program.sendUniform("materialDiffuse", vec3(0.2f, 0.2f, 0.8f));
	program.sendUniform("materialAmbient", vec3(0.2f, 0.2f, 0.8f));
	vase.render(m);

	//candle
	m = matrixView;
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	m = translate(m, vec3(-12.0f, 15.2f, .0f));
	m = rotate(m, radians(0.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.2f, 0.2f, 0.2f));
	program.sendUniform("materialDiffuse", vec3(0.9f, 0.8f, 0.f));
	program.sendUniform("materialAmbient", vec3(0.9f, 0.8f, 0.f));
	candle.render(m);
	
	// setup materials - green
	program.sendUniform("material", vec3(0.164f, 0.898f, 0.121f));

	//// teapot
	m = matrixView;
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	m = translate(m, vec3(6.0f, 15.f, .0f));
	m = rotate(m, radians(-90.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(2.f, 2.f, 2.f));
	program.sendUniform("materialDiffuse", vec3(0.164f, 0.898f, 0.121f));
	program.sendUniform("materialAmbient", vec3(0.164f, 0.898f, 0.121f));
	teapot.render(m);

}

// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(_fov), ratio, 0.02f, 1000.f);

	// Setup the Projection Matrix
	program.sendUniform("matrixProjection", matrixProjection);
	programParticle.sendUniform("matrixProjection", matrixProjection);
}

// Creates a shadow map and stores in idFBO
// lightTransform - lookAt transform corresponding to the light position predominant direction
void createShadowMap(mat4 lightTransform, float time, float deltaTime)
{
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	// Store the current viewport in a safe place
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int w = viewport[2], h = viewport[3];
	// setup the viewport to 2x2 the original and wide (120 degrees) FoV (Field of View)
	glViewport(0, 0, w * 2, h * 2);
	mat4 matrixProjection = perspective(radians(160.f), (float)w / (float)h, 0.5f, 50.0f);
	program.sendUniform("matrixProjection", matrixProjection);

	// prepare the camera
	mat4 matrixView = lightTransform;
	// send the View Matrix
	program.sendUniform("matrixView", matrixView);
	// Bind the Framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, idFBO);
	// OFF-SCREEN RENDERING FROM NOW!
	// Clear previous frame values - depth buffer only!
	glClear(GL_DEPTH_BUFFER_BIT);
	// Disable color rendering, we only want to write to the Z-Buffer (this is to speed-up)
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	// Prepare and send the Shadow Matrix - this is matrix transform every coordinate x,y,z
	// x = x* 0.5 + 0.5
	// y = y* 0.5 + 0.5
	// z = z* 0.5 + 0.5
	// Moving from unit cube [-1,1] to [0,1]
	const mat4 bias = {
	{ 0.5, 0.0, 0.0, 0.0 },
	{ 0.0, 0.5, 0.0, 0.0 },
	{ 0.0, 0.0, 0.5, 0.0 },
	{ 0.5, 0.5, 0.5, 1.0 }
	};
	program.sendUniform("matrixShadow", bias * matrixProjection * matrixView);
	// Render all objects in the scene
	renderScene(matrixView, time, deltaTime);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_CULL_FACE);
	onReshape(w, h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}



void onRender()
{
	// these variables control time & animation
	static float prev = 0;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;	// time since start in seconds
	float deltaTime = time - prev;						// time since last frame
	prev = time;	
	programParticle.sendUniform("time", glutGet(GLUT_ELAPSED_TIME) / 1000.f - 2);

	float timeOrigin = time;
	float deltaTimeOrigin = deltaTime;

	// Pendulum mechanics
	static float alpha = 0; // angular position (swing)
	static float omega = 0.6f; // angular velocity
	deltaTime = glm::min(deltaTime, 0.2f); // remove time distortions (longer than 0.2s)
	omega -= alpha * 0.16f * deltaTime; // Hooke's law: acceleration proportional to swing
	alpha += omega * deltaTime * 50; // motion equation: swing += velocity * delta-time

	mat4 lightSourceVestor = lookAt(
		vec3(.1f, 25.f, .0f), // coordinates of the source of the light
		vec3(0.0f, 13.0f, 0.0f), // coordinates of a point within or behind the scene
		vec3(0.0f, 1.0f, 0.0f)); // a reasonable "Up" vector
	lightSourceVestor = rotate(lightSourceVestor, radians(alpha), vec3(1, 0.5, 1));;

	createShadowMap(lightSourceVestor,
		time, deltaTime);

	glCullFace(GL_BACK);

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	_vel = clamp(_vel + _acc * deltaTime, -vec3(maxspeed), vec3(maxspeed));
	float pitch = getPitch(matrixView);
	matrixView = rotate(translate(rotate(mat4(1),
		pitch, vec3(1, 0, 0)),	// switch the pitch off
		_vel * deltaTime),		// animate camera motion (controlled by WASD keys)
		-pitch, vec3(1, 0, 0))	// switch the pitch on
		* matrixView;

	// setup View Matrix
	program.sendUniform("matrixView", matrixView);

	// revert to texture unit 0
	glActiveTexture(GL_TEXTURE0);
	// render the scene objects
	renderScene(matrixView, timeOrigin, deltaTimeOrigin);

	// setup the point size
	glEnable(GL_POINT_SPRITE);
	glPointSize(2);

	glEnable(GL_BLEND);
	// particles
	glDepthMask(GL_FALSE);				// disable depth buffer updates
	glActiveTexture(GL_TEXTURE0);			// choose the active texture
	//glColorMask(true, true, true, false);
	glAlphaFunc(GL_GREATER, 0.1f);
	glBindTexture(GL_TEXTURE_2D, idTexSmoke);	// bind the texture

	// RENDER THE PARTICLE SYSTEM
	programParticle.use();
	mat4 m;
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m = matrixView;
	m = translate(m, vec3(-30.2f, 17.2f, -9.4f));

	programParticle.sendUniform("matrixModelView", m);

	// render the buffer
	glEnableVertexAttribArray(0);	// velocity
	glEnableVertexAttribArray(1);	// start time
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, NPARTICLES);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glDepthMask(GL_TRUE);		// don't forget to switch the depth test updates back on

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': _acc.z = accel; break;
	case 's': _acc.z = -accel; break;
	case 'a': _acc.x = accel; break;
	case 'd': _acc.x = -accel; break;
	case 'e': _acc.y = accel; break;
	case 'q': _acc.y = -accel; break;
	case '1': Lpoint1 = 1 - Lpoint1; program.sendUniform("lightPoint1.on", Lpoint1); break;
	}
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': _acc.z = _vel.z = 0; break;
	case 'a':
	case 'd': _acc.x = _vel.x = 0; break;
	case 'q':
	case 'e': _acc.y = _vel.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	glutSetCursor(state == GLUT_DOWN ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
	if (button == 1)
	{
		_fov = 60.0f;
		onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
}

// handle mouse move
void onMotion(int x, int y)
{
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

	// find delta (change to) pan & pitch
	float deltaYaw = 0.005f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
	float deltaPitch = 0.005f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

	if (abs(deltaYaw) > 0.3f || abs(deltaPitch) > 0.3f)
		return;	// avoid warping side-effects

	// View = Pitch * DeltaPitch * DeltaYaw * Pitch^-1 * View;
	constexpr float maxPitch = radians(80.f);
	float pitch = getPitch(matrixView);
	float newPitch = glm::clamp(pitch + deltaPitch, -maxPitch, maxPitch);
	matrixView = rotate(rotate(rotate(mat4(1.f),
		newPitch, vec3(1.f, 0.f, 0.f)),
		deltaYaw, vec3(0.f, 1.f, 0.f)), 
		-pitch, vec3(1.f, 0.f, 0.f)) 
		* matrixView;
}

void onMouseWheel(int button, int dir, int x, int y)
{
	_fov = glm::clamp(_fov - dir * 5.f, 5.0f, 175.f);
	onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("3DGL Scene Level 2: Khrystyna Tkachuk");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		C3dglLogger::log("GLEW Error {}", (const char*)glewGetErrorString(err));
		return 0;
	}
	C3dglLogger::log("Using GLEW {}", (const char*)glewGetString(GLEW_VERSION));

	// register callbacks
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutMouseWheelFunc(onMouseWheel);

	C3dglLogger::log("Vendor: {}", (const char *)glGetString(GL_VENDOR));
	C3dglLogger::log("Renderer: {}", (const char *)glGetString(GL_RENDERER));
	C3dglLogger::log("Version: {}", (const char*)glGetString(GL_VERSION));
	C3dglLogger::log("");

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		C3dglLogger::log("Application failed to initialise\r\n");
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}

