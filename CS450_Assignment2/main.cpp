// Simple 2D OpenGL Program

//Includes vec, mat, and other include files as well as macro defs
#define GL3_PROTOTYPES

// Include the vector and matrix utilities from the textbook, as well as some
// macro definitions.
#include "Angel.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <fstream>

#ifdef __APPLE__
#  include <OpenGL/gl3.h>
#endif

using namespace std;

class Splitter {
	std::vector<std::string> _tokens;
public:
	typedef std::vector<std::string>::size_type size_type;
public:

	Splitter ( const std::string& src, const std::string& delim )
	{
		reset ( src, delim );
	}

	std::string& operator[] ( size_type i )
	{
		return _tokens.at ( i );
	}

	size_type size() const
	{
		return _tokens.size();
	}

	void reset ( const std::string& src, const std::string& delim )
	{
		std::vector<std::string> tokens;
		std::string::size_type start = 0;
		std::string::size_type end;
		for ( ; ; ) {
			end = src.find ( delim, start );
			tokens.push_back ( src.substr ( start, end - start ) );
			// We just copied the last token
			if ( end == std::string::npos )
				break;
			// Exclude the delimiter in the next search
			start = end + delim.size();
		}
		_tokens.swap ( tokens );
	}
};


#define NO_OBJECT_SELECTED -1
#define NO_PREVIOUS_X -INT_MAX
#define WINDOW_SIZE 512

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  projection; // projection matrix uniform shader variable location

vector<vector<point4>>	vertexStore;
vector<vector<vec4>>	normalStore;

// vectors of each objects vertices/normals
vector<vector<point4>>	vertices;
vector<vector<vec4>>	normals;

vector<struct LookAtInfo> modelViewMatrices;

vector<GLuint> VBOs;
vector<GLuint> VAOs;
vector<color4> colors;

bool mouseDown;
int objectSelected;
// number of vertices used for axis lines
int axisLineVerticesCount = 0;
// number of vertices used for axis line end caps
int endCapVerticesCount = 216;

struct LookAtInfo
{
	vec4 eye;
	vec4 at;
	vec4 up;
	vec3 rotate;
	vec3 scale;
	vec3 translate;
};

GLuint program;

enum TransformMode {
	ModeRotate = 0,
	ModeTranslate,
	ModeScale
};

enum Axis {
	XAxis = 0,
	YAxis,
	ZAxis,
	NoAxis
};

enum mouseTrackDirection {
	UpDown = 0,
	LeftRight
};

struct Coordinates {
	int x;
	int y;
};

TransformMode mode;
Axis selectedAxis;
struct Coordinates mouseLoc;

static int previousMousePointX = NO_PREVIOUS_X;

#pragma mark Function declarations
vector<string> readSceneFile(string fileName);
void loadObjectFromFile(string objFileName);
void normalizeVector(vec4 *vector, vec4 min, vec4 max);

#pragma mark -


//----------------------------------------------------------------------------

void addTri( int pointA, int pointB, int pointC, int normalA, int normalB, int normalC, int index )
{
	vertices.back().push_back(vertexStore[index][pointA-1]);
	vertices.back().push_back(vertexStore[index][pointB-1]);
	vertices.back().push_back(vertexStore[index][pointC-1]);

	normals.back().push_back(normalStore[index][normalA-1]);
	normals.back().push_back(normalStore[index][normalB-1]);
	normals.back().push_back(normalStore[index][normalC-1]);
}

