#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "shader_s.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include"camera.h"
#include"string.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>  
#include <sstream>
#include <string>
#include<vector>
#include <time.h>
using namespace std;


//**********************************************************
const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
//"layout (location = 2) in vec2 aTexCoord;\n"
//"layout (location = 1) in vec3 aColor;\n"
"layout (location = 1) in vec3 aNormal;\n"
//"out vec2 TexCoord;\n"
//"out vec3 ourColor;\n"

"out vec3 Normal;\n"
"out vec3 FragPos;\n"

"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position =projection * view * model *vec4(aPos, 1.0);\n"
"FragPos = vec3(model * vec4(aPos, 1.0));\n"
//"TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
"Normal = mat3(transpose(inverse(model))) * aNormal;\n"


//"   ourColor = aColor;\n"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
//"in vec3 ourColor;\n"
//"in vec2 TexCoord;\n"
//"uniform sampler2D texture1;\n"
//"uniform sampler2D texture2;\n"
"struct Material {\n"
"	vec3 ambient;\n"
"	vec3 diffuse;\n"
"	vec3 specular;\n"
"	float shininess;\n"
"};\n"
"struct Light  {\n"
"	vec3 position;\n"
"	vec3 ambient;\n"
"	vec3 diffuse;\n"
"	vec3 specular;\n"
"};\n"

"in vec3 Normal;  \n"
"in vec3 FragPos; \n"
"uniform vec3 viewPos; \n"
"uniform Material material;\n"
"uniform Light light;\n"

"void main()\n"
"{\n"
// ambient
"vec3 ambient = light.ambient * material.ambient;\n"
// diffuse 
"    vec3 norm = normalize(Normal);\n"
"vec3 lightDir = normalize(light.position - FragPos);\n"
"float diff = max(dot(norm, lightDir), 0.0);\n"
"vec3 diffuse = light.diffuse * (diff * material.diffuse);\n"
// specular
"vec3 viewDir = normalize(viewPos - FragPos);\n"
"vec3 reflectDir = reflect(-lightDir, norm);\n"
"float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
"vec3 specular = light.specular * (spec * material.specular);\n"
"vec3 result = ambient + diffuse + specular;\n"
"  FragColor = vec4(result, 1.0f);\n"//*mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);\n"
"}\n\0";
const char *lampvs = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"  gl_Position =projection * view * model *vec4(aPos, 1.0);\n"
"}\n\0";

const char *lampfs = "#version 330 core\n"
"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"  FragColor = vec4(1.0); \n"
"}\n\0";
const char *texturevs = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoord;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"  gl_Position =projection * view * model *vec4(aPos, 1.0);\n"
"TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
"}\n\0";

const char *texturefs = "#version 330 core\n"
"in vec2 TexCoord;\n"
"out vec4 FragColor;\n"
"uniform sampler2D texture1;\n"
"void main()\n"
"{\n"
"  FragColor = texture(texture1, TexCoord);\n"
"}\n\0";

const char *shadow_mapping_depthvs = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 lightSpaceMatrix;\n"
"uniform mat4 model;\n"
"void main()\n"
"{\n"
"  gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);\n"
"}\n\0";

const char *shadow_mapping_depthfs="#version 330 core\n"
"void main()\n"
"{\n"
//"  // FragColor = texture(texture1, TexCoord);\n"
"}\n\0";

const char *debug_quadvs = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoords;\n"
"out vec2 TexCoords;\n"
"void main()\n"
"{\n"
"TexCoords = aTexCoords;\n"
"  gl_Position = vec4(aPos, 1.0);\n"
"}\n\0";

const char *debug_quad_depthfs = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 TexCoords;\n"
"uniform sampler2D depthMap;\n"
"uniform float near_plane;\n"
"uniform float far_plane;\n"
"float LinearizeDepth(float depth)\n"
"{\n"
"  float z = depth * 2.0 - 1.0;\n"
"return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));\n"
"}\n"
"void main()\n"
"{\n"
"  float depthValue = texture(depthMap, TexCoords).r;\n"
"FragColor = vec4(vec3(depthValue), 1.0);\n"
"}\n\0";


const char *shadow_mappingvs = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aTexCoords;\n"
"out vec2 TexCoords;\n"
"out VS_OUT {\n"
"vec3 FragPos;\n"
"vec3 Normal;\n"
"vec2 TexCoords;\n"
"vec4 FragPosLightSpace;\n"
"} vs_out;\n"
"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"
"uniform mat4 lightSpaceMatrix;\n"
"void main()\n"
"{\n"
"vs_out.FragPos = vec3(model * vec4(aPos, 1.0));\n"
"  vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;\n"
"vs_out.TexCoords = aTexCoords;\n"
"vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);\n"
"gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"}\n\0";

