// --- Humanoid Properties ---
struct Humanoid_Animation
{    
    bool bWalk;

    // - leg movement -
    GLfloat left_thigh_speed;
    GLfloat right_thigh_speed;
    GLfloat left_knee_speed;
    GLfloat right_knee_speed;
    GLfloat right_heel_speed;
    GLfloat left_heel_speed;

    GLfloat left_thigh;
    GLfloat right_thigh;
    GLfloat left_knee;
    GLfloat right_knee;
    GLfloat right_heel;
    GLfloat left_heel;

    GLfloat headAngle;

    bool right_knee_first_cycle;
    bool left_knee_first_cycle;
    bool right_heel_first_cycle;
    bool left_heel_first_cycle;
    
    // - hand movement -
    GLfloat left_elbow;
    GLfloat right_elbow;
    GLfloat left_forearm;
    GLfloat right_forearm;

    GLfloat left_elbow_speed;
    GLfloat right_elbow_speed;
    GLfloat left_forearm_speed;
    GLfloat right_forearm_speed;

    bool left_forearm_first_cycle;
    bool right_forearm_first_cycle;
};

struct Humanoid_Appearance
{
    GLfloat shirtColor[3];
    GLfloat pantColor[3]; 
    
    bool bShadow;
    bool bChains;

    //position
    GLfloat x, y, z;
};

// --- Function Prototypes ---
void RenderHumanoid(Humanoid_Animation *prop, Humanoid_Appearance looks);
void InitHumanoid(Humanoid_Animation *prop);
void UpdateHumanoid(Humanoid_Animation *prop);

void RenderBird(GLuint bird_texture, GLuint feather_texture);
