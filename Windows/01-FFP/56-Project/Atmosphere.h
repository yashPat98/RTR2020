// --- Function Prototypes ---
void RenderSky(void);
void RenderGround(void);
void RenderChristmasTreeLarge(bool bShadow);
void RenderChristmasTreeSmall(bool bShadow);

void ShadowMatrix(float *proj, const float *planeEq, const float *lightPos);
void GetPlaneEquation(float *planeEq, const float *p1, const float *p2, const float *p3);
void CrossProduct(const float vector1[], const float vector2[], float out[3]);
void Normalize(float vector[]);
