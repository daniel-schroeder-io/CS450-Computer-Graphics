#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include "glew.h"
#endif


#include <OpenGL/gl3.h>

#include "glslprogram.h"
#include <OpenGL/gl.h>

#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include "bmp.cpp"
#include "loadTGA.cpp"
#include "mjbsphere.cpp"
//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// NOTE: There are a lot of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.


// title of these windows:

const char *WINDOWTITLE = { "Project 6 Geometric Modeling - Daniel Schroeder" };
const char *GLUITITLE   = { "User Interface Window" };


// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE		0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// size of the box:

const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// minimum allowable scale factor:

const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


// which projection:

enum Projections
{
	ORTHO,
	PERSP
};


// which button:

enum ButtonVals
{
	RESET,
	QUIT
};


// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };


// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };


// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char * ColorNames[ ] =
{
	"Red",
	"Yellow",
	"Green",
	"Cyan",
	"Blue",
	"Magenta",
	"White",
	"Black"
};


// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};

const GLfloat NeutralColors[ ][3] =
		{
				{ 0.71484375, 0.6484375, 0.67578125},	// purple
				{ 0.7890625, 0.69921875, 0.53125 },		// gold/bronze
				{ 0.70703125, 0.6953125, 0.5703125 },	// green
				{ 0.65625, 0.67578125, 0.703125 },		// blue
				{ 0.85546875, 0.83984375, 0.796875 },	// silver
		};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to use the z-buffer
GLuint	BoxList;				// object display list
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees


// function prototypes:
void 	drawNormal( float normal[3], float vertex[3]);
void    drawSkydome( float radius, int lats, int longs );
void    drawStrip(int length );
void 	moveForward( float camera[3], float lookat[3] );
void 	moveBackward( float camera[3], float lookat[3] );
void 	drawGround( );
void 	drawSky( );
void    drawClouds( );
float 	Unit( float vin[3], float vout[3] );
float	Dot( float v1[3], float v2[3]);
void 	drawBuilding(float width, float height , const GLfloat window[3]);
void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void	Axes( float );
void	HsvRgb( float[3], float [3] );


//---------------------------------------------------
//Final Program
//---------------------------------------------------

#define TIME_VARIABLE 1000
float Time;
GLuint	circleList;
float cameraVector[] = {0., 1., -2};
float lookatVector[] = {0., 1., 0.};
int freeze = 0;
int drawControlPoints = 1;
int drawControlLines = 1;
int showHead = 1;
int showCluster = 0;
int lookatAngle = 90;

//Textures//
unsigned char *texture;
int texture_w, texture_h;
unsigned char *texture2;
int texture2_w, texture2_h;


//Shaders//
GLSLProgram *Building;
GLSLProgram *Sky;
GLSLProgram *Clouds;
float pat = 0;
float FRAGMENT = 1.;
float FREEZE = 1.;
float PATTERN = 1.;
const float DEG2RAD = 3.14159/180;

#define noiseWidth 20
#define noiseHeight 20
double noise[noiseHeight][noiseWidth];
// main program:
void generateNoise();
double smoothNoise(double x, double y);
double turbulence(double x, double y, double size);
void	HsvRgb( float[3], float [3] );


