#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include "Dependencies\glm\gtc\type_ptr.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <math.h>
#include <time.h>
#define PI 3.1415926
using namespace std;
using glm::vec3;
using glm::mat4;

GLint programID;
const float viewMotionSpeed = 0.001;
const int numRocks = 222;
float yaw = 0;
int winWidth = 1024;
int winHeight = 720;
bool mouseCapture = true;
float wonderStarRotateAngle = 0.0f;
float earthRotateAngle = 0.0f;
float ringRotateAngle = 0.0f;
glm::vec3 craftPos(0.0f, 0.0f, 0.0f);
glm::vec3 wonderStarPos(10.0f, 0.0f, 0.0f);
glm::vec3 earthPos(50.0f, 0.0f, 0.0f);
glm::vec3 ringPos1(20.0f, 0.0f, 0.0f);
glm::vec3 ringPos2(30.0f, 0.0f, 0.0f);
glm::vec3 ringPos3(40.0f, 0.0f, 0.0f);
glm::vec3 rockPos[numRocks];
bool rockVisible[numRocks];
bool wonderStarVisible = true;
bool earthVisible = true;
float rockOrientation[numRocks][2];
float ringDelta = 0.7f;
float rockDelta = 0.7f;
float planetDelta = 1.2f;
float move_pace = 0.1f;
GLuint earthNormal;


void setMat4(const std::string &name, const glm::mat4& value)
{
	unsigned int transformLoc = glGetUniformLocation(programID, name.c_str());
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(value));
}
void setVec4(const std::string &name, const glm::vec4& value)
{
	glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setVec3(const std::string &name, const glm::vec3& value)
{
	glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setFloat(const std::string &name, float value)
{
	glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}
void setInt(const std::string &name, int value)
{
	glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}
bool loadOBJ(const char * path, std::vector<glm::vec3> & out_vertices, std::vector<glm::vec2> & out_uvs, std::vector<glm::vec3> & out_normals);
GLuint loadTexture(string imagepath);
GLuint loadSkyBox();
class Model
{
protected:
	string objFile;
	string textureFile;
	vector<glm::vec3> vertices;
	vector<glm::vec2> uvs;
	vector<glm::vec3> normals;
	GLuint VAO;
	GLuint VBO;
	GLuint UVB;
	GLuint NBO;
public:
	GLuint textureID;
	GLfloat scale;
	glm::mat4 modelMat; //在scene 中的位置
	glm::mat4 viewMat; //view matrix
	glm::mat4 transformMat; //在obj的坐标下的变换
	Model(string objFile, string textureFile, const glm::mat4& modelMat, GLfloat scale)
	{
		this->modelMat = modelMat;
		this->scale = scale;
		this->objFile = objFile;
		this->textureFile = textureFile;
	}

	void init() 
	{
		if (!loadOBJ(objFile.c_str(), vertices, uvs, normals))
			exit(1);
		if (textureFile == "skybox")
			textureID = loadSkyBox();
		else
			textureID = loadTexture(textureFile);
		if (!textureID)
			exit(1);

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &UVB);
		glBindBuffer(GL_ARRAY_BUFFER, UVB);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
		glGenBuffers(1, &NBO);
		glBindBuffer(GL_ARRAY_BUFFER, NBO);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
	}
	void render()
	{
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, UVB);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, NBO);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		setMat4("transform", modelMat * transformMat);
		setMat4("view", viewMat);
		setFloat("scale", scale);
		glActiveTexture(GL_TEXTURE0);
		if (textureFile == "skybox")
		{
			setInt("skybox", 1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
			setInt("skyboxTexture", 0);
		}
		else
		{
			setInt("skybox", 0);
			glBindTexture(GL_TEXTURE_2D, textureID);
			setInt("textureSampler", 0);
		}
		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());
	}
};