const char *shadow_mappingfs = "#version 330 core\n"
"out vec4 FragColor;\n"
"in VS_OUT {\n"
"vec3 FragPos;\n"
"vec3 Normal;\n"
"vec2 TexCoords;\n"
"vec4 FragPosLightSpace;\n"
"} fs_in;\n"

"uniform sampler2D diffuseTexture;\n"
"uniform sampler2D shadowMap;\n"

"uniform vec3 lightPos;\n"
"uniform vec3 viewPos;\n"

"float ShadowCalculation(vec4 fragPosLightSpace)\n"
"{\n"
"vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;\n"
"projCoords = projCoords * 0.5 + 0.5;\n"
"float closestDepth = texture(shadowMap, projCoords.xy).r; \n"
"float currentDepth = projCoords.z;\n"
"float bias = 0.005;\n"
"float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;\n"
"return shadow;\n"
"}\n"
"void main()\n"
"{\n"
"vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;\n"
"   vec3 normal = normalize(fs_in.Normal);\n"
"vec3 lightColor = vec3(0.3);\n"
"vec3 ambient = 0.3 * color;\n"
"vec3 lightDir = normalize(lightPos - fs_in.FragPos);\n"
"float diff = max(dot(lightDir, normal), 0.0);\n"
"vec3 diffuse = diff * lightColor;\n"
"vec3 viewDir = normalize(viewPos - fs_in.FragPos);\n"
"vec3 reflectDir = reflect(-lightDir, normal);\n"
"float spec = 0.0;\n"
"vec3 halfwayDir = normalize(lightDir + viewDir); \n"
"spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);\n"
"vec3 specular = spec * lightColor;  \n"
"float shadow = ShadowCalculation(fs_in.FragPosLightSpace);\n"
"vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;\n"
"FragColor = vec4(lighting, 1.0);\n"
"}\n\0";


//***************************************************
bool up_press = false, down_press = false, right_press = false, left_press = false,cameraTran=false;
void processSpecialKeys(int key, int x, int y);
void processSpeciaUplKeys(int key, int x, int y);
glm::vec3 lightPos(10.0f, 40.0f, 10.0f);
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

glm::vec3 carPos(-10.0f, 0.0f, 10.0f);
glm::vec3 carFront(0.0f, 0.0f, -1.0f);

void createGLUTMenus();
void processMenuEvents(int option);
void processKeys(unsigned char key, int x, int y);
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
GLuint planeVAO, planeVAO2, planeVAO3, planeVAO4, planeVAO5;
const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
GLuint depthMap, depthMap2, depthMap3, depthMap4, depthMap5;

GLuint depthMapFBO, depthMapFBO2, depthMapFBO3, depthMapFBO4, depthMapFBO5;
float carangle = 0.0f;

unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;
vector<float> vertices;
vector<float> vertices2;
vector<float> norm;
vector<unsigned int> indices;