void addLine( vec4 pointA, vec4 pointB )
{
	vertices.back().push_back(pointA);
	vertices.back().push_back(pointB);

	normals.back().push_back(pointA);
	normals.back().push_back(pointB);

	axisLineVerticesCount += 2;
}

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 cubeVertex(vec3 center, GLfloat sideLength, int vertexIndex)
{
	point4 cubeVertices[8] = {
		point4( center.x-sideLength/2, center.y-sideLength/2, center.z+sideLength/2, 1.0 ),
		point4( center.x-sideLength/2, center.y+sideLength/2, center.z+sideLength/2, 1.0 ),
		point4( center.x+sideLength/2, center.y+sideLength/2, center.z+sideLength/2, 1.0 ),
		point4( center.x+sideLength/2, center.y-sideLength/2, center.z+sideLength/2, 1.0 ),
		point4( center.x-sideLength/2, center.y-sideLength/2, center.z-sideLength/2, 1.0 ),
		point4( center.x-sideLength/2, center.y+sideLength/2, center.z-sideLength/2, 1.0 ),
		point4( center.x+sideLength/2, center.y+sideLength/2, center.z-sideLength/2, 1.0 ),
		point4( center.x+sideLength/2, center.y-sideLength/2, center.z-sideLength/2, 1.0 )
	};

	return cubeVertices[vertexIndex];
}

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices.  Notice we keep the relative ordering when constructing the tris
void addCube( vec3 center, GLfloat sideLength )
{
	vertices.back().push_back(cubeVertex(center, sideLength, 4));
	vertices.back().push_back(cubeVertex(center, sideLength, 5));
	vertices.back().push_back(cubeVertex(center, sideLength, 6));
	vertices.back().push_back(cubeVertex(center, sideLength, 4));
	vertices.back().push_back(cubeVertex(center, sideLength, 6));
	vertices.back().push_back(cubeVertex(center, sideLength, 7));
	vertices.back().push_back(cubeVertex(center, sideLength, 5));
	vertices.back().push_back(cubeVertex(center, sideLength, 4));
	vertices.back().push_back(cubeVertex(center, sideLength, 0));
	vertices.back().push_back(cubeVertex(center, sideLength, 5));
	vertices.back().push_back(cubeVertex(center, sideLength, 0));
	vertices.back().push_back(cubeVertex(center, sideLength, 1));
	vertices.back().push_back(cubeVertex(center, sideLength, 1));
	vertices.back().push_back(cubeVertex(center, sideLength, 0));
	vertices.back().push_back(cubeVertex(center, sideLength, 3));
	vertices.back().push_back(cubeVertex(center, sideLength, 1));
	vertices.back().push_back(cubeVertex(center, sideLength, 3));
	vertices.back().push_back(cubeVertex(center, sideLength, 2));
	vertices.back().push_back(cubeVertex(center, sideLength, 2));
	vertices.back().push_back(cubeVertex(center, sideLength, 3));
	vertices.back().push_back(cubeVertex(center, sideLength, 7));
	vertices.back().push_back(cubeVertex(center, sideLength, 2));
	vertices.back().push_back(cubeVertex(center, sideLength, 7));
	vertices.back().push_back(cubeVertex(center, sideLength, 6));
	vertices.back().push_back(cubeVertex(center, sideLength, 3));
	vertices.back().push_back(cubeVertex(center, sideLength, 0));
	vertices.back().push_back(cubeVertex(center, sideLength, 4));
	vertices.back().push_back(cubeVertex(center, sideLength, 3));
	vertices.back().push_back(cubeVertex(center, sideLength, 4));
	vertices.back().push_back(cubeVertex(center, sideLength, 7));
	vertices.back().push_back(cubeVertex(center, sideLength, 6));
	vertices.back().push_back(cubeVertex(center, sideLength, 5));
	vertices.back().push_back(cubeVertex(center, sideLength, 1));
	vertices.back().push_back(cubeVertex(center, sideLength, 6));
	vertices.back().push_back(cubeVertex(center, sideLength, 1));
	vertices.back().push_back(cubeVertex(center, sideLength, 2));
}

//----------------------------------------------------------------------------

