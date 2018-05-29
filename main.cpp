//Copyright 2017-2018 Christian Nuckols

#include <iostream>
#include <gl/glut.h>
#include <gl/freeglut.h>
#include <stdio.h>     //For format strings


GLfloat windowWidth = 800.0;			 //16:9 aspect ratio 
GLfloat windowHeight = 450.0;
const GLfloat rotationIncrement = 1.0;   //Number of degrees each teapot rotates per frame
const GLfloat translationIncrement = .01;//Units teapots are translated per frame
static GLint frameCount = 0;			 //Each call to the display function increments this value
static GLfloat row1XPosition[5] = { 1.6, 3.2, 4.8, 6.4, 7.99 }; //Starting positions of the odd-numbered rows (row 1 & 2)
static GLfloat row2XPosition[5] = { 1.6, 3.2, 4.8, 6.4, 7.99 };	//Starting positions of the even-numbered row (row 2)
static float startRotation[5] = { 60.0, 180.0, 300.0, 120.0, 240.0 };

 //COLORS
//Colors are defined here for neatness and readibility within the 'drawRows' function. 
static float red[3] = { 1, 0, 0 };
static float green[3] = { 0, 1, 0 };
static float blue[3] = { 0, 0, 1 };
static float magenta[3] = { 1, 0, 1 };
static float cyan[3] = { 0, 1, 2 };
static float yellow[3] = { 1, 1, 0 };
static float orange[3] = { 1, .5, 0 };
static float pink[3] = { 1, .7, .7 };
static float purple[3] = { .9, .5 , .9 };
static float darkRed[3] = { .5, 0, 0 };
static float darkGreen[3] = { 0, .5, 0 };
static float darkBlue[3] = { 0, 0, .3 };//Make dark-blue darker than dark-green and dark-red to distinguish it from purple.
static float mediumGrey[3] = { .5, .5, .5 };
static float darkGrey[3] = { .9, .9, .9 };
static float lightGrey[3] = { .05, .05, .05 };

void *font = GLUT_BITMAP_TIMES_ROMAN_24;//Set font here
int teapotCount;    //Keeps track of the teapot count to determine state of the game and text string
int ballCount;		//Keep track of the number of cannonballs on the screen. Up to 5 are allowed.
int xTrajectory[5]; //Stores the x-axis angle of the cannonball(s)' trajectory
int yTrajectory[5];
float unitsTravelled[5];//Stores the number of units each cannonball has travelled.
//RGBpixmap pixMap;   //Initialize a picture map object defined in the header file. 
char text [100];    //This will store the text at the top of the screen
GLfloat cannonYaw;  //Stores the value for the cannon yaw (y-axis/left-right cannon movement)
GLfloat cannonPitch;//Stores the value for the cannon pitch (x-axis/up-down cannon movement)
bool isTeapot[15];  //Stores a boolean value for each teapot to determine if it should be rendered.
					//They are in order of rows, from left to right starting positions, bottom to top.
					//isTeapot[0] is the left bottom, isTeapot[7] is the third teapot in the middle row
					//isTeapot[14] is the rightmost teapot on the top row and so on.

//Simple function for displaying text. Accepts a c-string and displays it near the 
//top and center of the screen.
void displayText(const char *str)
{
	glDisable(GL_LIGHTING);
	glColor3d(0.9, 0.9, 1.0); //Set font color to near white.
	glRasterPos3f(0, 0, 0);//This must be defined to position the glutBitmapCharacter function.
	while (*str) {
		glutBitmapCharacter(font, *str);
		str++;
	}
	glEnable(GL_LIGHTING);
}

//Determines the current state of the game to determine the content of the test. This function 
//is called when a collision occurs, upon initialization, during a new game, or when the game is won. 
void setText(char * str) {
	if (teapotCount) {//If there are any teapots, the game is in progress and the message
		{ snprintf(str, 100, "Teapots Remaining: %d", teapotCount); }
	}
	else { snprintf(str, 100, "    Continue (Y/N)?"); }
}