void car() {
	string s, str, s1, s2, s3, s4;
	ifstream inf;
	inf.open("D:\\vscode\\GlfwGlad\\car.obj");


	//int vn = 0;
	int vnum = 0;
	int fnum = 0;
	//int ffnum = 0;
	while (getline(inf, s)) {
		istringstream in(s);
		in >> s1;
		if (s1 == "v") {
			in >> s2 >> s3 >> s4;
			vertices2.push_back(stof(s2) + 1);
			vnum += 1;
			vertices2.push_back(stof(s3) + 1);
			vnum += 1;
			vertices2.push_back(stof(s4) + 1);
			vnum += 1;
			/*vertices.push_back(0.0 + vnum % 4);
			vnum += 1;
			vertices.push_back(0.0 + vnum % 4);
			vnum += 1;*/
		}
		if (s1 == "f") {
			in >> s2 >> s3 >> s4;
			indices.push_back(stoi(s2));
			fnum += 1;
			indices.push_back(stoi(s3));
			fnum += 1;
			indices.push_back(stoi(s4));
			fnum += 1;
			//cout << ffnum << endl;
			//ffnum += 1;
		}
	}
	//vector<vector<int>> trianglelist;
	vector<vector<int>> pointlist[12];
	for (int i = 0; i < indices.size(); i = i + 3) {
		int point1 = indices[i];
		int point2 = indices[i + 1];
		int point3 = indices[i + 2];

		vector<int> triangles;
		triangles.push_back(point1 - 1);
		triangles.push_back(point2 - 1);
		triangles.push_back(point3 - 1);

		//cout << point1<<endl;
		pointlist[point1 - 1].push_back(triangles);
		//cout << point2<<endl;
		pointlist[point2 - 1].push_back(triangles);
		//cout << point3<<endl;
		pointlist[point3 - 1].push_back(triangles);

		triangles.clear();

	}
	for (int i = 0; i < 12; ++i) {
		float sumx = 0, sumy = 0, sumz = 0;
		for (int j = 0; j < pointlist[i].size(); ++j) {
			int point1 = pointlist[i][j][0];
			int point2 = pointlist[i][j][1];
			int point3 = pointlist[i][j][2];
			//cout << "point:" << point1 << " " << point2 << " " << point3 << endl;
			int id1 = point1 * 3;
			//cout << "id1:" << id1 << endl;
			float px1 = vertices2[id1];
			float py1 = vertices2[id1 + 1];
			float pz1 = vertices2[id1 + 2];
			int id2 = point2 * 3;
			//cout << "id2:" << id2 << endl;
			float px2 = vertices2[id2];
			float py2 = vertices2[id2 + 1];
			float pz2 = vertices2[id2 + 2];
			int id3 = point3 * 3;
			//cout << "id3:" << id3 << endl;
			float px3 = vertices2[id3];
			float py3 = vertices2[id3 + 1];
			float pz3 = vertices2[id3 + 2];
			float vec1x = px1 - px2;
			float vec1y = py1 - py2;
			float vec1z = pz1 - pz1;
			float vec2x = px3 - px1;
			float vec2y = py3 - py1;
			float vec2z = pz3 - pz1;
			float normx = vec1y * vec2z - vec1z * vec2y;
			float normy = vec1z * vec2x - vec1x * vec2z;
			float normz = vec1x * vec2y - vec1y * vec2x;
			sumx = sumx + normx;
			sumy = sumy + normy;
			sumz = sumz + normz;
		}
		sumx = sumx / pointlist[i].size();
		sumy = sumy / pointlist[i].size();
		sumz = sumz / pointlist[i].size();
		float mo = sqrtf(powf(sumx, 2) + powf(sumy, 2) + powf(sumz, 2));
		sumx = sumx / mo;
		sumy = sumy / mo;
		sumz = sumz / mo;
		norm.push_back(-sumx);
		norm.push_back(-sumy);
		norm.push_back(-sumz);
	}
	for (int i = 0; i < indices.size(); ++i) {
		int num = (indices[i] - 1) * 3;
		vertices.push_back(vertices2[num]);
		vertices.push_back(vertices2[num + 1]);
		vertices.push_back(vertices2[num + 2]);
		//vertices.push_back(1.0);
		//vertices.push_back(0.0);
		//vertices.push_back(0.0);
		vertices.push_back(norm[num]);
		vertices.push_back(norm[num + 1]);
		vertices.push_back(norm[num + 2]);
	}
	cout << "load finished" << endl;
	//cout << "vsize:" << vertices.size()<<"#"<<vertices[14997];
	inf.close();
}
float textvertices[] = {
	-25.0f,0.0f,-25.0f,0.0f,1.0f,
	25.0f,0.0f,-25.0f,1.0f,1.0f,
	-25.0f,0.0f,25.0f,0.0f,0.0f,

	25.0f,0.0f,-25.0f,1.0f,1.0f,
	25.0f,0.0f,25.0f,1.0f,0.0f,
	-25.0f,0.0f,25.0f,0.0f,0.0f
};

float road1[] = {
	-20.0f,0.0f,-20.0f,0.0f,1.0f,
	-15.0f,0.0f,-15.0f,1.0f,1.0f,
	-20.0f,0.0f,20.0f,0.0f,0.0f,

	-15.0f,0.0f,-15.0f,1.0f,1.0f,
	-15.0f,0.0f,15.0f,1.0f,0.0f,
	-20.0f,0.0f,20.0f,0.0f,0.0f
};
float road2[] = {
	20.0f,0.0f,-20.0f,0.0f,1.0f,
	15.0f,0.0f,-15.0f,1.0f,1.0f,
	-20.0f,0.0f,-20.0f,0.0f,0.0f,

	15.0f,0.0f,-15.0f,1.0f,1.0f,
	-15.0f,0.0f,-15.0f,1.0f,0.0f,
	-20.0f,0.0f,-20.0f,0.0f,0.0f
};
float road3[] = {
	15.0f,0.0f,-15.0f,0.0f,1.0f,
	20.0f,0.0f,-20.0f,1.0f,1.0f,
	15.0f,0.0f,15.0f,0.0f,0.0f,

	20.0f,0.0f,-20.0f,1.0f,1.0f,
	20.0f,0.0f,20.0f,1.0f,0.0f,
   15.0f,0.0f,15.0f,0.0f,0.0f
};
float road4[] = {
	20.0f,0.0f,20.0f,0.0f,1.0f,
	15.0f,0.0f,15.0f,1.0f,1.0f,
	-20.0f,0.0f,20.0f,0.0f,0.0f,

	15.0f,0.0f,15.0f,1.0f,1.0f,
	-15.0f,0.0f,15.0f,1.0f,0.0f,
	-20.0f,0.0f,20.0f,0.0f,0.0f
};


float lightvertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

unsigned int texture1, texture2;
unsigned int lightVBO, lightVAO, VBO, VAO, textVBO, textVAO, road1VBO, road1VAO;//, EBO;;
unsigned int road2VBO,road2VAO,road3VBO, road3VAO,road4VBO,road4VAO;
float velocity = 0.0f;
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
Shader shader,lampShader,lightingShader,textureShader, road1Shader, road2Shader, road3Shader, road4Shader, simpleDepthShader, debugDepthQuad;