// OpenGL initialization
void init()
{
    // Load shaders and use the resulting shader program
    program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram( program );

	for (int i = 0; i < vertices.size(); i++)
	{
		GLuint buffer1;
		VBOs.push_back(buffer1);

		GLuint buffer2;
		VAOs.push_back(buffer2);
	}

	glGenVertexArrays( (int)VAOs.size(), &VAOs[0] );
    glGenBuffers( (int)VBOs.size(), &VBOs[0] );

	for (int i = 0; i < VBOs.size(); i++)
	{
		glBindVertexArray( VAOs[i] );
		glBindBuffer( GL_ARRAY_BUFFER, VBOs[i] );
		glBufferData( GL_ARRAY_BUFFER, (vertices[i].size() + normals[i].size()) * sizeof(vec4), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, vertices[i].size() * sizeof(point4), &vertices[i][0] );
		glBufferSubData( GL_ARRAY_BUFFER, vertices[i].size() * sizeof(point4), normals[i].size() * sizeof(vec4), &normals[i][0] );

		// set up vertex arrays
		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

		GLuint vNormal = glGetAttribLocation( program, "vNormal" );
		glEnableVertexAttribArray( vNormal );
		glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertices[i].size() * sizeof(point4)) );
	}

    // Initialize shader lighting parameters
    // RAM: No need to change these...we'll learn about the details when we
    // cover Illumination and Shading
    vec4 light_position( 1.5, 0.5, 2.0, 1.0 );
    color4 light_ambient( 0.2, 0.5, 0.2, 1.0 );
    color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
    color4 light_specular( 1.0, 1.0, 1.0, 1.0 );

    color4 material_ambient( 1.0, 0.0, 1.0, 1.0 );
    color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
    color4 material_specular( 1.0, 0.8, 0.0, 1.0 );
    float  material_shininess = 100.0;

    color4 ambient_product = light_ambient * material_ambient;
    color4 diffuse_product = light_diffuse * material_diffuse;
    color4 specular_product = light_specular * material_specular;

    glUniform4fv( glGetUniformLocation(program, "AmbientProduct"), 1, ambient_product );
    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"), 1, diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"), 1, specular_product );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, light_position );
    glUniform1f( glGetUniformLocation(program, "Shininess"), material_shininess );


    model_view = glGetUniformLocation( program, "ModelView" );
    projection = glGetUniformLocation( program, "Projection" );

//	mat4 mv = LookAt( eye, at, up );
//	mat4 p = Ortho(-0.094552, 0.06105, 0.033349, 0.186195, -5, 5);

//	glUniformMatrix4fv( model_view, 1, GL_TRUE, LookAt(vec4(0.0, 0.0, 1.5, 1.0),
//													   vec4(0.0, 0.0, 0.0, 1.0),
//													   vec4(0.0, 1.0, 0.0, 0.0)) );

	mat4 p = Perspective (90.0, 1.0, 0.1, 20.0);
    glUniformMatrix4fv( projection, 1, GL_TRUE, p );


	for (int i = 0; i < vertices.size(); i++)
	{
		struct LookAtInfo lookAtInfo;
		lookAtInfo.eye = vec4(0.0+i-2, 0.0, 3.0, 1.0);
		lookAtInfo.at = vec4(0.0+i-2, 0.0, 0.0, 1.0);
		lookAtInfo.up = vec4(0.0, 1.0, 0.0, 0.0);
		lookAtInfo.rotate = 0.0;
		lookAtInfo.translate = 0.0;
		lookAtInfo.scale = 1.0;
		modelViewMatrices.push_back(lookAtInfo);
	}

	for (int i = 0; i < modelViewMatrices.size(); i++)
	{
		float r;
		float g;
		float b;
		do
		{
			r = (arc4random() % 255);
			g = (arc4random() % 255);
			b = (arc4random() % 255);
		} while (r == 1 || g == 1 || b == 1);

		colors.push_back(vec4(r, g, b, 1.0f));
	}

    glEnable( GL_DEPTH_TEST );
    glClearColor( 1.0, 1.0, 1.0, 1.0 );

	objectSelected = NO_OBJECT_SELECTED;
}

//----------------------------------------------------------------------------