int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );


	// setup all the graphics stuff:

	InitGraphics( );


	// create the display structures that will not change:

	InitLists( );


	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );


	// setup all the user interface stuff:

	InitMenus( );


	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );


	// this is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate( )
{
	glutSetWindow( MainWindow );
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:

	int ms = glutGet( GLUT_ELAPSED_TIME );	// milliseconds

	ms  %=  (TIME_VARIABLE);
	if (freeze == 0) {
		Time = (float) ms / (float) TIME_VARIABLE;        // [ 0., 1. )
		// force a call to Display( ) next time it is convenient:
		Time = (int) (Time * 360) * M_PI / 180.0;
	}
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}


	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );


	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	if( DepthBufferOn != 0 )
		glEnable( GL_DEPTH_TEST );
	else
		glDisable( GL_DEPTH_TEST );


	// specify shading to be flat:

	glShadeModel( GL_FLAT );


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( WhichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
	else
		gluPerspective( 90., 1.,	0.1, 1000. );


	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );


	// set the eye position, look-at position, and up-vector:

	gluLookAt( cameraVector[0], cameraVector[1], cameraVector[2],
			   lookatVector[0], lookatVector[1], lookatVector[2],
			   0., 1., 0.);


	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );


	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );


	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}


	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[WhichColor][0] );
		glCallList( AxesList );
	}

	glEnable( GL_NORMALIZE );

	// since we are using glScalef( ), be sure normals get unitized:



		// draw the current object:




	glPushMatrix( );
		glTranslatef( 5., 0., 0. );
		drawBuilding(3., 8., NeutralColors[1]);
	glPopMatrix( );

	glPushMatrix( );
		glTranslatef( 0., 0., 5. );
		drawBuilding(3., 12., NeutralColors[3]);
	glPopMatrix( );
    glPushMatrix( );
        glTranslatef( -5., 0., 0. );
        drawBuilding(3., 10., NeutralColors[4]);
    glPopMatrix( );
    glPushMatrix( );
        glTranslatef( 5., 0., 5. );
        drawBuilding(3., 6., NeutralColors[0]);
    glPopMatrix( );
    glPushMatrix( );
        glTranslatef( 0., 0., -10. );
        drawBuilding(3., 16., NeutralColors[2]);
    glPopMatrix( );



    /*
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, texture2_w, texture2_h, 0, GL_RGB, GL_UNSIGNED_BYTE, texture2);
    glDisable(GL_TEXTURE_2D);
    */
    glPushMatrix( );
		drawSky( );
	glPopMatrix( );
	glPushMatrix( );
	glPopMatrix( );

	drawGround( );
	/*

	*/
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.,   0., 1., 0. );
			glCallList( BoxList );
		glPopMatrix( );
	}
	/*

	*/

	// draw some gratuitous text that just rotates on top of the scene:

	/* draw some gratuitous text that is fixed on the screen:
	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0., 100.,     0., 100. );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1., 1., 1. );
	DoRasterString( 28., 25., 0., "FLOCK OF SEAGULLS" );
	 */
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates


	// swap the double-buffered framebuffers:

	glutSwapBuffers( );


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	WhichColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	WhichProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(int) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Colors",        colormenu);
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics( )
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );
	glutIdleFunc( Animate );

	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	//init texture
	texture = BmpToTexture("stone_wall.bmp", &texture_w, &texture_h);
	texture2 = BmpToTexture("sky.bmp", &texture2_w, &texture2_h);


	//init shader
	//init shaders
	Building = new GLSLProgram( );
	bool buildingValid = Building->GLSLProgram::Create( "building.vert",  "building.frag" );
	if( ! buildingValid )
	{
		fprintf( stderr, "Shader cannot be created!\n" );
		DoMainMenu( QUIT );
	}
	else
	{
		fprintf( stderr, "Shader created.\n" );
	}
	Building->GLSLProgram::SetVerbose( false );


    drawStrip(5);


	Sky = new GLSLProgram( );
	bool skyValid = Sky->GLSLProgram::Create( "sky.vert",  "sky.frag" );
	if( ! skyValid )
	{
		fprintf( stderr, "Shader cannot be created!\n" );
		DoMainMenu( QUIT );
	}
	else
	{
		fprintf( stderr, "Shader created.\n" );
	}
	Sky->GLSLProgram::SetVerbose( false );

    Clouds = new GLSLProgram( );
    bool cloudsValid = Clouds->GLSLProgram::Create( "cloud.vert",  "cloud.frag" );
    if( ! cloudsValid )
    {
        fprintf( stderr, "Shader cannot be created!\n" );
        DoMainMenu( QUIT );
    }
    else
    {
        fprintf( stderr, "Shader created.\n" );
    }
    Clouds->GLSLProgram::SetVerbose( false );

}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( ) {
    float dx = BOXSIZE / 2.f;
    float dy = BOXSIZE / 2.f;
    float dz = BOXSIZE / 2.f;
    glutSetWindow(MainWindow);

    // create the object:

    BoxList = glGenLists(1);
    glNewList(BoxList, GL_COMPILE);


    glEndList();

    circleList = glGenLists(1);
    glNewList(circleList, GL_COMPILE);
    float x, y;
    float radius = 6.;
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);

    x = (float) radius * cos(359 * M_PI / 180.0f);
    y = (float) radius * sin(359 * M_PI / 180.0f);
    for (int j = 0; j < 360; j++) {
        glVertex2f(x, y);
        x = (float) radius * cos(j * M_PI / 180.0f);
        y = (float) radius * sin(j * M_PI / 180.0f);
        glVertex2f(x, y);
    }
    glEnd();
    glEndList();
    // create the axes:

    AxesList = glGenLists(1);
    glNewList(AxesList, GL_COMPILE);
    glLineWidth(AXES_WIDTH);
    Axes(1.5);
    glLineWidth(1.);
    glEndList();
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	float looky;
	float lookx;
	float a;
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			WhichProjection = ORTHO;
			break;
		case 'k':
		case 'K':
			if (drawControlPoints == 1)
				drawControlPoints = 0;
			else
				drawControlPoints = 1;
			break;
		case 'l':
		case 'L':
			if (drawControlLines == 1)
				drawControlLines = 0;
			else
				drawControlLines = 1;
			break;
		case 'f':
		case 'F':
			if ( freeze == 0 )
				freeze = 1;
			else
				freeze = 0;
			break;
		case 'h':
		case 'H':
			if ( showHead == 0 )
				showHead = 1;
			else
				showHead = 0;
			break;
		case '1':
			if ( showCluster == 0 )
				showCluster = 1;
			else
				showCluster = 0;
			break;
		case 'p':
		case 'P':
			WhichProjection = PERSP;
			break;
		case 'w':
		case 'W':
			moveForward(cameraVector, lookatVector);

			break;
		case 'a':
		case 'A':
			lookatAngle -= 15;
			a = lookatAngle * (M_PI/180);
			lookatVector[0] = cameraVector[0] + cos(a);
			lookatVector[2] = cameraVector[2] + sin(a);
			break;
		case 's':
		case 'S':

			moveBackward(cameraVector, lookatVector);
			break;
		case 'd':
		case 'D':
			lookatAngle += 15;
			a = lookatAngle * (M_PI/180);

			lookatVector[0] = cameraVector[0] + cos(a);
			lookatVector[2] = cameraVector[2] + sin(a);
			break;

		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}


	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );


	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}


	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:
		printf("%f\n", Scale);
		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[ ] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[ ] = {
		1, 2, -3, 4
		};

