// -- Headers ---
#pragma once
#include "Destiny.h"
#include "Atmosphere.h"                    

// --- RenderSky() - Renders sky as texture in ortho mode ---
void RenderSky(void)
{
    //code
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0f, 1.0f, 0.0f, 1.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        //dont write to depth buffer
        glDepthMask(GL_FALSE);
        glDisable(GL_LIGHTING);

        glBegin(GL_QUADS);
            glColor3f(1.0f, 1.0f, 1.0f);

            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, 0.0f);

            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(0.0f, 1.0f, 0.0f);

            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(0.0f, 0.4f, 0.0f);

            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(1.0f, 0.4f, 0.0f);
        glEnd();

        glEnable(GL_LIGHTING);
        glDepthMask(GL_TRUE);

        //reset projection matrix
        glMatrixMode(GL_PROJECTION);
    glPopMatrix();

     glMatrixMode(GL_MODELVIEW);
}

// --- RenderGround() - Renders a terrain of size 40 x 40 ---
void RenderGround(void)
{
    GLfloat fExtent = 20.0f;
    GLfloat fStep = 1.0f;
    GLfloat y = -0.4f;
    GLfloat iStrip, iRun;
    GLfloat s = 0.0f;
    GLfloat t = 0.0f;
    GLfloat texStep = 1.0f / (fExtent * 0.055f);

    for(iStrip = -fExtent; iStrip <= fExtent; iStrip += fStep)
    {
        t = 0.0f;
        glBegin(GL_TRIANGLE_STRIP);
            glColor3f(1.0f, 1.0f, 1.0f);
            for(iRun = fExtent; iRun >= -fExtent; iRun -= fStep)
            {
                glNormal3f(0.0f, 1.0f, 0.0f);

                glTexCoord2f(s, t);
                glVertex3f(iStrip, y, iRun);

                glTexCoord2f(s + texStep, t);
                glVertex3f(iStrip + fStep, y, iRun);

                t += texStep;
            }
        glEnd();
        s += texStep;
    }
}

// --- RenderChristmasTreeLarge() - Renders large christmas tree ---
void RenderChristmasTreeLarge(bool bShadow)
{
    //variable declaration
    GLUquadric *quadric = NULL;

    //code
    glBindTexture(GL_TEXTURE_2D, NULL);
    glColor3f(0.0f, 0.0f, 0.0f);

    glPushMatrix();
        //main branch
        glTranslatef(0.0f, 0.1f, 0.0f);
        glScalef(0.5f, 0.5f, 0.5f);

        if(!bShadow)
            glColor3f(0.55294f, 0.38039f, 0.258823f);
        
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.2f, 0.25f, 1.0f, 6, 2);
        
        //leaves
        if(!bShadow)
            glColor3f(0.0f, 0.333333f, 0.0f);

        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.7f, 1.0f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.6f, 0.9f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.5f, 0.8f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.4f, 0.7f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.3f, 0.6f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -1.0f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.0f, 0.5f, 1.2f, 10, 2);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
    gluDeleteQuadric(quadric);
    quadric = NULL;
}

// --- RenderChristmasTreeLarge() - Renders Small christmas tree ---
void RenderChristmasTreeSmall(bool bShadow)
{
    //variable declaration
    GLUquadric *quadric = NULL;

    //code
    glBindTexture(GL_TEXTURE_2D, NULL);
    glColor3f(0.0f, 0.0f, 0.0f);

    //code
    glPushMatrix();
        //main branch
        glTranslatef(0.0f, 0.1f, 0.0f);
        glScalef(0.5f, 0.5f, 0.5f);

        if(!bShadow)
            glColor3f(0.55294f, 0.38039f, 0.258823f);

        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.2f, 0.25f, 1.0f, 6, 2);
        
        //leaves
        if(!bShadow)
            glColor3f(0.0f, 0.333333f, 0.0f);

        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.7f, 1.0f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.6f, 0.9f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.5f, 0.8f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -0.5f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.4f, 0.7f, 0.6f, 10, 2);

        glTranslatef(0.0f, 0.0f, -1.0f);
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluCylinder(quadric, 0.0f, 0.6f, 1.2f, 10, 2);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
    gluDeleteQuadric(quadric);
    quadric = NULL;
}

// --- ShadowMatrix() - Creates a shadow projection matrix on ground (y = -4) ---
void ShadowMatrix(float *proj, const float *planeEq, const float *lightPos)
{
    //variable declaration
    float a = planeEq[0];
    float b = planeEq[1];
    float c = planeEq[2];
    float d = planeEq[3];

    float dx = -lightPos[0];
    float dy = -lightPos[1];
    float dz = -lightPos[2];

    //code
    //fill the projction matrix
    proj[0] = b * dy + c * dz;
    proj[1] = -a * dy;
    proj[2] = -a * dz;
    proj[3] = 0.0f;

    proj[4] = -b * dx;
    proj[5] = a * dx + c * dz;
    proj[6] = -b * dz;
    proj[7] = 0.0f;

    proj[8] = -c * dx;
    proj[9] = -c * dy;
    proj[10] = a * dx + b * dy;
    proj[11] = 0.0f;

    proj[12] = -d * dx;
    proj[13] = -d * dy;
    proj[14] = -d * dz;
    proj[15] = a * dx + b * dy + c * dz;
}

// --- GetPlaneEquation() - Creates plane equation from three points ---
void GetPlaneEquation(float *planeEq, const float *p1, const float *p2, const float *p3)
{
    //function declaration
    void CrossProduct(const float vector1[], const float vector2[], float out[3]);

    //variable declaration
    float vec1[3];
    float vec2[3];

    //code
    //vector1 = point3 - point1
    vec1[0] = p3[0] - p1[0];
    vec1[1] = p3[1] - p1[1];
    vec1[2] = p3[2] - p1[2];

    //vector2 = point2 - point1
    vec2[0] = p2[0] - p1[0];
    vec2[1] = p2[1] - p1[1];
    vec2[2] = p2[2] - p1[2];

    //unit normal to plane
    CrossProduct(vec1, vec2, planeEq);

    //substitute any of point to get value of constat d
    planeEq[3] = -(planeEq[0] * p3[0] + planeEq[1] * p3[1] + planeEq[2] * p3[2]);
}

// --- CrossProduct() - Calculates cross product of two vector (Redbook) ---
void CrossProduct(const float vector1[], const float vector2[], float out[3])
{
    //function declaration 
    void Normalize(float vector[]);

    //code
    out[0] = vector1[1] * vector2[2] - vector1[2] * vector2[1];
    out[1] = vector1[2] * vector2[0] - vector1[0] * vector2[2];
    out[2] = vector1[0] * vector2[1] - vector1[1] * vector2[0];

    Normalize(out);
}

// --- Normalize() - Normalizes given vector (Redbook) ---
void Normalize(float vector[])
{
    //code
    GLfloat dist = sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);

    if(dist == 0.0f)
        return;
    
    vector[0] = vector[0] / dist;
    vector[1] = vector[1] / dist;
    vector[2] = vector[2] / dist;
}