GLfloat cubevertices[] = {
	// Back face
	-0.4f,0.0f, 0.2f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
	0.6f, 2.5f, 0.2f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
	0.6f, 0.0f, 0.2f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
	0.6f, 2.5f, 0.2f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  // top-right
	-0.4f, 0.0f, 0.2f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
	-0.4f, 2.5f, 0.2f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,// top-left
	// Front face
	-0.4f, 0.0f, 1.8f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
	0.6f, 0.0f, 1.8f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
	0.6f, 2.5f, 1.8f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // top-right
	0.6f, 2.5f, 1.8f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
	-0.4f, 2.5f, 1.8f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
	-0.4f, 0.0f, 1.8f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
	// Left face
	-0.4f, 2.5f, 1.8f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
	-0.4f, 2.5f, 0.2f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
	-0.4f, 0.0f, 0.2f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
	-0.4f, 0.0f, 0.2f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
	-0.4f, 0.0f, 1.8f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
	-0.4f, 2.5f, 1.8f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
	// Right face
	0.6f, 2.5f, 1.8f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
	0.6f, 0.0f, 0.2f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
	0.6f, 2.5f, 0.2f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
	0.6f, 0.0f, 0.2f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-right
	0.6f, 2.5f, 1.8f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-left
	0.6f, 0.0f, 1.8f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     
	// Bottom face
	-0.5f, 0.0f, 0.2f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
	0.6f, 0.0f, 0.2f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
	0.6f, 0.0f, 1.8f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,// bottom-left
	0.6f, 0.0f, 1.8f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
	-0.4f, 0.0f, 1.8f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
	-0.4f, 0.0f, 0.2f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
	// Top face
	-0.4f, 2.5f, 0.2f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
	0.6f, 2.5f, 1.8f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
	0.6f, 2.5f, 0.2f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right     
	0.6f, 2.5f, 1.8f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
	-0.4f, 2.5f, 0.2f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
	-0.4f, 2.5f, 1.8f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left        
};


