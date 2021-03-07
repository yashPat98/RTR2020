// --- Headers ---
#include "Destiny.h"
#include "Models.h"

void InitHumanoid(Humanoid_Animation *prop)
{
    prop->bWalk = true;
    prop->headAngle = 130.0f;

    // - leg movement -
    prop->left_thigh_speed = 1.5f;
    prop->right_thigh_speed = 1.5f;
    prop->left_knee_speed = 2.4f;
    prop->right_knee_speed = 2.4f;
    prop->right_heel_speed = 1.5f;
    prop->left_heel_speed = 1.5f;

    prop->left_thigh = 0.0f;
    prop->right_thigh = 0.0f;
    prop->left_knee = 0.0f;
    prop->right_knee = 0.0f;
    prop->right_heel = 0.0f;
    prop->left_heel = 0.0f;

    prop->right_knee_first_cycle = false;
    prop->left_knee_first_cycle = false;
    prop->right_heel_first_cycle = false;
    prop->left_heel_first_cycle = true;
    
    // - hand movement -
    prop->left_elbow = 0.0f;
    prop->right_elbow = 0.0f;
    prop->left_forearm = 0.0f;
    prop->right_forearm = 0.0f;

    prop->left_elbow_speed = 0.75f;
    prop->right_elbow_speed = 0.75f;
    prop->left_forearm_speed = 1.5f;
    prop->right_forearm_speed = 1.5f;

    prop->left_forearm_first_cycle = false;
    prop->right_forearm_first_cycle = true;
}