void initialize() {
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);		//Set to light 0 (the only one)
	glShadeModel(GL_SMOOTH);	//Smooth, as opposed to flat (1-color-per-face) shading. 
	glEnable(GL_COLOR_MATERIAL);//Must be enable to allow color AND material properties in conjunction.  
	glEnable(GL_DEPTH_TEST);    //Must be enabled to compare depth.
	glEnable(GL_NORMALIZE);		//Enable normal vector-based shading.

	//TEXTURE INITIALIZATION
	//pixMap.readBMPFile("tile.bmp");//Load the bitmap file into the pixmap 
	//pixMap.setTexture(1);          //Set to texture named '1' (must be an int)

	//SET LIGHT AND MATERIAL QUALITIES
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f }; //Ambient light.  Equal RGB values don't give any color precedence
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);	//and low-value colors make the ambience weak for more dramatic shading
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 0.8f };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 0.4f };//Neutral-colored, relatively strong specular light. 
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	GLfloat shininess[] = { 70.0f };				//Very shiny teapots. 
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
	//Light source properties
	GLfloat light0_position[] = { 7.0f, 2.0f, 10.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	GLfloat light0_Intensity[] = { 0.2f, 0.4f, 0.4f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_Intensity);

	//CAMERA/PERSPECTIVE PROPERTIES
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, windowWidth / windowHeight, .1, 50);
	glMatrixMode(GL_MODELVIEW);
	//Camera position
	gluLookAt(4.0, 2.5, 10.0, //Camera placement
		      4.0, 2.5, 0.0,  //Looking at the center
	          0.0, 1.0, 0.0);  //Keep y-axis up for simplicity
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//INITIALIZE GAMEPLAY VARIABLES
	teapotCount = 15;//Set the teapot count to 15.  Will be reset 
					 //if the player chooses to play again.
	setText(text);   //Initialize the text to be displayed
	for (int c = 0; c < 15; c++) {	isTeapot[c] = true; }//Set to true to render every teapot. 
}

//This function renders the cannon.  The pitch and yaw are determined by the keyboard function's
//reading of the arrow keys.  The arrow key simply needs to be pressed to tilt the cannon.
void drawCannon() {
	glColor3fv(red);//Set the cannon color
	glPushMatrix();
	glTranslatef(4.0, 1.6, 7.0);//Place the cannon in the center of the screen
	glRotated(180.0 + cannonPitch, 1, 0, 0);  //The cannon's PITCH
	//NOTE, for the sake of simplicity, all the angles (of the cannon and the balls) are 
	//rendered 180 degrees on + the pitch.  This is so the distance travelled by the cannonballs
	//can remain positive, since a base-angle of 0 degrees would result a vector than proceeds
	//towards the viewer (the glutSolidCylinder function renders the cylinder like a can lying
	//on its side).  
	glRotated(cannonYaw, 0, 1, 0);  //The cannon's YAW
	glutSolidCylinder(0.1, 1, 20, 20);
	glPopMatrix();
}