// RenderQuad() Renders a 1x1 quad in NDC, best used for framebuffer color targets
// and post-processing effects.
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			// Positions        // Texture Coords
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		};
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
void road1buffer() {
	//texture*****************
	glGenVertexArrays(1, &road1VAO);
	glGenBuffers(1, &road1VBO);
	glBindBuffer(GL_ARRAY_BUFFER, road1VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(road1), road1, GL_STATIC_DRAW);
	glBindVertexArray(road1VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// texture 1
	// ---------
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char *data = stbi_load("D:\\vscode\\CGlab2Glut\\road.jpg", &width, &height, &nrChannels, STBI_rgb_alpha);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		cout << "loaded" << endl;
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	road1Shader.use();
	road1Shader.setInt("texture1", 0);
}
void road2buffer() {
	//texture*****************
	glGenVertexArrays(1, &road2VAO);
	glGenBuffers(1, &road2VBO);
	glBindBuffer(GL_ARRAY_BUFFER, road2VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(road2), road2, GL_STATIC_DRAW);
	glBindVertexArray(road2VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// texture 1
	// ---------
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char *data = stbi_load("D:\\vscode\\CGlab2Glut\\road.jpg", &width, &height, &nrChannels, STBI_rgb_alpha);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		cout << "loaded" << endl;
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	road2Shader.use();
	road2Shader.setInt("texture1", 0);
}
void road3buffer() {
	//texture*****************
	glGenVertexArrays(1, &road3VAO);
	glGenBuffers(1, &road3VBO);
	glBindBuffer(GL_ARRAY_BUFFER, road3VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(road3), road3, GL_STATIC_DRAW);
	glBindVertexArray(road3VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// texture 1
	// ---------
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char *data = stbi_load("D:\\vscode\\CGlab2Glut\\road.jpg", &width, &height, &nrChannels, STBI_rgb_alpha);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		cout << "loaded" << endl;
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	road3Shader.use();
	road3Shader.setInt("texture1", 0);
}
void road4buffer() {
	//texture*****************
	glGenVertexArrays(1, &road4VAO);
	glGenBuffers(1, &road4VBO);
	glBindBuffer(GL_ARRAY_BUFFER, road4VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(road4), road4, GL_STATIC_DRAW);
	glBindVertexArray(road4VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// texture 1
	// ---------
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char *data = stbi_load("D:\\vscode\\CGlab2Glut\\road.jpg", &width, &height, &nrChannels, STBI_rgb_alpha);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		cout << "loaded" << endl;
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	road4Shader.use();
	road4Shader.setInt("texture1", 0);
}

void depthbuffer() {

	GLfloat planeVertices[] = {
		// Positions          // Normals         // Texture Coords
		25.0f, 0.0f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
		-25.0f,  0.0f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
		-25.0f,  0.0f, 25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

		25.0f,  0.0f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
		25.0f,  0.0f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f,
		-25.0f,  0.0f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f
	};
	// Setup plane VAO
	GLuint planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glBindVertexArray(0);

	// Configure depth map FBO
	
	glGenFramebuffers(1, &depthMapFBO);
	// - Create depth texture
	
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void depthbuffer2() {

	GLfloat planeVertices2[] = {
	-20.0f,0.0f,-20.0f,0.0f, 1.0f, 0.0f,0.0f,1.0f,
	-15.0f,0.0f,-15.0f, 0.0f, 1.0f, 0.0f, 1.0f,1.0f,
	-20.0f,0.0f,20.0f, 0.0f, 1.0f, 0.0f,0.0f,0.0f,

	-15.0f,0.0f,-15.0f,0.0f, 1.0f, 0.0f, 1.0f,1.0f,
	-15.0f,0.0f,15.0f, 0.0f, 1.0f, 0.0f,1.0f,0.0f,
	-20.0f,0.0f,20.0f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f
	};
	// Setup plane VAO
	GLuint planeVBO2;
	glGenVertexArrays(1, &planeVAO2);
	glGenBuffers(1, &planeVBO2);
	glBindVertexArray(planeVAO2);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices2), &planeVertices2, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glBindVertexArray(0);


	// Configure depth map FBO

	glGenFramebuffers(1, &depthMapFBO2);
	// - Create depth texture

	glGenTextures(1, &depthMap2);
	glBindTexture(GL_TEXTURE_2D, depthMap2);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO2);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap2, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void depthbuffer3() {

	GLfloat planeVertices3[] = {
	20.0f,0.0f,-20.0f,0.0f, 1.0f, 0.0f,0.0f,1.0f,
	15.0f,0.0f,-15.0f, 0.0f, 1.0f, 0.0f, 1.0f,1.0f,
	-20.0f,0.0f,-20.0f, 0.0f, 1.0f, 0.0f,0.0f,0.0f,

	15.0f,0.0f,-15.0f,0.0f, 1.0f, 0.0f, 1.0f,1.0f,
	-15.0f,0.0f,-15.0f, 0.0f, 1.0f, 0.0f,1.0f,0.0f,
	-20.0f,0.0f,-20.0f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f
	};
	// Setup plane VAO
	GLuint planeVBO3;
	glGenVertexArrays(1, &planeVAO3);
	glGenBuffers(1, &planeVBO3);
	glBindVertexArray(planeVAO3);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices3), &planeVertices3, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glBindVertexArray(0);


	// Configure depth map FBO

	glGenFramebuffers(1, &depthMapFBO3);
	// - Create depth texture

	glGenTextures(1, &depthMap3);
	glBindTexture(GL_TEXTURE_2D, depthMap3);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO3);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap3, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void depthbuffer4() {

	GLfloat planeVertices4[] = {
	15.0f,0.0f,-15.0f,0.0f, 1.0f, 0.0f,0.0f,1.0f,
	20.0f,0.0f,-20.0f, 0.0f, 1.0f, 0.0f, 1.0f,1.0f,
	15.0f,0.0f,15.0f, 0.0f, 1.0f, 0.0f,0.0f,0.0f,

	20.0f,0.0f,-20.0f,0.0f, 1.0f, 0.0f, 1.0f,1.0f,
	20.0f,0.0f,20.0f, 0.0f, 1.0f, 0.0f,1.0f,0.0f,
	15.0f,0.0f,15.0f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f
	};
	// Setup plane VAO
	GLuint planeVBO4;
	glGenVertexArrays(1, &planeVAO4);
	glGenBuffers(1, &planeVBO4);
	glBindVertexArray(planeVAO4);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices4), &planeVertices4, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glBindVertexArray(0);


	// Configure depth map FBO

	glGenFramebuffers(1, &depthMapFBO4);
	// - Create depth texture

	glGenTextures(1, &depthMap4);
	glBindTexture(GL_TEXTURE_2D, depthMap4);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO4);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap2, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void depthbuffer5() {

	GLfloat planeVertices5[] = {
	20.0f,0.0f,20.0f,0.0f, 1.0f, 0.0f,0.0f,1.0f,
	15.0f,0.0f,15.0f, 0.0f, 1.0f, 0.0f, 1.0f,1.0f,
	-20.0f,0.0f,20.0f, 0.0f, 1.0f, 0.0f,0.0f,0.0f,

	15.0f,0.0f,15.0f,0.0f, 1.0f, 0.0f, 1.0f,1.0f,
	-15.0f,0.0f,15.0f, 0.0f, 1.0f, 0.0f,1.0f,0.0f,
	-20.0f,0.0f,20.0f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f
	};
	// Setup plane VAO
	GLuint planeVBO5;
	glGenVertexArrays(1, &planeVAO5);
	glGenBuffers(1, &planeVBO5);
	glBindVertexArray(planeVAO5);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO5);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices5), &planeVertices5, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glBindVertexArray(0);


	// Configure depth map FBO

	glGenFramebuffers(1, &depthMapFBO5);
	// - Create depth texture

	glGenTextures(1, &depthMap5);
	glBindTexture(GL_TEXTURE_2D, depthMap5);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO5);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap2, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void cardepthbuffer() {
	//glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	// Fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubevertices), cubevertices, GL_STATIC_DRAW);
	// Link vertex attributes
	glBindVertexArray(cubeVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
static void CreateVertexBuffer()
{
	
	
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &lightVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lightvertices), lightvertices, GL_STATIC_DRAW);

	glBindVertexArray(lightVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	

	//************	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size(), &indices[0], GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//texture*****************
	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textvertices), textvertices, GL_STATIC_DRAW);
	glBindVertexArray(textVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	// texture 1
	// ---------
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char *data = stbi_load("D:\\vscode\\CGlab2Glut\\sand.png", &width, &height, &nrChannels,  STBI_rgb_alpha);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		cout << "loaded"<<endl;
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	textureShader.use();
	textureShader.setInt("texture1", 0);

	road1buffer();
	road2buffer();
	road3buffer();
	road4buffer();
	depthbuffer();
	depthbuffer2();
	depthbuffer3();
	depthbuffer4();
	depthbuffer5();
	//cardepthbuffer();

	// material properties
	lightingShader.setVec3("material.ambient", 1.0f, 1.0f, 1.0f);
	lightingShader.setVec3("material.diffuse", 1.0f, 1.0f, 0.8f);
	lightingShader.setVec3("material.specular", 0.6f, 0.6f, 0.5f); // specular lighting doesn't have full effect on this object's material
	lightingShader.setFloat("material.shininess", 25.0f);
}