static float yx[ ] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[ ] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[ ] = {
		1, 2, 3, -2, 4
		};

static float zx[ ] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[ ] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[ ] = {
		1, 2, 3, 4, -5, 6
		};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r, g, b;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
drawBuilding(float width, float height , const GLfloat window[3]){
    Building->Use();
    Building->SetUniformVariable((char *) "uniformTime", (float) (abs(sin(DEG2RAD * (int) (Time * 3.6)) / 2)));

    Building->SetUniformVariable((char *) "uPat", (float) (pat));
    Building->SetUniformVariable((char *) "uKa", (float) 0.25);
    Building->SetUniformVariable((char *) "uKd", (float) .5);
    Building->SetUniformVariable((char *) "uKs", (float) .25);
    Building->SetUniformVariable((char *) "uShininess", (float) 1);
    Building->SetUniformVariable((char *) "uS0", 1);
    Building->SetUniformVariable((char *) "uT0", 1);
    Building->SetUniformVariable((char *) "uSize", (float) 1);
	Building->SetUniformVariable((char *) "r", (float) window[0]);
	Building->SetUniformVariable((char *) "g", (float) window[1]);
	Building->SetUniformVariable((char *) "b", (float) window[2]);
	Building->SetUniformVariable((char *) "height", (float) height );
	Building->SetUniformVariable((char *) "width", (float) width );
	float y = height;
	float x = width/2;
	float z = width/2;

	int i;
	for ( i = 0; i < 4; i++){
		glPushMatrix( );
			glRotatef(90*i, 0., 1., 0.);
			glBegin( GL_QUADS );
				glColor3f( 0., 0., 1. );
				glNormal3f( 0., 0.,  1. );
				glTexCoord2f( 0, 0 );
				glVertex3f( -x, 0,  z );
				glTexCoord2f( 0, 24 );
				glVertex3f( -x, y,  z );
				glTexCoord2f( 12, 24 );
				glVertex3f(  x,  y,  z );
				glTexCoord2f(12, 0 );
				glVertex3f( x,  0,  z );
			glEnd( );
		glPopMatrix( );
	}

	/*
	glColor3f(1., 0., 0.);
	glPushMatrix( );
		glBegin( GL_QUADS );
			glNormal3f( 0., 0.,  1. );
			glVertex3f( -x, 0,  z );
			glVertex3f(  x, 0,  z );
			glVertex3f(  x,  0,  -z );
			glVertex3f( -x,  0,  -z );
		glEnd( );
	glPopMatrix( );

	glPushMatrix( );
		glTranslatef(0, height, 0);
		glBegin( GL_QUADS );
			glNormal3f( 0., 0.,  1. );
			glVertex3f( -x, 0,  z );
			glVertex3f(  x, 0,  z );
			glVertex3f(  x,  0,  -z );
			glVertex3f( -x,  0,  -z );
		glEnd( );
	glPopMatrix( );
	*/
    Building->Use(0);
}


float
Dot( float v1[3], float v2[3] )
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

