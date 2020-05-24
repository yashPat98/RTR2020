#include <GL/freeglut.h>

bool bFullscreen = false;

int main(int argc, char** argv) {
	//Code
	//function declaration
	void initialize(void);
	void resize(int, int);
	void display(void);
	void keyboard(unsigned char, int, int);
	void mouse(int, int, int, int);
	void uninitialize(void);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("GLUT: Yash Patel");

	initialize();

	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutCloseFunc(uninitialize);

	glutMainLoop();

	return (0);
}

void initialize(void)
{
	//Code
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void resize(int width, int height)
{
	//Code
	if (height <= 0)
		height = 1;

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

void display(void)
{
	//Code
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    	glBegin(GL_QUADS);
        	glColor3f(0.67f, 0.84f, 0.9f);
       		glVertex3f(-1.0f, 0.0f, 0.0f);
        	glVertex3f(-1.0f, 1.0f, 0.0f);
        	glVertex3f(1.0f, 1.0f, 0.0f);
        	glVertex3f(1.0f, 0.0f, 0.0f);

        	glColor3f(0.71f, 0.5f, 0.11f);
        	glVertex3f(-1.0f, 0.0f, 0.0f);
        	glVertex3f(1.0f, 0.0f, 0.0f);
        	glVertex3f(1.0f, -1.0f, 0.0f);
        	glVertex3f(-1.0f, -1.0f, 0.0f);
    	glEnd();

    	glBegin(GL_TRIANGLES);
        	//Mountain T3
        	glColor3f(0.0f, 0.5f, 0.0f);
        	glVertex3f(0.0f, 0.0f, 0.0f);

        	glColor3f(1.0f, 1.0f, 1.0f);
        	glVertex3f(-0.7f, 0.5f, 0.0f);

        	glColor3f(0.0f, 0.5f, 0.0f);
        	glVertex3f(-1.2f, -0.2f, 0.0f);

        	//Mountain T3
        	glColor3f(0.0f, 0.5f, 0.0f);
        	glVertex3f(0.0f, 0.0f, 0.0f);

        	glColor3f(1.0f, 1.0f, 1.0f);
        	glVertex3f(0.4f, 0.6f, 0.0f);

        	glColor3f(0.0f, 0.5f, 0.0f);
        	glVertex3f(1.2f, -0.2f, 0.0f);

        	//Mountain T2
        	glColor3f(0.0f, 0.4f, 0.0f);
        	glVertex3f(-1.0f, -0.2f, 0.0f);

        	glColor3f(0.0f, 0.5f, 0.0f);
       	 	glVertex3f(-1.0f, 0.3f, 0.0f);

       		glColor3f(0.0f, 0.4f, 0.0f);
        	glVertex3f(-0.5f, -0.2f, 0.0f);

        	//Mountain T2
        	glColor3f(0.0f, 0.4f, 0.0f);
        	glVertex3f(-0.5f, -0.2f, 0.0f);

        	glColor3f(0.0f, 0.5f, 0.0f);
        	glVertex3f(-0.1f, 0.6f, 0.0f);

        	glColor3f(0.0f, 0.4f, 0.0f);
        	glVertex3f(0.3f, -0.2f, 0.0f);

        	//Mountain T2
        	glColor3f(0.0f, 0.4f, 0.0f);
        	glVertex3f(0.1f, -0.2f, 0.0f);

        	glColor3f(0.0f, 0.4f, 0.0f);
        	glVertex3f(0.5f, 0.4f, 0.0f);

        	glColor3f(0.0f, 0.4f, 0.0f);
        	glVertex3f(0.8f, -0.2f, 0.0f);


        	//Mountain T1
        	glColor3f(0.0f, 0.3f, 0.0f);
        	glVertex3f(-1.0f, -0.2f, 0.0f);

        	glColor3f(0.0f, 0.4f, 0.0f);
        	glVertex3f(-0.6f, 0.3f, 0.0f);

        	glColor3f(0.0f, 0.3f, 0.0f);
        	glVertex3f(-0.2f, -0.2f, 0.0f);

       	 	//Mountain T1
        	glColor3f(0.0f, 0.3f, 0.0f);
        	glVertex3f(-0.2f, -0.2f, 0.0f);

        	glColor3f(0.0f, 0.4f, 0.0f);
        	glVertex3f(0.2f, 0.2f, 0.0f);

        	glColor3f(0.0f, 0.3f, 0.0f);
        	glVertex3f(0.5f, -0.2f, 0.0f);

        	//Mountain T1
        	glColor3f(0.0f, 0.3f, 0.0f);
        	glVertex3f(0.5f, -0.2f, 0.0f);

        	glColor3f(0.0f, 0.4f, 0.0f);
        	glVertex3f(0.75f, 0.5f, 0.0f);

        	glColor3f(0.0f, 0.3f, 0.0f);
        	glVertex3f(1.0f, -0.2f, 0.0f);
    	glEnd();


	glFlush();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		glutLeaveMainLoop();
		break;
	case 'F':
	case 'f':
		if (bFullscreen == false)
		{
			glutFullScreen();
			bFullscreen = true;
		}
		else
		{
			glutLeaveFullScreen();
			bFullscreen = false;
		}
		break;
	default:
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	//Code
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		break;
	case GLUT_RIGHT_BUTTON:
		glutLeaveMainLoop();
		break;
	default:
		break;
	}
}

void uninitialize(void)
{
	//Code
}