void carMove() {
	//car move
	if (velocity > 0.0f) {
		if (velocity < 0.0001f)velocity = 0.0f;
		else  velocity = velocity - 0.0001f;
	}
	if (velocity < 0.0f) {
		if (velocity > -0.0001f)velocity = 0.0f;
		else velocity = velocity + 0.0001f;
	}
	carPos = carPos + velocity * 0.05f*carFront;
	if (cameraTran == true) {
		camera.Position = carPos;
		camera.Front = carFront;
	}
	if (up_press == true) {
		//cout << "up";
		if (velocity < 0.5f) {
			if (velocity > 0.0f)
				velocity = velocity + (deltaTime / 1000)* 0.1f;
			else velocity = velocity + (deltaTime / 1000) * 0.3f;
		}
		else {
			velocity = 0.5f;

		}
	}
	if (down_press == true) {
		if (velocity > -0.5f) {
			if (velocity < 0.0f) {
				velocity = velocity - (deltaTime / 1000) * 0.1f;
			}
			else velocity = velocity - (deltaTime / 1000) * 0.3f;
		}
		else {
			velocity = -0.5f;

		}

	}
	if (right_press == true) {
		if (carangle > (deltaTime / 1000) * 50.0f) {
			carangle = carangle - (deltaTime / 1000) * 50.0f;//camera.ProcessKeyboard(ROTATE, deltaTime);
		}
		else carangle = 360.0f;
		carFront = glm::vec3(-sin(glm::radians(carangle)), 0.0f, -cos(glm::radians(carangle)));
		if (cameraTran == true) {
			camera.Position = carPos;
			camera.Front = carFront;
		}
	}
	if (left_press == true) {
		if (carangle < 360.0f - (deltaTime / 1000) * 50.0f)
			carangle = carangle + (deltaTime / 1000) * 50.0f;//camera.ProcessKeyboard(ROTATE, deltaTime);
		else carangle = 0.0f;
		//carangle = 45.0f;
		carFront = glm::vec3(-sin(glm::radians(carangle)), 0.0f, -cos(glm::radians(carangle)));
		if (cameraTran == true) {
			camera.Position = carPos;
			camera.Front = carFront;
		}
	}



}

void lampdraw(glm::mat4 projection, glm::mat4 view) {
	// also draw the lamp object
	lampShader.use();
	lampShader.setMat4("projection", projection);
	lampShader.setMat4("view", view);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, lightPos);
	model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
	lampShader.setMat4("model", model);
	glBindVertexArray(lightVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);


	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_POINTS, 0, 1);

	glDisableVertexAttribArray(0);
}