void display( void )
{
	printf("mouse at (%i, %i)\n", mouseLoc.x, mouseLoc.y);


	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	mat4 transformedMatrix;

	for (int i = 0; i < VAOs.size(); i++)
	{
		glBindVertexArray(VAOs[i]);

		transformedMatrix = LookAt(modelViewMatrices[i].eye, modelViewMatrices[i].at, modelViewMatrices[i].up) * RotateX(modelViewMatrices[i].rotate.x)
		*= RotateY(modelViewMatrices[i].rotate.y)
		*= RotateZ(modelViewMatrices[i].rotate.z)
		*= Translate(modelViewMatrices[i].translate.x, modelViewMatrices[i].translate.y, modelViewMatrices[i].translate.z)
		*= Scale(modelViewMatrices[i].scale.x, modelViewMatrices[i].scale.y, modelViewMatrices[i].scale.z);

		glUniformMatrix4fv(model_view, 1, GL_TRUE, transformedMatrix);

		if (mouseDown)
		{
			// set the colorID for the current object being checked.
			glUniform4f(glGetUniformLocation(program, "colorID"), colors[i].x/255.0, colors[i].y/255.0, colors[i].z/255.0, 1.0f);
		}
		else
		{
			if (i == objectSelected)
			{
				// if there's an object selected, draw it in wireframe mode.
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glPolygonOffset(1.0, 2 );
			}
			else
			{
				// else draw it filled.
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			// set colorID of fshader to -1 to let it know mouse is not down.
			glUniform4f(glGetUniformLocation(program, "colorID"), -1.0f, 0.0f, 0.0f, 0.0f);
		}

		// draw the object
		glDrawArrays(GL_TRIANGLES, 0, (int)vertices[i].size()-axisLineVerticesCount-endCapVerticesCount);

		// draw axis lines/endcaps if object is selected
		if (i == objectSelected)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glUniform4f(glGetUniformLocation(program, "colorID"), 1.0, 0.0, 0.0, 1.0);
			glDrawArrays(GL_TRIANGLES, (int)vertices[i].size() - axisLineVerticesCount - endCapVerticesCount, endCapVerticesCount/3);

			glUniform4f(glGetUniformLocation(program, "colorID"), 0.0, 1.0, 0.0, 1.0);
			glDrawArrays(GL_TRIANGLES, (int)vertices[i].size() - axisLineVerticesCount - endCapVerticesCount + endCapVerticesCount/3, endCapVerticesCount/3);

			glUniform4f(glGetUniformLocation(program, "colorID"), 0.0, 0.0, 1.0, 1.0);
			glDrawArrays(GL_TRIANGLES, (int)vertices[i].size() - axisLineVerticesCount - endCapVerticesCount + (2*endCapVerticesCount/3), endCapVerticesCount/3);

			glUniform4f(glGetUniformLocation(program, "colorID"), 0.0, 0.0, 0.0, 1.0);
			glDrawArrays(GL_LINES, (int)vertices[i].size()-axisLineVerticesCount, axisLineVerticesCount);
		}
	}

	glutSwapBuffers();

	if (mouseDown)
	{
		mouseDown = false;

		glFlush();
		glFinish();

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		unsigned char data[4];

		glReadPixels(mouseLoc.x, mouseLoc.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

		for (int i = 0; i < colors.size(); i++)
		{
			if (data[0] == 255 && data[1] == 255 && data[2] == 255)
				objectSelected = NO_OBJECT_SELECTED;
			if (colors[i].x == data[0] && colors[i].y == data[1] && colors[i].z == data[2])
				objectSelected = i;

			if (data[0] == 255 && data[1] == 0 && data[2] == 0)
				selectedAxis = XAxis;
			else if (data[0] == 0 && data[1] == 255 && data[2] == 0)
				selectedAxis = YAxis;
			else if (data[0] == 0 && data[1] == 0 && data[2] == 255)
				selectedAxis = ZAxis;
			else
				selectedAxis = NoAxis;


			printf("\nobj selected: %i\n", objectSelected);
		}

		glutPostRedisplay();
	}

}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y )
{

    switch( key ) {
	case 'a':
		{
			modelViewMatrices[objectSelected].eye.x += .1;
			modelViewMatrices[objectSelected].at.x += .1;
			break;
		}
	case 'w':
		{
			modelViewMatrices[objectSelected].eye.z += .1;
			modelViewMatrices[objectSelected].at.z += .1;
			break;
		}
	case 'd':
		{
			modelViewMatrices[objectSelected].eye.x -= .1;
			modelViewMatrices[objectSelected].at.x -= .1;
			break;
		}
	case 's':
		{
			modelViewMatrices[objectSelected].eye.z -= .1;
			modelViewMatrices[objectSelected].at.z -= .1;
			break;
		}
	case 'e':
		{
			modelViewMatrices[objectSelected].eye.y -= .1;
			modelViewMatrices[objectSelected].at.y -= .1;
			break;
		}
	case 'q':
		{
			modelViewMatrices[objectSelected].eye.y += .1;
			modelViewMatrices[objectSelected].at.y += .1;
			break;
		}
	case 'u':
		{
			modelViewMatrices[objectSelected].rotate.x -= 45;
			break;
		}
	case 'i':
		{
			modelViewMatrices[objectSelected].rotate.x += 45;
			break;
		}
	case 'j':
		{
			modelViewMatrices[objectSelected].rotate.y -= 45;
			break;
		}
	case 'k':
		{
			modelViewMatrices[objectSelected].rotate.y += 45;
			break;
		}
	case 'n':
		{
			modelViewMatrices[objectSelected].rotate.z -= 45;
			break;
		}
	case 'm':
		{
			modelViewMatrices[objectSelected].rotate.z += 45;
			break;
		}
	case '1':
		{
			mode = ModeRotate;
			printf("mode: rotate\n");
			break;
		}
	case '2':
		{
			mode = ModeScale;
			printf("mode: scale\n");
			break;
		}
	case '3':
		{
			mode = ModeTranslate;
			printf("mode: translate\n");
			break;
		}
	case '-':
		{
			switch (mode) {
				case ModeRotate:
					switch (selectedAxis) {
						case XAxis:
							modelViewMatrices[objectSelected].rotate.x -= 1;
							break;
						case YAxis:
							modelViewMatrices[objectSelected].rotate.y -= 1;
							break;
						case ZAxis:
							modelViewMatrices[objectSelected].rotate.z -= 1;
							break;
						default:
							break;
					}
					glutPostRedisplay();
					break;
				case ModeTranslate:
					switch (selectedAxis) {
						case XAxis:
							modelViewMatrices[objectSelected].translate.x -= .05;
							break;
						case YAxis:
							modelViewMatrices[objectSelected].translate.y -= .05;
							break;
						case ZAxis:
							modelViewMatrices[objectSelected].translate.z -= .05;
							break;
						default:
							break;
					}
					glutPostRedisplay();
					break;
				case ModeScale:
					switch (selectedAxis) {
						case XAxis:
							modelViewMatrices[objectSelected].scale.x -= .1;
							break;
						case YAxis:
							modelViewMatrices[objectSelected].scale.y -= .1;
							break;
						case ZAxis:
							modelViewMatrices[objectSelected].scale.z -= .1;
							break;
						default:
							break;
					}
					glutPostRedisplay();
					break;
				default:
					break;
			}
			break;
		}
	case '=':
		{
			switch (mode) {
				case ModeRotate:
					switch (selectedAxis) {
						case XAxis:
							modelViewMatrices[objectSelected].rotate.x += 1;
							break;
						case YAxis:
							modelViewMatrices[objectSelected].rotate.y += 1;
							break;
						case ZAxis:
							modelViewMatrices[objectSelected].rotate.z += 1;
							break;
						default:
							break;
					}
					glutPostRedisplay();
					break;
				case ModeTranslate:
					switch (selectedAxis) {
						case XAxis:
							modelViewMatrices[objectSelected].translate.x += .05;
							break;
						case YAxis:
							modelViewMatrices[objectSelected].translate.y += .05;
							break;
						case ZAxis:
							modelViewMatrices[objectSelected].translate.z += .05;
							break;
						default:
							break;
					}
					glutPostRedisplay();
					break;
				case ModeScale:
					switch (selectedAxis) {
						case XAxis:
							modelViewMatrices[objectSelected].scale.x += .1;
							break;
						case YAxis:
							modelViewMatrices[objectSelected].scale.y += .1;
							break;
						case ZAxis:
							modelViewMatrices[objectSelected].scale.z += .1;
							break;
						default:
							break;
					}
					glutPostRedisplay();
					break;
				default:
					break;
			}
			break;
		}
    }

	glutPostRedisplay();

	printf("eye= (%f, %f, %f)\nat= (%f, %f, %f)\nrotate= (%f, %f, %f)\n",
		   modelViewMatrices[0].eye.x, modelViewMatrices[0].eye.y, modelViewMatrices[0].eye.z,
		   modelViewMatrices[0].at.x, modelViewMatrices[0].at.y, modelViewMatrices[0].at.z,
		   modelViewMatrices[0].rotate.x, modelViewMatrices[0].rotate.y, modelViewMatrices[0].rotate.z);
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			mouseLoc.x = x;
			mouseLoc.y = y + 2*(WINDOW_SIZE/2 - y);
			mouseDown = true;
		}
		else if (state == GLUT_UP)
		{
			mouseDown = false;
			previousMousePointX = NO_PREVIOUS_X;
		}

		glutPostRedisplay();
	}
}

