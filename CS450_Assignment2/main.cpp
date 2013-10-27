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

typedef Angel::vec4  color4;
typedef Angel::vec3  point3;
const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  projection; // projection matrix uniform shader variable location

vector<point3> vertices;
vector<point3> points;
vector<vec3>   normals;

#pragma mark Function declarations
vector<string> readSceneFile(string fileName);
void loadObjectFromFile(string objFileName);

#pragma mark -


//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices.  Notice we keep the relative ordering when constructing the tris
void addTri( int pointA, int pointB, int pointC, int normalA, int normalB, int normalC )
{
	points.push_back(point3(pointA, pointB, pointC));
	normals.push_back(vec3(normalA, normalB, normalC));
}

//----------------------------------------------------------------------------

// OpenGL initialization
void init()
{
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, points.size()*sizeof(point3), &points[0] );
    glBufferSubData( GL_ARRAY_BUFFER, points.size()*sizeof(point3), sizeof(normals), &normals[0] );

    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram( program );

    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

    GLuint vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );


    // Initialize shader lighting parameters
    // RAM: No need to change these...we'll learn about the details when we
    // cover Illumination and Shading
    vec4 light_position( 1.5, 0.5, 2.0, 1.0 );
    color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
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



    mat4 p = Perspective(45, 1.0, 0.1, 10.0);
    vec4  eye( 1.0, 1.0, 2.0, 1.0);
    vec4  at( 0.0, 0.0, 0.0, 1.0 );
    vec4  up( 0.0, 1.0, 0.0, 0.0 );


    mat4  mv = LookAt( eye, at, up );
    //vec4 v = vec4(0.0, 0.0, 1.0, 1.0);

    glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );
    glUniformMatrix4fv( projection, 1, GL_TRUE, p );


    glEnable( GL_DEPTH_TEST );
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

//----------------------------------------------------------------------------

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033:  // Escape key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    }
}

//----------------------------------------------------------------------------


int main(int argc, char** argv)
{
	string sceneFileName;
	vector<string> objectFileNames;

	if (argc > 1)
		sceneFileName = argv[1];
	else
		sceneFileName = "test.scn";

	objectFileNames = readSceneFile(sceneFileName);

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
    glutInitWindowSize(512, 512);
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
			objectFileNames.push_back(split[0]);
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
	ifstream fileStream(objFileName);
	string line;

	fileStream.clear();
	if (fileStream.is_open())
	{
		getline(fileStream, line);
		Splitter split(line, " ");

		// ignore comment lines
		while (split[0].c_str()[0] == '#')
		{
			getline(fileStream, line);
			split.reset(line, " ");
		}

		while (fileStream.good())
		{
			// get vertex info
			while (split[0].compare("v") == 0)
			{
				vertices.push_back(point3(atof(split[1].c_str()), atof(split[2].c_str()), atof(split[3].c_str())));
				getline(fileStream, line);
				split.reset(line, " ");
			}

			// get normals
			while (split[0].compare("vn") == 0)
			{
				normals.push_back(point3(atof(split[1].c_str()), atof(split[2].c_str()), atof(split[3].c_str())));
				getline(fileStream, line);
				split.reset(line, " ");
			}

			while (split[0].compare("f") == 0)
			{
				// extract index values for faces
				vec3 vertexIndices;
				vec3 normalIndices;

				Splitter slashSplitter(split[1], "//");
				vertexIndices.x = atof(slashSplitter[0].c_str());
				normalIndices.x = atof(slashSplitter[1].c_str());

				slashSplitter.reset(split[2], "//");
				vertexIndices.y = atof(slashSplitter[0].c_str());
				normalIndices.y = atof(slashSplitter[1].c_str());

				slashSplitter.reset(split[3], "//");
				vertexIndices.z = atof(slashSplitter[0].c_str());
				normalIndices.z = atof(slashSplitter[1].c_str());

				addTri(vertexIndices.x, vertexIndices.y, vertexIndices.z, normalIndices.x, normalIndices.y, normalIndices.z);

				getline(fileStream, line);
				split.reset(line, " ");
			}
		}
	}
	else
	{
		cout << "\nCouldn't read file\n";
		exit(1);
	}

}