void cardraw(glm::mat4 projection, glm::mat4 view) {
	lightingShader.use();
	lightingShader.setMat4("projection", projection);
	// camera/view transformation

	lightingShader.setMat4("view", view);
	// calculate the model matrix for each object and pass it to shader before drawing
	glm::mat4 carmodel = glm::mat4(1.0f);
	carmodel = glm::translate(carmodel, carPos);
	carmodel = glm::rotate(carmodel, glm::radians(carangle), glm::vec3(0.0f, 1.0f, 0.0f));

	//model = glm::translate(model, cubePositions[i]);


	lightingShader.setMat4("model", carmodel);
	lightingShader.setVec3("light.position", lightPos);
	lightingShader.setVec3("viewPos", camera.Position);
	// light properties
	glm::vec3 lightColor;
	lightColor.x = 1.0f;//sin(glfwGetTime() * 2.0f);
	lightColor.y = 1.0f;//sin(glfwGetTime() * 0.7f);
	lightColor.z = 1.0f;//sin(glfwGetTime() * 1.3f);
	glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
	glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
	lightingShader.setVec3("light.ambient", ambientColor);
	lightingShader.setVec3("light.diffuse", diffuseColor);
	lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	
	//glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(carmodel));
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 60);
	glBindVertexArray(0);
}

void textdraw(glm::mat4 projection, glm::mat4 view) {
	//texture****************************
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);
	textureShader.use();
	textureShader.setMat4("projection", projection);
	textureShader.setMat4("view", view);
	glm::mat4 textmodel = glm::mat4(1.0f);
	textureShader.setMat4("model", textmodel);
	glBindVertexArray(textVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture2);
	road2Shader.use();
	road2Shader.setMat4("projection", projection);
	road2Shader.setMat4("view", view);
	road2Shader.setMat4("model", textmodel);
	glBindVertexArray(road2VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture2);
	road1Shader.use();
	road1Shader.setMat4("projection", projection);
	road1Shader.setMat4("view", view);
	road1Shader.setMat4("model", textmodel);
	glBindVertexArray(road1VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture2);
	road3Shader.use();
	road3Shader.setMat4("projection", projection);
	road3Shader.setMat4("view", view);
	road3Shader.setMat4("model", textmodel);
	glBindVertexArray(road3VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture2);
	road4Shader.use();
	road4Shader.setMat4("projection", projection);
	road4Shader.setMat4("view", view);
	road4Shader.setMat4("model", textmodel);
	glBindVertexArray(road4VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);



}