//COLLISION DETECTION
//This function detects collisions.  Only one cannonball is tested per function call.  
//The x,y, and z position are and the ball index are sent (if the third-furthest ball from
//the viewer is a potential collision, its coordinates and the index of 2 will be passed). 
void detectCollisions(double x, double y, double z, int ballNumber) {
	
	//The outer for-loop interates the horizontal positions of the balls.
	for (int c = 0; c < 5; c++) {
		//BOTTOM ROW
		if (isTeapot[c])  {//First, make sure the teapot exists.
		//If it does, test the x-position of the teapot relative to 
		//the position of the ball being tested. 
			if (row1XPosition[c] - x < .3 && row1XPosition[c] - x > -.3) {
			//Then test the y-position of the teapot relative to the ball.
				if (y > .95  && y < 1.55) {
					isTeapot[c] = false;//Delete teapot
					teapotCount--;//Lower teapot count
					setText(text);//Update the text
					//The following for-loop updates the positions of balls prior to 
					//lowering the ballCount.
					for (int c3 = ballNumber; c3 < ballCount - 1; c3++) {
						unitsTravelled[c3] = unitsTravelled[c3 + 1];
						xTrajectory[c3] = xTrajectory[c3 + 1];
						yTrajectory[c3] = yTrajectory[c3 + 1];
					}
					ballCount--;//Lower the ballCount. Now the ball that collided with
					//the teapot will not be rendered in the next call of 'drawBalls.'
				 }
			}
		}//END OF BOTTOM ROW 

		//MIDDLE ROW
		if (isTeapot[5 + c]) {//Iterates teapot row 2.  The logic is identical to the 
			//above detection logic, except the positions are taken from the middle row.
			if (row2XPosition[c] - x < .3 && row2XPosition[c] - x > -.3) {
				if (y >(2.2) && y < (2.8)) {
					isTeapot[5 + c] = false;
					teapotCount--;
					setText(text);
					for (int c3 = ballNumber; c3 < ballCount - 1; c3++){
						unitsTravelled[c3] = unitsTravelled[c3 + 1];
						xTrajectory[c3] = xTrajectory[c3 + 1];
						yTrajectory[c3] = yTrajectory[c3 + 1];
					}
					ballCount--;
				}
			}
		}//END OF MIDDLE ROW
		
		//TOP ROW
		if (isTeapot[c + 10]) {
			//The teapot positions of the top and bottom row are the same, so in the
			//if-statement below, test for 'c' rather than 'c + 10.'
			if (row1XPosition[c] - x < .3 && row1XPosition[c] - x > -.3) {
				//Then test the y-position of the teapot relative to the ball.
				if (y > 3.45  && y < 4.05) {
					isTeapot[c + 10] = false;//Delete teapot
					teapotCount--;//Lower teapot count
					setText(text);//Update the text
								  //The following for-loop updates the positions of balls prior to 
								  //lowering the ballCount.
					for (int c3 = ballNumber; c3 < ballCount - 1; c3++) {
						unitsTravelled[c3] = unitsTravelled[c3 + 1];
						xTrajectory[c3] = xTrajectory[c3 + 1];
						yTrajectory[c3] = yTrajectory[c3 + 1];
					}
					ballCount--;//Lower the ballCount. Now the ball that collided with
								//the teapot will not be rendered in the next call of 'drawBalls.'
				}
			}
		}//END OF TOP ROW 
	}
}