// --- RenderHumanoid() - Renders Human Shaped Model like puppet ---
void RenderHumanoid(Humanoid_Animation *prop, Humanoid_Appearance looks)
{
    //function declaration
    void DrawTorus(GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor);
    void DrawHand(void);

    //variable declaration
    GLUquadric *quadric = NULL;

    //code
    glPushMatrix();
        glTranslatef(0.0f, 0.26f, 0.0f);
        glScalef(0.12f, 0.12f, 0.12f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glColor3f(0.0f, 0.0f, 0.0f);

        glPushMatrix(); 
            glRotatef(prop->headAngle, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, 0.9f, 0.0f);

            // --- Head ---
            glPushMatrix();
                if(!looks.bShadow)
                    glColor3f(0.85546f, 0.66015f, 0.453125f);
                glScalef(0.75f, 1.0f, 0.8f);
                quadric = gluNewQuadric();
                gluSphere(quadric, 0.5f, 20, 20);
            glPopMatrix();

            //eyes
            glPushMatrix();
                glTranslatef(0.0f, 0.0f, 0.4f);
                gluSphere(quadric, 0.05f, 10, 10);
            glPopMatrix();

            // --- Neck ---
            glTranslatef(0.0f, -0.55f, 0.0f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
            glScalef(0.7f, 0.8f, 1.0f);
            DrawTorus(0.13f, 0.08f, 20, 20);
        glPopMatrix();

        glPushMatrix();
            // --- Central Body ---
            glPushMatrix();
                if(!looks.bShadow)
                    glColor3fv(looks.shirtColor);
                glScalef(1.0f, 0.6f, 1.0f);
                quadric = gluNewQuadric();
                gluCylinder(quadric, 0.85f, 0.15f, 0.3f, 20, 20);

                glTranslatef(0.0f, 0.0f, -1.2f);
                quadric = gluNewQuadric();
                gluCylinder(quadric, 0.75f, 0.85f, 1.2f, 20, 20);
            glPopMatrix();

            glPushMatrix();
                //joint for central part
                glTranslatef(0.0f, 0.0f, -1.31f);
                glScalef(0.95f, 0.6f, 0.5f);
                DrawTorus(0.4575f, 0.325f, 20, 20);

                // --- Waist ---
                glTranslatef(0.0f, 0.0f, -2.0f);
                quadric = gluNewQuadric();
                gluCylinder(quadric, 0.85f, 0.75f, 1.8f, 20, 20);
            glPopMatrix();
        glPopMatrix();

        // --- Arms ---

        //left
        glPushMatrix();
            glTranslatef(-0.85f, 0.0f, -0.14f);
            glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

            //shoulder joint
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.18f, 20, 20);

            //elbow
            glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
            glRotatef(prop->left_elbow, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.175f, 0.15f, 1.0f, 20, 20);

            //joint
            glTranslatef(0.0f, 0.0f, 1.0f);
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.15f, 20, 20);

            //forearm
            glRotatef(prop->left_forearm, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.15f, 0.08f, 1.1f, 20, 20);

            //rope and chains
            if(looks.bChains)
            {
                glPushMatrix();
                    if(!looks.bShadow)
                        glColor3f(0.62109f, 0.46875f, 0.375f);
                    glTranslatef(0.0f, 0.0f, 1.0f);
                    DrawTorus(0.1f, 0.05f, 20, 20);

                    glDisable(GL_LIGHTING);
                    glDisable(GL_TEXTURE_2D);

                    glBegin(GL_LINES);
                        glVertex3f(0.0f, 0.0f, 0.0f);
                        glVertex3f(0.0f, -prop->left_forearm + 5.0f, -50.0f);
                    glEnd();

                    glEnable(GL_LIGHTING);
                    glEnable(GL_TEXTURE_2D);
                glPopMatrix();
            }

            //hand
            if(!looks.bShadow)
                glColor3f(0.85546f, 0.66015f, 0.453125f);
            glTranslatef(0.0f, 0.0f, 1.15f);
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            glScalef(0.22f, 0.22f, 0.22f);
            DrawHand();
        glPopMatrix();

        //right
        glPushMatrix();
            if(!looks.bShadow)
                glColor3fv(looks.shirtColor);
            glTranslatef(0.85f, 0.0f, -0.14f);
            glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

            //shoulder joint
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.18f, 20, 20);

            //elbow
            glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
            glRotatef(prop->right_elbow, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.175f, 0.15f, 1.0f, 20, 20);

            //joint
            glTranslatef(0.0f, 0.0f, 1.0f);
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.15f, 20, 20);

            //forearm
            glRotatef(prop->right_forearm, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.15f, 0.08f, 1.1f, 20, 20);

            //rope and chains
            if(looks.bChains)
            {
                glPushMatrix();
                    if(!looks.bShadow)
                        glColor3f(0.62109f, 0.46875f, 0.375f);
                    glTranslatef(0.0f, 0.0f, 1.0f);
                    DrawTorus(0.1f, 0.05f, 20, 20);

                    glDisable(GL_LIGHTING);
                    glDisable(GL_TEXTURE_2D);

                    glBegin(GL_LINES);
                        glVertex3f(0.0f, 0.0f, 0.0f);
                        glVertex3f(0.0f, -prop->right_forearm + 5.0f, -50.0f);
                    glEnd();

                    glEnable(GL_LIGHTING);
                    glEnable(GL_TEXTURE_2D);
                glPopMatrix();
            }

            //hand
            if(!looks.bShadow)
                glColor3f(0.85546f, 0.66015f, 0.453125f);
            glTranslatef(0.0f, 0.0f, 1.15f);
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
            glScalef(0.22f, 0.22f, 0.22f);
            DrawHand();
        glPopMatrix();

        // --- Lower Body ---
        //right 
        glPushMatrix();
            glTranslatef(0.4f, 0.0f, -2.3f);
            
            if(!looks.bShadow)
                glColor3fv(looks.pantColor);

            //joint for left leg
            glPushMatrix();
                glScalef(1.2f, 1.0f, 1.0f);
                quadric = gluNewQuadric();
                gluSphere(quadric, 0.32f, 20, 20);
            glPopMatrix();

            //thigh
            glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(prop->right_thigh, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.35f, 0.2f, 1.5f, 20, 20);

            //knee
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -1.5f, 0.0f);
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.225f, 10, 10);

            //leg
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, 0.05f);
            glRotatef(prop->right_knee, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.225f, 0.125f, 1.5f, 20, 20);

            //rope and chains
            if(looks.bChains)
            {
                glPushMatrix();
                    if(!looks.bShadow)
                        glColor3f(0.62109f, 0.46875f, 0.375f);
                    glTranslatef(0.0f, 0.0f, 1.3f);
                    DrawTorus(0.15f, 0.075f, 20, 20);

                    glDisable(GL_LIGHTING);
                    glDisable(GL_TEXTURE_2D);

                    glBegin(GL_LINES);
                        glVertex3f(0.0f, 0.0f, 0.0f);
                        glVertex3f(0.0f, -(prop->right_thigh + prop->right_knee) / 2.1f, -50.0f);
                    glEnd();

                    glEnable(GL_LIGHTING);
                    glEnable(GL_TEXTURE_2D);
                glPopMatrix();
            }

            //heel
            if(!looks.bShadow)
                glColor3f(0.2109f, 0.26953f, 0.30859f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -1.6f, 0.0f);
            glRotatef(prop->right_heel, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.15f, 10, 10);

            //foot
            glTranslatef(0.0f, 0.0f, 0.08f);
            glScalef(0.5f, 0.35f, 1.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.34f, 0.33f, 0.4f, 20, 20);

            glTranslatef(0.0f, 0.0f, 0.4f);
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.33f, 10, 10);
        glPopMatrix();

        //left
        glPushMatrix();
            glTranslatef(-0.4f, 0.0f, -2.3f);

            if(!looks.bShadow)
                glColor3fv(looks.pantColor);

            //joint for left leg
            glPushMatrix();
                glScalef(1.2f, 1.0f, 1.0f);
                quadric = gluNewQuadric();
                gluSphere(quadric, 0.32f, 20, 20);
            glPopMatrix();

            //thigh
            glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(prop->left_thigh, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.35f, 0.2f, 1.5f, 20, 20);

            //knee
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -1.5f, 0.0f);
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.225f, 10, 10);

            //leg
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, 0.05f);
            glRotatef(prop->left_knee, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.225f, 0.125f, 1.5f, 20, 20);

            //rope and chains
            if(looks.bChains)
            {
                glPushMatrix();
                    if(!looks.bShadow)
                        glColor3f(0.62109f, 0.46875f, 0.375f);
                    glTranslatef(0.0f, 0.0f, 1.3f);
                    DrawTorus(0.15f, 0.075f, 20, 20);

                    glDisable(GL_LIGHTING);
                    glDisable(GL_TEXTURE_2D);

                    glBegin(GL_LINES);
                        glVertex3f(0.0f, 0.0f, 0.0f);
                        glVertex3f(0.0f, -(prop->left_thigh + prop->left_knee) / 2.1f, -50.0f);
                    glEnd();

                    glEnable(GL_LIGHTING);
                    glEnable(GL_TEXTURE_2D);
                glPopMatrix();
            }
            
            //heel
            if(!looks.bShadow)
                glColor3f(0.2109f, 0.26953f, 0.30859f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -1.6f, 0.0f);
            glRotatef(prop->left_heel, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.15f, 10, 10);

            //foot
            glTranslatef(0.0f, 0.0f, 0.08f);
            glScalef(0.5f, 0.35f, 1.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.34f, 0.33f, 0.4f, 20, 20);

            glTranslatef(0.0f, 0.0f, 0.4f);
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.33f, 10, 10);
        glPopMatrix();

    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);

    //free quadric object
    gluDeleteQuadric(quadric);
    quadric = NULL;
}