Model skyboxModel("skybox.obj", "skybox", glm::mat4(), 1.0f);
Model spaceCraft("spaceCraft.obj", "spaceCraftTexture.bmp", glm::mat4(), 0.001f);
Model wonderStar("planet.obj", "WonderStarTexture.bmp", glm::mat4(), 0.3f);
Model earth("planet.obj", "earthTexture.bmp", glm::mat4(), 0.3f);
Model ring1("Ring.obj", "ringTexture.bmp", glm::mat4(), 0.025f);
Model ring2("Ring.obj", "ringTexture.bmp", glm::mat4(), 0.025f);
Model ring3("Ring.obj", "ringTexture.bmp", glm::mat4(), 0.025f);
Model rock("rock.obj", "RockTexture.bmp", glm::mat4(), 0.03f);

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(programID);
}

float clamp(float in, float l, float h)
{
	if (in < l)
		return l;
	if (in > h)
		return h;
}

void timer(int value)
{
	wonderStarRotateAngle += 0.0025f;
	if (wonderStarRotateAngle > 2 * PI)
		wonderStarRotateAngle -= 2 * PI;
	for (int i = 0; i < numRocks; i++)
	{
		glm::vec4 rockPos4 = glm::rotate(glm::mat4(), 0.0025f, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(rockPos[i], 1.0f);
		rockPos[i] = glm::vec3(rockPos4);
	}
	earthRotateAngle += 0.0015f;
	if (earthRotateAngle > 2 * PI)
		earthRotateAngle -= 2 * PI;
	ringRotateAngle += 0.001f;
	if (ringRotateAngle > 2 * PI)
		ringRotateAngle -= 2 * PI;
	for (int i = 0; i < numRocks; i++)
	{
		if (rockVisible[i] && abs(rockPos[i][1]) < 0.3f)
		{
			if (glm::distance((rockPos[i] + wonderStarPos), craftPos) < rockDelta) 
			{	
				rockVisible[i] = false;
				printf("Delete rock: %d\n", i);
			}
		}
	}
	if (glm::distance(wonderStarPos, craftPos) < planetDelta)
		wonderStarVisible = false;
	if (glm::distance(earthPos, craftPos) < planetDelta)
		earthVisible = false;
	glutTimerFunc(15, timer, 1);
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == ' ')
	{
		if (mouseCapture)
		{
			glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
			mouseCapture = false;
		}
		else
		{
			glutSetCursor(GLUT_CURSOR_NONE);
			glutWarpPointer(winWidth / 2, winHeight / 2);
			mouseCapture = true;
		}
	}
	if (key == 27)
		glutLeaveMainLoop();
}

void move(int key, int x, int y) 
{
	if (key == GLUT_KEY_UP)
	{
		craftPos[0] += move_pace * cos(yaw);
		craftPos[2] -= move_pace * sin(yaw);
	}
	else if (key == GLUT_KEY_DOWN)
	{
		craftPos[0] -= move_pace * cos(yaw);
		craftPos[2] += move_pace * sin(yaw);
	}
	else if (key == GLUT_KEY_LEFT)
	{
		craftPos[0] -= move_pace * sin(yaw);
		craftPos[2] -= move_pace * cos(yaw);
	}
	else if (key == GLUT_KEY_RIGHT)
	{
		craftPos[0] += move_pace * sin(yaw);
		craftPos[2] += move_pace * cos(yaw);
	}
}

void PassiveMouse(int x, int y)
{
	if (mouseCapture)
	{
		int deltaX = winWidth / 2 - x;
		int deltaY = winHeight / 2 - y;
		yaw += deltaX * viewMotionSpeed;
		glutWarpPointer(winWidth / 2, winHeight / 2);
	}
}

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file;
	errno_t err = fopen_s(&file, path, "r");
	if (err) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 6 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf_s(file, "%s", lineHeader, 128);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
	//		uv.y = -uv.y; 
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			normal = glm::normalize(normal);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i<vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}

	return true;
}

unsigned char* loadBMP(const char * imagepath, unsigned int& width, unsigned int&height) {

	printf("Reading image %s\n", imagepath);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned char * data;

	FILE * file;
	errno_t err = fopen_s(&file, imagepath, "rb");
	if (err) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	if (imageSize == 0)    imageSize = width*height * 3; 
	if (dataPos == 0)      dataPos = 54; 

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);
	return data;
}

