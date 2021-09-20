#pragma once

//class interface
@interface Sphere:NSObject
{
    int maxElements;
    int numElements;
    int numVertices;
    float model_vertices[1146];
    float model_normals[1146];
    float model_textures[764];
    short model_elements[2280];
}

-(id)init;
-(void)getSphereVertexData:(float*)spherePositionCoords :(float*)sphereNormalCoords :(float*)sphereTexCoords :(short*)sphereElements;
-(int)getNumberOfSphereVertices;
-(int)getNumberOfSphereElements;
-(void)processSphereData;
-(void)addTriangle:(float[3][3])single_vertex :(float[3][3])single_normal :(float[3][2])single_texture;
-(void)normalizeVector:(float*)v;
-(bool)isFoundIdentical:(const float)val1 :(const float)val2  :(const float)diff;

@end