void UpdateHumanoid(Humanoid_Animation *prop)
{
    //code
    // --- Right Arm Movement ---
    if(prop->bWalk == true)
    {
        prop->right_elbow -= prop->right_elbow_speed;
        if(prop->right_elbow <= -15.0f || prop->right_elbow >= 15.0f)
            prop->right_elbow_speed = -prop->right_elbow_speed;

        if(prop->right_elbow <= -1.0f)
            prop->right_forearm_first_cycle = true;

        if(prop->right_forearm_first_cycle)
        {
            prop->right_forearm -= prop->right_forearm_speed;
            if(prop->right_forearm <= -30.0f)
                prop->right_forearm_speed = -prop->right_forearm_speed;
        
            if(prop->right_forearm >= 0.0f)
            {
                prop->right_forearm = 0.0f;
                prop->right_forearm_first_cycle = false;
                prop->right_forearm_speed = 1.5f;
            }
        }

        // --- Left Arm Movement ---
        prop->left_elbow -= prop->left_elbow_speed;
        if(prop->left_elbow <= -15.0f || prop->left_elbow >= 15.0f)
            prop->left_elbow_speed = -prop->left_elbow_speed;

        if(prop->left_elbow >= 1.0f)
            prop->left_forearm_first_cycle = true;

        if(prop->left_forearm_first_cycle)
        {
            prop->left_forearm += prop->left_forearm_speed;
            if(prop->left_forearm >= 30.0f)
                prop->left_forearm_speed = -prop->left_forearm_speed;
        
            if(prop->left_forearm <= 0.0f)
            {
                prop->left_forearm = 0.0f;
                prop->left_forearm_first_cycle = false;
                prop->left_forearm_speed = 1.5f;
            }
        }

        // --- Right Leg Movement ---
        prop->right_thigh += prop->right_thigh_speed;
        if(prop->right_thigh >= 30.0f || prop->right_thigh <= -30.0f)
            prop->right_thigh_speed = -prop->right_thigh_speed;

        if(prop->right_thigh >= 25.0f)
            prop->right_knee_first_cycle = true;
    
        if(prop->right_knee_first_cycle)
        {
            prop->right_knee += prop->right_knee_speed;
            if(prop->right_knee >= 60.0f)
                prop->right_knee_speed = -prop->right_knee_speed;
        
            if(prop->right_knee <= 0.0f)
            {
                prop->right_knee_first_cycle = false;
                prop->right_knee_speed = 2.4f;
                prop->right_knee = 0.0f;
            }
        }

        if(prop->right_thigh <= -20.0f)
            prop->right_heel_first_cycle = true;

        if(prop->right_heel_first_cycle)
        {
            prop->right_heel += prop->right_heel_speed;
            if(prop->right_heel >= 20.0f)
                prop->right_heel_speed = -prop->right_heel_speed;
        
            if(prop->right_heel <= 0.0f)
            {
                prop->right_heel_first_cycle = false;
                prop->right_heel = 0.0f;
                prop->right_heel_speed = 1.5f;
            }
        }

        // --- Left Leg Movement ---
        prop->left_thigh -= prop->left_thigh_speed;
        if(prop->left_thigh >= 30.0f || prop->left_thigh <= -30.0f)
            prop->left_thigh_speed = -prop->left_thigh_speed;

        if(prop->left_thigh >= 25.0f)
            prop->left_knee_first_cycle = true;
    
        if(prop->left_knee_first_cycle)
        {
            prop->left_knee += prop->left_knee_speed;
            if(prop->left_knee >= 60.0f)
                prop->left_knee_speed = -prop->left_knee_speed;
        
            if(prop->left_knee <= 0.0f)
            {
                prop->left_knee_first_cycle = false;
                prop->left_knee_speed = 2.4f;
                prop->left_knee = 0.0f;
            }
        }

        if(prop->left_thigh <= -20.0f)
            prop->left_heel_first_cycle = true;

        if(prop->left_heel_first_cycle)
        {
            prop->left_heel += prop->left_heel_speed;
            if(prop->left_heel >= 20.0f)
                prop->left_heel_speed = -prop->left_heel_speed;
        
            if(prop->left_heel <= 0.0f)
            {
                prop->left_heel_first_cycle = false;
                prop->left_heel = 0.0f;
                prop->left_heel_speed = 1.5f;
            }
        }
    }
    else
    {
        prop->left_thigh = 0.0f;
        prop->right_thigh = 0.0f;
        prop->left_knee = 0.0f;
        prop->right_knee = 0.0f;
        prop->left_elbow = 0.0f;
        prop->right_elbow = 0.0f;
        prop->left_forearm = 0.0f;
        prop->right_forearm = 0.0f;
    }
}