//drawBalls is called from the display function when the ballCount variable is greater than one.
//This function also calls the 'detectCollisions' function, which is used to determine whether or
//not a cannonball is colliding with a teapot. 
void drawBalls() {
	int c;//counter variable

	//The following values store the relative positions of the each ball.
	//These are used as parameters for the collision detection function or
	//to test if a ball is past a wall and can be removed
	double xCenter[5];
	double yCenter[5];
	double zCenter[5];

	for (c = 0; c < ballCount; c++) {
		glPushMatrix();
		glColor3f(0.0, 0.0, 0.0);
		glTranslatef(4.0, 1.6, 7.0);       //Generate at the end of the cannon
		glRotated(xTrajectory[c], 1, 0, 0);//Set the x trajectory 
		glRotated(yTrajectory[c], 0, 1, 0);//Set the y trajectory 
		unitsTravelled[c] += .2;//Increment the distance the ball has travelled
		glTranslatef(0.0, 0.0, unitsTravelled[c]);//Translate the ball along the rotated vector.
		glutSolidSphere(.1, 7, 7);
		
		//The following function gets the modelview position of ball being drawn.
		GLfloat matrix[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
		//The camera is positioned at 4, 2.5, 10, so add these values to each
		//point to get the relative position of the ball to scene. 
		xCenter[c] = 4.0 + matrix[12];
		yCenter[c] = 2.5 + matrix[13];
		zCenter[c] = 10 + matrix[14];
		//The following if statement determines whether or not a collision should be tested.
		//Because the teapots always stay within the same z-range, the 'detectCollisions' function
		//is only called if the center of the balls is within the range of the teapot detection bounds.
		//Then, for efficiency, only the three coordinate points of the current ball are passed, along
		//with the number of the ball being sent.  If a collision is detected, that ball will be 
		//deleted.  
		if (zCenter[c] >= 2.2 && zCenter[c] <= 2.8) { detectCollisions(xCenter[c], yCenter[c], zCenter[c], c); }
		glPopMatrix();


	}
	
	//TEST TO SEE IF THE BALL IS FAR AWAY ENOUGH TO BE REMOVED 
	//If any wall boundary is passed by any cannonball, it can be removed
	for (c = 0; c < ballCount; c++){
		if (xCenter[c] < -0.2 || xCenter[c] > 8.2 ||//Left and right walls (0.0x and 8.0x)
			yCenter[c] < -0.2 || yCenter[c] > 5.2 ||//Top and bottom walls (0.0y and 5.0y)
			zCenter[c] < -1.9) { //Back wall at -1.7 z
			for (int c2 = 0; c2 < ballCount - 1; c2++) {//Move each value one position down the array
				unitsTravelled[c2] = unitsTravelled[c2 + 1];
				xTrajectory[c2] = xTrajectory[c2 + 1];
				yTrajectory[c2] = yTrajectory[c2 + 1];
			}
			ballCount--;
		}
	}

}

void fireCannon() {//This fires the cannon.  
	ballCount++; //Update the ballCount variable.
	xTrajectory[ballCount - 1] = 180 + cannonPitch;//Save the current position of the cannon
	yTrajectory[ballCount - 1] = cannonYaw;        //to keep the balls trajectory.
	unitsTravelled[ballCount - 1] = 0.0; //This value will increment each time the ball is rendered.

}

//The function that actually calls the 'glutTeapot' function to render the teapot.  The 'rowNumber' parameter
//is used to determine which direction the teapots will move in translation and which color each teapot will be 
//rendered.  The color is also dependent on the counter in the for loop (c), so each teapot is uniquely colored. 
//The row number also determines the y-position of each row.  The scene is drawn with 8 by 5 units, so 
//dividing 5 into four intervals (necessary for three evenly-spaced rows) will result in a height of 1.25.  The
//row number is multiplied by this interval, so row 1 will be lowest, row 2 will be 1.25 units above row 1 and so on.
//If additional rows (more than 3) are defined, colors will repeat.  
void drawTeapots(int rowNumber) {

	for (int c = 0; c < 5; c++) {//Runs through the following to draw five teapots per row. 

		glPushMatrix();
		//TEAPOT COLORS
		if (rowNumber % 3 == 0) {  //Colors for third (top) row are defined here. 
			switch (c) {
			case 0: glColor3fv(darkGrey);
				break;
			case 1: glColor3fv(darkRed);
				break;
			case 2: glColor3fv(blue);
				break;
			case 3: glColor3fv(darkGreen);
				break;
			case 4: glColor3fv(magenta);
				break;
			default: break;
			}
		}
		else if (rowNumber % 2 == 0) { //Second row colors
			switch (c) {
			case 0: glColor3fv(lightGrey);
				break;
			case 1: glColor3fv(red);
				break;
			case 2: glColor3fv(darkBlue);
				break;
			case 3: glColor3fv(mediumGrey);
				break;
			case 4: glColor3fv(yellow);
				break;
			default: break;
			}
		}
		else { //Colors for first (bottom) row
			switch (c) {
			case 0: glColor3fv(purple);
				break;
			case 1: glColor3fv(orange);
				break;
			case 2: glColor3fv(green);
				break;
			case 3: glColor3fv(pink);
				break;
			case 4: glColor3fv(cyan);
				break;
			default: break;
			}
		}
		//END OF STATEMENTS DEFINING TEAPOT COLORS

		//TEAPOT TRANSLATION
		if (rowNumber % 2 == 0) {	//Even row[s] will move left by decrementing on the x-axis.
			if (row2XPosition[c] > 0) {
				row2XPosition[c] -= translationIncrement;//Get the new position and move left.
				glTranslated(row2XPosition[c], 1.25*rowNumber, 2.5);
			}
			else {					   //This statement will execute when the given teapot passes the left boundary.
				row2XPosition[c] = 8.0;//Reset the x-position to the far right side of the screen
				glTranslated(row2XPosition[c], 1.25*rowNumber, 2.5);
			}
		}
		else {						//Odd rows will move right by incrementing on the y-axis. 
			if (row1XPosition[c] < 8.0) {
				row1XPosition[c] += translationIncrement;//Get the new position and move right.
				glTranslated(row1XPosition[c], 1.25*rowNumber, 2.5);
			}
			else {					 //This statement will execute when the teapot passes the right boundary.
				row1XPosition[c] = 0;//Reset the x-position to the far left side of the screen.
				glTranslated(row1XPosition[c], 1.25*rowNumber, 2.5);
			}
		}
		//END OF TEAPOT TRANSLATION STATEMENTS

		//The following rotates the position of the teapot.  To ensure that neighboring teapots don't match the relative rotations 
		//of neighboring teapots in the other rows, the row number is multiplied by 60 degrees and added to the starting position.
		//Each additional frame will increment the rotation of the teapot.  The vector of rotation is the y-axis, so 
		//rotation will show the all sides of the teapot (as opposed to 'rolling' (x-axis) or 'flipping' (z-axis) the teapots).
		glRotated(startRotation[c] + (60 * rowNumber) + rotationIncrement*frameCount, 0, 1, 0);  //ROTATION STATEMENT
		if (isTeapot[(rowNumber - 1) * 5 + c]) {
			//glutSolidSphere(0.3, 10, 10);
			glutSolidTeapot(0.3);
		}
		glPopMatrix();

	}//end of for-loop
	return;

}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//TEXT PLACEMENT 
	glTranslated(3.3, 3.5, 5.2);//Place text in center of world window, towards tops
	displayText(text);
	glTranslated(-3.3, -3.5, -5.2);//Return to origin

	//Calls the function to draw the cannon.
	drawCannon();

	//If there are any balls, call the 'drawBalls' function to render them and 
	//increment their positions. 
	if (ballCount) { drawBalls(); } 

	//The following three functions render the teapots with respect to their
	//current translation and rotation. 
	drawTeapots(1);
	drawTeapots(2);
	drawTeapots(3);
	
	//WALLS
	//The room that covers the whole screen represents 8 x-units, 5 y-units, and 3.4 z-units.
	
	
	/*
	//To keep the tiles proportionate and aligned for each wall, the texel cooderdinates are 
	//used with these scales as well.  The walls are simple 2D quads. 
	glEnable(GL_TEXTURE_2D);//Enable texture state object
	glBindTexture(GL_TEXTURE_2D, 1);//Bind the texture loaded in 'initialize' to '1'
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);//Turn on the texture display modes
	*/

	glColor3fv(orange);
	//Back Wall (8x5 units, 3.4 units back)
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0); glVertex3f(0.0, 0.0, -1.7);//Bottom left corner
	glTexCoord2f(0.0, 5.0);  glVertex3f(0.0, 5.0, -1.7);//Top left corner
	glTexCoord2f(8.0, 5.0);   glVertex3f(8.0, 5.0, -1.7);//Top right corner
	glTexCoord2d(8.0, 0);  glVertex3f(8.0, 0.0, -1.7);//Bottom right corner
	glEnd();
	//Left Wall (5x3.4 units, 0 units from left)
	glColor3fv(blue);
	glBegin(GL_QUADS);
	glTexCoord2f(-3.4, 0); glVertex3f(0.0, 0.0, 1.7);//Front bottom corner
	glTexCoord2f(-3.4, 5.0);  glVertex3f(0.0, 5.0, 1.7);//Front top corner
	glTexCoord2f(0, 5.0);   glVertex3f(0.0, 5.0, -1.7);//Back top corner
	glTexCoord2f(0, 0);  glVertex3f(0.0, 0.0, -1.7);//Back bottom corner
	glEnd();
	//Right wall (5x3.4 units, 8 units from left)
	glColor3fv(green);
	glBegin(GL_QUADS);
	glTexCoord2f(-3.4, 0); glVertex3f(8.0, 0.0, 1.7);//Front bottom corner
	glTexCoord2f(-3.4, 5.0);  glVertex3f(8.0, 5.0, 1.7);//Front top corner
	glTexCoord2f(0, 5.0);   glVertex3f(8.0, 5.0, -1.7);//Back top corner
	glTexCoord2f(0, 0);  glVertex3f(8.0, 0.0, -1.7);//Back bottom corner
	glEnd();
	//Top wall (8x3.4 units, 5 units from bottom)
	glColor3fv(red);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0.0); glVertex3f(0.0, 5.0, -1.7);//Back left corner
	glTexCoord2f(8.0, 0.0);  glVertex3f(8.0, 5.0, -1.7);//Back right corner
	glTexCoord2f(8.0, 3.4);   glVertex3f(8.0, 5.0, 1.7); //Front right corner
	glTexCoord2f(0, 3.4);  glVertex3f(0.0, 5.0, 1.7);//Front left corner
	glEnd();
	//Bottom wall (8x3.4 units, 0 units from bottom)
	glColor3fv(red);
	glBegin(GL_QUADS);
	glTexCoord2f(0, -3.4); glVertex3f(0.0, 0.0, 1.7);//Front left corner
	glTexCoord2f(8.0, -3.4);  glVertex3f(8.0, 0.0, 1.7);//Front right corner
	glTexCoord2f(8.0, 0);   glVertex3f(8.0, 0.0, -1.7); //Back right corner
	glTexCoord2f(0, 0);  glVertex3f(0.0, 0.0, -1.7);//Back left corner
	glEnd();
	//glDisable(GL_TEXTURE_2D);//Turn off texture mapping
	//END OF WALLS CODE

	//Display mechanisms
	glutSwapBuffers();//Render the frame
	frameCount++;//Increment the next frame.  This will be multipled by the rotation-increment value to determine what 
				 //the proper rotation for each frame will be.  
	glutPostRedisplay();//Rerender the display with adjusted values to animate.
}