void mouseDidMove(int x, int y)
{
	mouseLoc.x = x;
	mouseLoc.y = y + 2*(WINDOW_SIZE/2 - y);

	if (previousMousePointX == NO_PREVIOUS_X)
		previousMousePointX = x;

	int diffX = x - previousMousePointX;

	printf("x= %i, prevX = %i, diffX= %i\n", x, previousMousePointX, diffX);

	switch (mode) {
		case ModeRotate:
			switch (selectedAxis) {
				case XAxis:
					modelViewMatrices[objectSelected].rotate.x += diffX;
					break;
				case YAxis:
					modelViewMatrices[objectSelected].rotate.y += diffX;
					break;
				case ZAxis:
					modelViewMatrices[objectSelected].rotate.z += diffX;
					break;
				default:
					break;
			}
			glutPostRedisplay();
			break;
		case ModeTranslate:
			switch (selectedAxis) {
				case XAxis:
					modelViewMatrices[objectSelected].translate.x += diffX * .005;
					break;
				case YAxis:
					modelViewMatrices[objectSelected].translate.y += diffX * .005;
					break;
				case ZAxis:
					modelViewMatrices[objectSelected].translate.z += diffX * .005;
					break;
				default:
					break;
			}
			glutPostRedisplay();
			break;
		case ModeScale:
			switch (selectedAxis) {
				case XAxis:
					modelViewMatrices[objectSelected].scale.x += diffX * .05;
					break;
				case YAxis:
					modelViewMatrices[objectSelected].scale.y += diffX * .05;
					break;
				case ZAxis:
					modelViewMatrices[objectSelected].scale.z += diffX * .05;
					break;
				default:
					break;
			}
			glutPostRedisplay();
			break;
		default:
			break;
	}

	previousMousePointX = x;
}