GLuint loadTexture(string imagePath)
{
	unsigned int width, height;
	unsigned char* data = loadBMP(imagePath.c_str(), width, height);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	delete[] data;
	return textureID;

}

GLuint loadSkyBox()
{
	cout << "loading skybox..." << endl;
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	unsigned int width, height;
	unsigned char* data;
	data = loadBMP("skybox/ft.bmp", width, height);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;
	data = loadBMP("skybox/bk.bmp", width, height);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;
	data = loadBMP("skybox/rt.bmp", width, height);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;
	data = loadBMP("skybox/lf.bmp", width, height);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;
	data = loadBMP("skybox/up.bmp", width, height);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;
	data = loadBMP("skybox/dn.bmp", width, height);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return textureID;
}

void windowReshape(int width, int height)
{
	winWidth = width;
	winHeight = height;
}


void sendDataToOpenGL()
{
	skyboxModel.init();
	spaceCraft.init();
	wonderStar.init();
	earth.init();
	ring1.init();
	ring2.init();
	ring3.init();
	rock.init();
	earthNormal = loadTexture("earth_normal.bmp");
}

void paintGL(void)
{
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glm::vec3 lookingDir = glm::vec3(cos(yaw), 0, -sin(yaw));
	glm::vec3 eyePos = craftPos - lookingDir;
	eyePos[1] = 0.65f;
	glm::mat4 viewMat = glm::lookAt(eyePos, eyePos+lookingDir, glm::vec3(0.0f, 1.0f, 0.0f));
	viewMat = glm::perspective(glm::radians(80.0f), winWidth/float(winHeight), 0.01f, 100.0f) * viewMat;

	setVec3("eyePos", eyePos);
	setVec3("lightPos1", glm::vec3(5.0f, 20.0f, 0.0f));
	setVec3("lightPos2", glm::vec3(35.0f, 20.0f, 0.0f));
	setVec3("ambientLight", glm::vec3(1.0f, 1.0f, 1.0f));
	setFloat("specularFactor", 0.0f);
	setFloat("diffuseFactor", 0.0f);
	skyboxModel.viewMat = viewMat;
	skyboxModel.modelMat = glm::translate(glm::mat4(), eyePos);
	glDepthMask(GL_FALSE);
	skyboxModel.render();
	glDepthMask(GL_TRUE);
	
	setVec3("ambientLight", glm::vec3(0.1f, 0.1f, 0.1f));
	setFloat("specularFactor", 1.0f);
	setFloat("diffuseFactor", 0.5f);

	//render spacecraft
	spaceCraft.viewMat = viewMat;
	spaceCraft.transformMat = glm::rotate(glm::mat4(), yaw + glm::half_pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
	spaceCraft.modelMat = glm::translate(glm::mat4(), craftPos);
	if (glm::distance(craftPos, ringPos1) < ringDelta || glm::distance(craftPos, ringPos2) < ringDelta || glm::distance(craftPos, ringPos3) < ringDelta)
		setInt("green", 1);
	spaceCraft.render();
	setInt("green", 0);
	//render wonderstar
	if (wonderStarVisible)
	{
		wonderStar.viewMat = viewMat;
		wonderStar.transformMat = glm::rotate(glm::mat4(), wonderStarRotateAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		wonderStar.modelMat = glm::translate(glm::mat4(), wonderStarPos);
		wonderStar.render();
	}
	//render earth
	if (earthVisible)
	{
		setInt("normalMapping", 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, earthNormal);
		setInt("normalSampler", 1);
		earth.viewMat = viewMat;
		earth.transformMat = glm::rotate(glm::mat4(), earthRotateAngle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(), -(float)PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
		earth.modelMat = glm::translate(glm::mat4(), earthPos);
		earth.render();
		setInt("normalMapping", 0);
	}
	//render ring1
	ring1.viewMat = viewMat;
	ring1.transformMat = glm::rotate(glm::mat4(), ringRotateAngle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(), (float)PI/2, glm::vec3(0.0f, 0.0f, 1.0f));
	ring1.modelMat = glm::translate(glm::mat4(), ringPos1);
	if (glm::distance(craftPos, ringPos1) < ringDelta)
		setInt("green", 1);
	ring1.render();
	setInt("green", 0);
	//render ring2
	ring2.viewMat = viewMat;
	ring2.transformMat = glm::rotate(glm::mat4(), ringRotateAngle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(), (float)PI / 2, glm::vec3(0.0f, 0.0f, 1.0f));
	ring2.modelMat = glm::translate(glm::mat4(), ringPos2);
	if (glm::distance(craftPos, ringPos2) < ringDelta)
		setInt("green", 1);
	ring2.render();
	setInt("green", 0);
	//render ring3
	ring3.viewMat = viewMat;
	ring3.transformMat = glm::rotate(glm::mat4(), ringRotateAngle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(), (float)PI / 2, glm::vec3(0.0f, 0.0f, 1.0f));
	ring3.modelMat = glm::translate(glm::mat4(), ringPos3);
	if (glm::distance(craftPos, ringPos3) < ringDelta)
		setInt("green", 1);
	ring3.render();
	setInt("green", 0);
	//render 222 rocks
	rock.viewMat = viewMat;
	
	for (int i = 0; i < numRocks; i++) {
		if (rockVisible[i]) {
			rock.transformMat = glm::rotate(glm::mat4(), rockOrientation[i][0], glm::vec3(1.0f, 0.0f, 0.0f));
			rock.transformMat = glm::rotate(glm::mat4(), rockOrientation[i][1], glm::vec3(0.0f, 0.0f, 1.0f)) * rock.transformMat;
			rock.transformMat = glm::translate(glm::mat4(), rockPos[i]) * rock.transformMat;
			rock.modelMat = glm::translate(glm::mat4(), wonderStarPos);
			rock.render();
		}
	}
	glFlush();
	glutPostRedisplay();
}

void initializedGL(void) //run only once
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	installShaders();
	sendDataToOpenGL();
}

int main(int argc, char *argv[])
{
	/*
	glm::vec3 normal(1.0f, 1.0f, 1.0f);
	normal = glm::normalize(normal);
	glm::vec3 x = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), normal));
	glm::vec3 y = glm::normalize(glm::cross(normal, x));
	glm::mat3 rotation(x[0], x[1], x[2], y[0], y[1], y[2], normal[0], normal[1], normal[2]);
	glm::vec3 result1 = rotation * glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 result2 = glm::transpose(rotation) * glm::vec3(0.0f, 0.0f, 1.0f);
	cout << result1[0] << " " << result1[1] << " " << result1[2] << endl;
	cout << result2[0] << " " << result2[1] << " " << result2[2] << endl;
	cout << glm::dot(result1, normal) << endl;
*/
	srand(time(NULL));
	for (int i = 0; i < numRocks; i++) {	//randomly create 222 rocks
		float r = 1.0 + (rand() % 1000) * 0.001; //from [1, 2]
		r *= r;
		float theta = (rand() % 1000) * 0.002 * PI;
		float y = (rand() % 1000) * 0.001;	//from [0, 1]
		y *= y;
		y *= 1 - 2 * rand() % 2;
		rockPos[i][0] = r * cos(theta);
		rockPos[i][1] = y;
		rockPos[i][2] = r * sin(theta);
		rockVisible[i] = true;		//flag for rander or not
		rockOrientation[i][0] = (rand() % 1000) * 0.001 * PI;
		rockOrientation[i][1] = (rand() % 1000) * 0.001 * PI;
	}

	glutInit(&argc, argv);
	glutInitWindowSize(winWidth, winHeight);
	glutReshapeFunc(windowReshape);
	glutPassiveMotionFunc(PassiveMouse);
	glutCreateWindow("Final Project");
	glutSetCursor(GLUT_CURSOR_NONE);
	glutWarpPointer(winWidth / 2, winHeight / 2);
	initializedGL();
	glutDisplayFunc(paintGL);
	
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(move);
	glutPassiveMotionFunc(PassiveMouse);
	glutTimerFunc(15, timer, 1);
	
	glutMainLoop();

	return 0;
}