void DrawHand(void)
{
    //variable declaration
    GLUquadric *quadric = NULL;

    //code
    glPushMatrix();
        //palm
        glPushMatrix();
            glTranslatef(0.0f, -0.25f, 0.0f);
            glScalef(0.6f, 0.3f, 0.25f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.88f, 1.0f, 4.0f, 20, 20);
        glPopMatrix();

        //1st finger
        glPushMatrix();
            glTranslatef(-0.45f, 0.9f, 0.0f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            //glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.125f, 0.115f, 0.34f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.34f);
            //glRotatef(60.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.115f, 0.095f, 0.34f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.34f);
            //glRotatef(60.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.095f, 0.075f, 0.2f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.2f);
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.075f, 10, 10);
        glPopMatrix();

        //2nd finger
        glPushMatrix();
            glTranslatef(-0.15f, 0.9f, 0.0f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            //glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.14f, 0.13f, 0.42f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.42f);
            //glRotatef(60.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.13f, 0.105f, 0.42f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.42f);
            //glRotatef(60.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.105f, 0.085f, 0.3f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.3f);
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.085f, 10, 10);
        glPopMatrix();

        //3rd finger
        glPushMatrix();
            glTranslatef(0.15f, 0.9f, 0.0f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            //glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.14f, 0.13f, 0.38f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.38f);
            //glRotatef(60.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.13f, 0.105f, 0.38f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.38f);
            //glRotatef(60.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.105f, 0.08f, 0.25f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.25f);
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.08f, 10, 10);
        glPopMatrix();

        //4th finger
        glPushMatrix();
            glTranslatef(0.45f, 0.9f, 0.0f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            //glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.12f, 0.105f, 0.28f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.28f);
            //glRotatef(60.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.105f, 0.085f, 0.28f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.28f);
            //glRotatef(60.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.085f, 0.07f, 0.18f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.18f);
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.07f, 10, 10);
        glPopMatrix();

        //thumb
        glPushMatrix();
            glTranslatef(-0.45f, 0.1f, 0.0f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(-45.0f, 0.0f, 1.0f, 0.0f);
            //glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.15f, 0.14f, 0.35f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.35f);
            //glRotatef(60.0f, 1.0f, 0.0f, 0.0f);
            //glRotatef(20.0f, 0.0f, 1.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.14f, 0.12f, 0.35f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.35f);
            //glRotatef(60.0f, 1.0f, 0.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.12f, 0.1f, 0.2f, 10, 10);

            glTranslatef(0.0f, 0.0f, 0.2f);
            quadric = gluNewQuadric();
            gluSphere(quadric, 0.1f, 10, 10);
        glPopMatrix();
    glPopMatrix();
    
    //free quadric object
    gluDeleteQuadric(quadric);
    quadric = NULL;
    
}

void DrawTorus(GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor)
{
    GLfloat vNormal[3];
    double majorStep = 2.0f*M_PI / numMajor;
    double minorStep = 2.0f*M_PI / numMinor;
    int i, j;
	
    for (i=0; i<numMajor; ++i) 
	{
		double a0 = i * majorStep;
		double a1 = a0 + majorStep;
		GLfloat x0 = (GLfloat) cos(a0);
		GLfloat y0 = (GLfloat) sin(a0);
		GLfloat x1 = (GLfloat) cos(a1);
		GLfloat y1 = (GLfloat) sin(a1);
		
		glBegin(GL_TRIANGLE_STRIP);
		for (j=0; j<=numMinor; ++j) 
		{
			double b = j * minorStep;
			GLfloat c = (GLfloat) cos(b);
			GLfloat r = minorRadius * c + majorRadius;
			GLfloat z = minorRadius * (GLfloat) sin(b);
			
			// First point
			glTexCoord2f((float)(i)/(float)(numMajor), (float)(j)/(float)(numMinor));
			vNormal[0] = x0*c;
			vNormal[1] = y0*c;
			vNormal[2] = z/minorRadius;
			
			glNormal3fv(vNormal);
			glVertex3f(x0*r, y0*r, z);
			
			glTexCoord2f((float)(i+1)/(float)(numMajor), (float)(j)/(float)(numMinor));
			vNormal[0] = x1*c;
			vNormal[1] = y1*c;
			vNormal[2] = z/minorRadius;
			
			glNormal3fv(vNormal);
			glVertex3f(x1*r, y1*r, z);
		}
		glEnd();
	}
}

void RenderBird(GLuint bird_texture, GLuint feather_texture)
{
    //function declaration
    void DrawCube(void);

    //variable declaration
    GLUquadric *quadric = NULL;

    static GLfloat wing_rot1 = 50.0f;
    static GLfloat wing_rot2 = 0.0f;
    static GLfloat wing_rot3 = 0.0f;
    static GLfloat step1 = 3.0f;
    static GLfloat step2 = 2.4f;
    static GLfloat step3 = 2.4f;
 
    static bool bFirstCycle = true;
    static bool bRot1 = true;
    static bool bRot2 = true;

    //code
    glPushMatrix();
        // --- Mid ---
        glPushMatrix();
            glBindTexture(GL_TEXTURE_2D, bird_texture);

            glScalef(0.8f, 0.35f, 0.6f);
            quadric = gluNewQuadric();
            gluQuadricTexture(quadric, GL_TRUE);
            gluSphere(quadric, 1.0f, 20, 20);
        glPopMatrix();

        // --- Head ---
        glPushMatrix();
            glTranslatef(-1.1f, 0.1f, 0.0f);
            
            glPushMatrix();
                glScalef(1.3f, 0.9f, 1.0f);
                quadric = gluNewQuadric();
                gluQuadricTexture(quadric, GL_TRUE);
                gluSphere(quadric, 0.3f, 20, 20);
            glPopMatrix();

            glDisable(GL_TEXTURE_2D);
            glColor3f(0.0f, 0.0f, 0.0f);

            glPushMatrix();
                glTranslatef(-0.28f, 0.13f, 0.12f);
                quadric = gluNewQuadric();
                gluSphere(quadric, 0.03f, 10, 10);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.28f, 0.13f, -0.12f);
                quadric = gluNewQuadric();
                gluSphere(quadric, 0.03f, 10, 10);
            glPopMatrix();

            // --- Beak ---
            glColor3f(1.0f, 1.0f, 0.0f);
            glTranslatef(-0.25f, -0.03f, 0.0f);
            glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
            quadric = gluNewQuadric();
            gluCylinder(quadric, 0.12f, 0.0f, 0.6f, 4, 1);

            glColor3f(1.0f, 1.0f, 1.0f);
            glEnable(GL_TEXTURE_2D);
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, feather_texture);

        // --- Left Wings ---
        glPushMatrix();
            glTranslatef(-0.2f, 0.0f, 0.0f);
            glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
            glRotatef(-10.0f, 0.0f, 0.0f, 1.0f);
            glRotatef(wing_rot1, 1.0f, 0.0f, 0.0f);

            glPushMatrix();
                glScalef(1.0f, 0.2f, 1.0f);
                quadric = gluNewQuadric();
                gluQuadricTexture(quadric, GL_TRUE);
                gluCylinder(quadric, 0.4f, 0.4f, 1.2f, 20, 20);
            glPopMatrix();

            glTranslatef(0.0f, 0.0f, 1.1f);
            glRotatef(-15.0f, 0.0f, 1.0f, 0.0f);
            glRotatef(-wing_rot2, 1.0f, 0.0f, 0.0f);
            
            glPushMatrix();
                glScalef(1.0f, 0.2f, 1.0f);
                quadric = gluNewQuadric();
                gluQuadricTexture(quadric, GL_TRUE);
                gluCylinder(quadric, 0.4f, 0.5f, 0.8f, 20, 20);
            glPopMatrix();

            glTranslatef(0.0f, 0.0f, 0.8f);
            glRotatef(-wing_rot3, 1.0f, 0.0f, 0.0f);
            glScalef(1.0f, 0.2f, 1.0f);
            quadric = gluNewQuadric();
            gluQuadricTexture(quadric, GL_TRUE);
            gluCylinder(quadric, 0.5f, 0.2f, 1.3f, 20, 20);

            glTranslatef(0.0f, 0.0f, 1.3f);
            quadric = gluNewQuadric();
            gluQuadricTexture(quadric, GL_TRUE);
            gluSphere(quadric, 0.2f, 10, 10);
        glPopMatrix();

        // --- Right Wings ---
        glPushMatrix();
            glTranslatef(-0.2f, 0.0f, 0.0f);
            glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(10.0f, 0.0f, 0.0f, 1.0f);
            glRotatef(wing_rot1, 1.0f, 0.0f, 0.0f);

            glPushMatrix();
                glScalef(1.0f, 0.2f, 1.0f);
                quadric = gluNewQuadric();
                gluQuadricTexture(quadric, GL_TRUE);
                gluCylinder(quadric, 0.4f, 0.4f, 1.2f, 20, 20);
            glPopMatrix();

            glTranslatef(0.0f, 0.0f, 1.1f);
            glRotatef(15.0f, 0.0f, 1.0f, 0.0f);
            glRotatef(-wing_rot2, 1.0f, 0.0f, 0.0f);
            
            glPushMatrix();
                glScalef(1.0f, 0.2f, 1.0f);
                quadric = gluNewQuadric();
                gluQuadricTexture(quadric, GL_TRUE);
                gluCylinder(quadric, 0.4f, 0.5f, 0.8f, 20, 20);
            glPopMatrix();

            glTranslatef(0.0f, 0.0f, 0.8f);
            glRotatef(-wing_rot3, 1.0f, 0.0f, 0.0f);
            glScalef(1.0f, 0.2f, 1.0f);
            quadric = gluNewQuadric();
            gluQuadricTexture(quadric, GL_TRUE);
            gluCylinder(quadric, 0.5f, 0.2f, 1.3f, 20, 20);

            glTranslatef(0.0f, 0.0f, 1.3f);
            quadric = gluNewQuadric();
            gluQuadricTexture(quadric, GL_TRUE);
            gluSphere(quadric, 0.2f, 10, 10);
        glPopMatrix();

        // --- Tail ---
        glPushMatrix();
            glTranslatef(0.5f, 0.1f, 0.0f);
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            glRotatef(-7.0f, 1.0f, 0.0f, 0.0f);
            glScalef(1.0f, 0.15f, 2.0f);
            quadric = gluNewQuadric();
            gluQuadricTexture(quadric, GL_TRUE);
            gluCylinder(quadric, 0.2f, 0.5f, 0.6f, 20, 20);

            glTranslatef(0.0f, 0.0f, 0.6f);
            glScalef(1.0f, 1.0f, 0.1f);
            quadric = gluNewQuadric();
            gluQuadricTexture(quadric, GL_TRUE);
            gluSphere(quadric, 0.5f, 10, 10);
        glPopMatrix();

        // --- Legs ---
        glPushMatrix();
            glTranslatef(0.4f, -0.37f, 0.15f);
            glRotatef(12.0f, 0.0f, 0.0f, 1.0f);
            glScalef(0.2f, 0.06f, 0.1f);
            DrawCube();
        glPopMatrix();

        glPushMatrix();
            glTranslatef(0.4f, -0.37f, -0.15f);
            glRotatef(10.0f, 0.0f, 0.0f, 1.0f);
            glScalef(0.2f, 0.06f, 0.1f);
            DrawCube();
        glPopMatrix();

        glColor3f(1.0f, 1.0f, 1.0f);
        glEnable(GL_TEXTURE_2D);
    glPopMatrix();

    //update
    if(bFirstCycle == true)
    {
        wing_rot1 -= step1;
        if(wing_rot1 <= -50.0f)
        {
            wing_rot2 = 0.0f;
            wing_rot3 = 0.0f;
            step2 = 2.4f;
            step3 = 2.4f;

            step1 = -step1;
            bFirstCycle = false;
        }

        wing_rot2 -= step2;
        if(wing_rot2 <= -37.5f)
            step2 = -step2;

        wing_rot3 -= step3;
        if(wing_rot3 <= -37.5f)
            step3 = -step3;
    }
    else
    {
        if(wing_rot1 <= 50.0f)
            wing_rot1 -= step1;
        
        if(bRot1 == true)
        {
            wing_rot2 += step2 + 0.4f;
            if(wing_rot2 >= 100.0f)
                step2 = -step2;
            else if(step2 <= 0.0f)
                bRot1 = false;
        }    
        else
        {
            if(wing_rot2 >= 0.0f)
                wing_rot2 += step2 * 4.0f;

            if(bRot2 == true)
            {
                wing_rot3 += step3 * 4.0f;
                if(wing_rot3 >= 100.0f)
                    step3 = -step3;
                else if(wing_rot3 <= 0.0f)
                    bRot2 = false;
            }
            else
            {
                wing_rot1 = 50.0f;
                wing_rot2 = 0.0f;
                wing_rot3 = 0.0f;
                step2 = 2.4f;
                step3 = 2.4f;
                step1 = 3.0f;

                bFirstCycle = true;
                bRot1 = true;
                bRot2 = true;
            }   
        }
        
    }

    gluDeleteQuadric(quadric);
}

void DrawCube(void)
{
    glBegin(GL_QUADS);
        //near
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);

        //right
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);

        //far
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);

        //left
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);

        //top
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);

        //bottom
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);        
    glEnd();
}