//----------------------------------------------------------------------------


int main(int argc, char** argv)
{
	string sceneFileName;
	vector<string> objectFileNames;

	if (argc > 10)
	{
		sceneFileName = argv[1];
	}
	else
	{
		sceneFileName = "test.scn";
	}


//	objectFileNames = readSceneFile(sceneFileName);
	objectFileNames.push_back("bunnyS.obj");
	objectFileNames.push_back("cow.obj");
	objectFileNames.push_back("frog.obj");
//	objectFileNames.push_back("sandal.obj");
//	objectFileNames.push_back("streetlamp.obj");
//	objectFileNames.push_back("teapotL.obj");

	for (int i = 0; i < objectFileNames.size(); i++)
	{
		loadObjectFromFile(objectFileNames[i]);
	}

    glutInit(&argc, argv);
#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DEPTH);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitContextVersion (3, 2);
    glutInitContextFlags (GLUT_FORWARD_COMPATIBLE);
#endif
    glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE);
    glutInitWindowPosition(500, 300);
    glutCreateWindow("Simple Open GL Program");
    printf("%s\n%s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));

#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    init();

    //NOTE:  callbacks must go after window is created!!!
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseDidMove);
    glutMainLoop();

    return(0);
}

vector<string> readSceneFile(string fileName)
{
	vector<string> objectFileNames;
	ifstream fileStream(fileName);
	string line;

	fileStream.clear();
	if (fileStream.is_open())
	{
		getline(fileStream, line);
		Splitter split(line, " ");

		// get names of object files
		while (fileStream.good())
		{
			getline(fileStream, line);
			split.reset(line, " ");
			string tmpFileName = split[0];
			if (tmpFileName.back() == '\r')
			{
				tmpFileName.pop_back();
			}

			objectFileNames.push_back(tmpFileName);
		}
	}
	else
	{
		cout << "\nCouldn't read file\n";
		exit(1);
	}

	return objectFileNames;
}