//KEYBOARD FUNCTIONS
//The 'keyboard' function accepts the key 'Q' at any point during gameplay, and
//'Y' or 'N' when the teapots have been cleared to allow the player to play again or quit. 
void keyboard(unsigned char keyDown, int mouseX, int mouseY) {
	if (keyDown == 'q' || keyDown == 'Q') { exit(-1); }//Press 'Q' to quit.
	else if (keyDown == 32) {	//32 is ASCII for space. Fires the cannon
		if (ballCount < 5) { fireCannon(); }//Only 5 balls are allowed.
	}
	//If the teapots have been cleared, accept Y or N to reset the game or quit.
	if (!teapotCount) {
		if (keyDown == 'Y' || keyDown == 'y') { //Reset the game.
			teapotCount = 15;
			for (int c = 0; c < 15; c++) { isTeapot[c] = true; }
			setText(text);
			displayText(text);
		}
		else if (keyDown == 'N' || keyDown == 'n') { exit(-1); }//Quit
	}

}
//The 'specialKeyboard' function sets the values for the arrow keys.  These
//adjust the pitch and yaw of the cannon. 
void specialKeyboard(int keyDown, int mouseX, int mouseY) {
		if (keyDown == GLUT_KEY_UP) {       //UP MOVEMENT 
			if (cannonPitch <= 36) { cannonPitch += 2; }
		}
		else if (keyDown == GLUT_KEY_DOWN) {//DOWN MOVEMENT 
			if (cannonPitch >= -6) { cannonPitch -= 2; }
		}
		else if (keyDown == GLUT_KEY_LEFT) {//LEFT MOVEMENT 
			if (cannonYaw >= -40) { cannonYaw -= 2; }
		}
		else if (keyDown == GLUT_KEY_RIGHT) {//RIGHT MOVEMENT 
			if (cannonYaw <= 40) { cannonYaw += 2; }
		}

}

void main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Homework 3");
	glutKeyboardFunc(keyboard);//Register keyboard function for standard keys (Q, Y, N);
	glutSpecialFunc(specialKeyboard);
	glutDisplayFunc(display);
	initialize();			//Move GL enable functions to init function for readibility.					
	glViewport(0, 0, windowWidth, windowHeight);//Set the viewport to the width and height parameters.
	glutMainLoop();
	return;
}