void RenderScene(glm::mat4 projection, glm::mat4 view,Shader &shader,int b)
{
	if (b == 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
	}
	glm::mat4 model=glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	if (b == 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
	}
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(planeVAO2);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	if (b == 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
	}
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(planeVAO3);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	if (b == 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
	}
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(planeVAO4);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	if (b == 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
	}
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(planeVAO5);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	if (b == 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
	}
		model = glm::translate(model, carPos);
		model = glm::rotate(model, glm::radians(carangle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		
		cardepthbuffer();
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		
		

	//cardraw(projection, view,shader);
	// Floor
	//glm::mat4 model;
	//glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
	//glBindVertexArray(planeVAO);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	//glBindVertexArray(0);

}

void shadow(glm::mat4 projection, glm::mat4 view) {
	// 1. Render depth of scene to texture (from light's perspective)
		// - Get light projection/view matrix.
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	GLfloat near_plane = 1.0f, far_plane = 70.0f;
	lightProjection =glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
	lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	// - render scene from light's point of view
	simpleDepthShader.use();
	glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	RenderScene(projection, view,simpleDepthShader,1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// 2. Render scene as normal 
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader.use();
	projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
	view = camera.GetViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
	// Set light uniforms
	glUniform3fv(glGetUniformLocation(shader.ID, "lightPos"), 1, &lightPos[0]);
	glUniform3fv(glGetUniformLocation(shader.ID, "viewPos"), 1, &camera.Position[0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	
	RenderScene(projection, view,shader,0);

	// 3. DEBUG: visualize depth map by rendering it to plane
	debugDepthQuad.use();
	glUniform1f(glGetUniformLocation(debugDepthQuad.ID, "near_plane"), near_plane);
	glUniform1f(glGetUniformLocation(debugDepthQuad.ID, "far_plane"), far_plane);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
}
static void RenderSceneCB()
{
	carMove();
	if (cameraTran == true) {
		camera.Position = carPos;
		camera.Position.y += 1.5f;
		camera.Front = carFront;
		//camera.Front.y = -camera.Front.y;
		//camera.Front.y -= 0.5f;
		camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
		camera.Up= glm::normalize(glm::cross(camera.Right, camera.Front));
	}
	else {
		camera.Position= glm::vec3(0.0, 60.0, 5.0);
		camera.Front = -camera.Position;
		camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
		camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));
		
	}
	float currentFrame = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = (currentFrame - lastFrame);
	//cout << deltaTime << endl;
	lastFrame = currentFrame;
	// render
		// ------
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

	// pass projection matrix to shader (note that in this case it could change every frame)
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
	glm::mat4 view = camera.GetViewMatrix();

	
	lampdraw(projection, view);
	//textdraw(projection, view);
	shadow(projection,view);
	cardraw(projection, view);
	
	
	glutSwapBuffers();
	glFlush();
}


static void InitializeGlutCallbacks()
{
	glutDisplayFunc(RenderSceneCB);
	glutIdleFunc(RenderSceneCB);
}

void MYReshape(int w , int h) {
	GLfloat ratio = (GLfloat)w / (GLfloat)h;
	SCR_WIDTH = w;
	SCR_HEIGHT = h;
	glViewport (0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, ratio, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv)
{
	car();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glEnable(GL_DEPTH_TEST);
	glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Car");

	

	// Must be done after glut is initialized!
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	lampShader.Create(lampvs, lampfs);
	lightingShader.Create(vertexShaderSource, fragmentShaderSource);
	textureShader.Create(texturevs, texturefs);
	road1Shader.Create(texturevs, texturefs);
	road2Shader.Create(texturevs, texturefs);
	road3Shader.Create(texturevs, texturefs);
	road4Shader.Create(texturevs, texturefs);

	simpleDepthShader.Create(shadow_mapping_depthvs, shadow_mapping_depthfs);
	debugDepthQuad.Create(debug_quadvs, debug_quad_depthfs);
	shader.Create(shadow_mappingvs, shadow_mappingfs);
	CreateVertexBuffer();

	InitializeGlutCallbacks();

	

	glutSpecialFunc(&processSpecialKeys);
	glutSpecialUpFunc(&processSpeciaUplKeys);
	glutKeyboardFunc(&processKeys);
	glutReshapeFunc(&MYReshape);

	createGLUTMenus();

	glutMainLoop();

	return 0;
}
void processKeys(unsigned char key, int x, int y) {
	if (key == 't') {
		//camera.Position.y -= 5.0f;
		cameraTran = !cameraTran;
	}
}

void createGLUTMenus() {

	int menu;

	// 创建菜单并告诉GLUT，processMenuEvents处理菜单事件。
		menu = glutCreateMenu(processMenuEvents);

	//给菜单增加条目
		glutAddMenuEntry("Red", 1);
	glutAddMenuEntry("Blue", 2);
	glutAddMenuEntry("Green", 3);
	glutAddMenuEntry("White", 4);
	glutAddMenuEntry("Exit", 5);

	// 把菜单和鼠标右键关联起来。
		glutAttachMenu(GLUT_RIGHT_BUTTON);
}
void processMenuEvents(int option) {
	//option 就是传递过来的value的值。
		switch (option) {
		case 1:
			// material properties
			lightingShader.setVec3("material.ambient", 1.0f, 0.0f, 0.0f);
			lightingShader.setVec3("material.diffuse", 1.0f, 0.0f, 0.0f);
			lightingShader.setVec3("material.specular", 0.6f, 0.6f, 0.5f); // specular lighting doesn't have full effect on this object's material
			lightingShader.setFloat("material.shininess", 25.0f); break;
		
		case 2:
			// material properties
			lightingShader.setVec3("material.ambient", 0.0f, 0.0f, 1.0f);
			lightingShader.setVec3("material.diffuse", 0.0f, 0.0f, 1.0f);
			lightingShader.setVec3("material.specular", 0.6f, 0.6f, 0.5f); // specular lighting doesn't have full effect on this object's material
			lightingShader.setFloat("material.shininess", 25.0f); break;
		case 3:
			// material properties
			lightingShader.setVec3("material.ambient", 0.0f, 1.0f, 0.0f);
			lightingShader.setVec3("material.diffuse", 0.0f, 1.0f, 0.0f);
			lightingShader.setVec3("material.specular", 0.6f, 0.6f, 0.5f); // specular lighting doesn't have full effect on this object's material
			lightingShader.setFloat("material.shininess", 25.0f); break;
		case 4:
			// material properties
			lightingShader.setVec3("material.ambient", 1.0f, 1.0f, 1.0f);
			lightingShader.setVec3("material.diffuse", 1.0f, 1.0f, 0.8f);
			lightingShader.setVec3("material.specular", 0.6f, 0.6f, 0.5f); // specular lighting doesn't have full effect on this object's material
			lightingShader.setFloat("material.shininess", 25.0f); break;
		case 5:
			exit(0);
		}
}
void processSpecialKeys(int key, int x, int y) {

	
	switch (key) {
	case GLUT_KEY_UP:
		up_press= true;

		break;
	case GLUT_KEY_DOWN:
		down_press = true;
		break;
	case GLUT_KEY_LEFT:
		left_press = true;
		break;
	case GLUT_KEY_RIGHT:
		right_press = true;
		break;
	}
}

void processSpeciaUplKeys(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		up_press = false;

		break;
	case GLUT_KEY_DOWN:
		down_press = false;
		break;
	case GLUT_KEY_LEFT:
		left_press = false;
		break;
	case GLUT_KEY_RIGHT:
		right_press = false;
		break;
	}
}