float
Unit( float vin[3], float vout[3] )
{
	float dist = vin[0]*vin[0] + vin[1]*vin[1] + vin[2]*vin[2];
	if( dist > 0.0 )
	{
		dist = sqrt( dist );
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}

void
moveForward( float camera[3], float lookat[3] ){

	float a = lookatAngle * (M_PI/180);

	camera[0] = lookat[0];
	camera[2] = lookat[2];
	lookat[0] = camera[0] + cos(a);
	lookat[2] = camera[2] + sin(a);
}
void
moveBackward( float camera[3], float lookat[3] ){

	float a = lookatAngle * (M_PI/180);

	float dx = lookat[0] - camera[0];
	float dz = lookat[2] - camera[2];

	camera[0] -= dx;
	camera[2] -= dz;
	lookat[0] = camera[0] + cos(a);
	lookat[2] = camera[2] + sin(a);

}


void drawGround( ){
	glPushMatrix( );

		glBegin( GL_QUADS );
			glColor3f( 0.09765625, 0.43359375, 0.23828125 );
			glNormal3f( 0., 0.,  1. );
			glTexCoord2f( 0, 0 );
			glVertex3f( -100, 0, 100);
			glTexCoord2f( 0, 24 );
			glVertex3f( 100, 0, 100);
			glTexCoord2f( 12, 24 );
			glVertex3f( 100, 0, -100);
			glTexCoord2f( 12, 0 );
			glVertex3f( -100, 0, -100);
		glEnd( );
	glPopMatrix( );
}
void drawSky( ) {


    glPushMatrix();
    	glRotatef(90, 1, 0, 0);
    	glScalef(20, 20, 20);
		drawSkydome(2, 50, 50);
	glPopMatrix( );

}

void drawStrip(int length ){
    int i;
    for ( i = 0; i < length; i++){
        //build road
        glPushMatrix();
        glTranslatef(i, 0.01, 0);
        glBegin( GL_QUADS );
            glColor3f( 0., 0., 0. );
            glNormal3f( 0., 1.,  0. );
            glTexCoord2f( 0, 0 );
            glVertex3f( 0, 0, 0);
            glTexCoord2f( 0, 1 );
            glVertex3f( 0, 0, 1);
            glTexCoord2f( 1, 1 );
            glVertex3f( 1, 0, 1);
            glTexCoord2f( 1, 0 );
            glVertex3f( 1, 0, 0);
        glEnd( );
        int randLeft = rand()%2;
        printf("%d\n", randLeft);
        glPopMatrix();

    }
}

void drawSkydome( float radius, int lats, int longs ) {
	Clouds->Use();
	float mouse[2] = {Xrot, Yrot};
	float res[2] = {600, 600};
	Clouds->SetUniformVariable((char *) "eyex", (float) cameraVector[0]);
	Clouds->SetUniformVariable((char *) "eyey", (float) cameraVector[1]);
	Clouds->SetUniformVariable((char *) "eyez", (float) cameraVector[2]);
	Clouds->SetUniformVariable((char *) "u_resolution_x", (float) res[0]);
	Clouds->SetUniformVariable((char *) "u_resolution_y", (float) res[1]);
	Clouds->SetUniformVariable((char *) "u_mouse_x", (float) mouse[0]);
	Clouds->SetUniformVariable((char *) "u_mouse_y", (float) mouse[1]);
	Clouds->SetUniformVariable((char *) "u_time", (float) (abs(sin(DEG2RAD * (int) (Time * 3.6)) / 2)));

	int i, j;
    int halfLats = lats / 2;
    for(i = 0; i <= halfLats; i++)
    {
        double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
        double z0 = sin(lat0);
        double zr0 = cos(lat0);

        double lat1 = M_PI * (-0.5 + (double) i / lats);
        double z1 = sin(lat1);
        double zr1 = cos(lat1);

        glBegin(GL_TRIANGLE_STRIP);
        for(j = 0; j <= longs; j++)
        {
            double lng = 2 * M_PI * (double) (j - 1) / longs;
            double x = cos(lng);
            double y = sin(lng);

            double s1, s2, t;
            s1 = ((double) i) / halfLats;
            s2 = ((double) i + 1) / halfLats;
            t = ((double) j) / longs;

            glTexCoord2d(s1, t);
            glNormal3d(x * zr0, y * zr0, z0);

            glVertex3d(radius * x * zr0,radius * y * zr0, radius * z0);

            glTexCoord2d(s2, t);
            glNormal3d(x * zr1, y * zr1, z1);
            glVertex3d(radius * x * zr1, radius * y * zr1, radius * z1);
        }
        glEnd();
    }
	Clouds->Use(0);


}
void drawNormal( float normal[3], float vertex[3]){




}