void loadObjectFromFile(string objFileName)
{
//	static int offset = 0;
	static int index = 0;

	ifstream fileStream(objFileName);
	string line;

	fileStream.clear();
	if (fileStream.is_open())
	{
		getline(fileStream, line);
		Splitter split(line, " ");

		vec4 minValues;
		vec4 maxValues;

		// ignore comment lines
		while (split[0].c_str()[0] == '#')
		{
			if (split[0].compare("#\tRange") == 0)
			{
				string value = split[2];
				value.pop_back();
				value.erase(0, 1);
				minValues.x = atof(value.c_str());

				value = split[3];
				value.pop_back();
				minValues.y = atof(value.c_str());

				value = split[4];
				value.pop_back();
				minValues.z = atof(value.c_str());

				value = split[6];
				value.pop_back();
				value.erase(0, 1);
				maxValues.x = atof(value.c_str());

				value = split[7];
				value.pop_back();
				maxValues.y = atof(value.c_str());

				value = split[8];
				value.pop_back();
				maxValues.z = atof(value.c_str());

				minValues.w = maxValues.w = 1.0;
			}
			getline(fileStream, line);
			split.reset(line, " ");
		}

		while (fileStream.good())
		{
			vertexStore.push_back(vector<point4>());
			vertices.push_back(vector<point4>());

			// get vertex info
			while (split[0].compare("v") == 0)
			{
				point4 vertex = point4(atof(split[1].c_str()), atof(split[2].c_str()), atof(split[3].c_str()), 1.0);
				normalizeVector(&vertex, minValues, maxValues);

//				vertex.x -= index;
				vertexStore[index].push_back(vertex);
				getline(fileStream, line);
				split.reset(line, " ");
			}

			normalStore.push_back(vector<vec4>());
			normals.push_back(vector<vec4>());
			// get normals
			while (split[0].compare("vn") == 0)
			{
				point4 normal = vec4(atof(split[1].c_str()), atof(split[2].c_str()), atof(split[3].c_str()), 1.0);
				normalizeVector(&normal, minValues, maxValues);
//				normal.x -= index;
				normalStore[index].push_back(normal);
				getline(fileStream, line);
				split.reset(line, " ");
			}

			while (split[0].compare("f") == 0)
			{
				// extract index values for faces
				point4 vertexIndices;
				vec4 normalIndices;

				Splitter slashSplitter(split[1], "//");
				vertexIndices.x = atof(slashSplitter[0].c_str());
				normalIndices.x = atof(slashSplitter[1].c_str());

				slashSplitter.reset(split[2], "//");
				vertexIndices.y = atof(slashSplitter[0].c_str());
				normalIndices.y = atof(slashSplitter[1].c_str());

				slashSplitter.reset(split[3], "//");
				vertexIndices.z = atof(slashSplitter[0].c_str());
				normalIndices.z = atof(slashSplitter[1].c_str());

				addTri(vertexIndices.x, vertexIndices.y, vertexIndices.z, normalIndices.x, normalIndices.y, normalIndices.z, index);

				getline(fileStream, line);
				split.reset(line, " ");
			}
		}

		// add axis line end cap cubes
		addCube( vec3(-1.0, 0.0, 0.0), .1);
		addCube( vec3(1.0, 0.0, 0.0), .1);
		addCube( vec3(0.0, -1.0, 0.0), .1);
		addCube( vec3(0.0, 1.0, 0.0), .1);
		addCube( vec3(0.0, 0.0, -1.0), .1);
		addCube( vec3(0.0, 0.0, 1.0), .1);


		// add axis lines
		addLine(vec4(-1.0, 0.0, 0.0, 1.0), vec4(1.0, 0.0, 0.0, 1.0));
		addLine(vec4(0.0, -1.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0));
		addLine(vec4(0.0, 0.0, -1.0, 1.0), vec4(0.0, 0.0, 1.0, 1.0));

		index++;

	}
	else
	{
		cout << "\nCouldn't read file " << objFileName << endl;
		exit(1);
	}

}


void normalizeVector(vec4 *vector, vec4 min, vec4 max)
{
//	(*vector).x = ((*vector).x - min.x) / (max.x - min.x);
//	(*vector).y = ((*vector).y - min.y) / (max.y - min.y);
//	(*vector).z = ((*vector).z - min.z) / (max.z - min.z);
}

