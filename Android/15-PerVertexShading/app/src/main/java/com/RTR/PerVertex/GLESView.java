package com.RTR.PerVertex;

import android.content.Context;

import android.opengl.GLES32;
import android.opengl.GLSurfaceView;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGLConfig;

import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

import android.opengl.Matrix;

//view for OpenGLES which also recives touch events
public class GLESView extends GLSurfaceView 
                      implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener
{
    private final Context context;
    private GestureDetector gestureDetector;

    private int vertexShaderObject;
    private int fragmentShaderObject;
    private int shaderProgramObject;

    private int modelMatrixUniform;
    private int viewMatrixUniform;
    private int projectionMatrixUniform;
    private int lightAmbientUniform;
    private int lightDiffuseUniform;
    private int lightSpecularUniform;
    private int lightPositionUniform;
    private int materialAmbientUniform;
    private int materialDiffuseUniform;
    private int materialSpecularUniform;
    private int materialShininessUniform;
    private int doubleTapUniform;

    private int[] vao_sphere = new int[1];
    private int[] vbo_sphere_vertices = new int[1];
    private int[] vbo_sphere_normals = new int[1];
    private int[] vbo_sphere_elements = new int[1];

    private float perspectiveProjectionMatrix[] = new float[16];
    private int double_tap = 0;
    int numVertices;
    int numElements;

    public GLESView(Context drawingContext)
    {
        super(drawingContext);
        context = drawingContext;

        setEGLContextClientVersion(3);
        setRenderer(this);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    
        gestureDetector = new GestureDetector(context, this, null, false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    //overriden methods of GLSurfaceView.Renderer
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
        //get OpenGL-ES version
        String glesVersion = gl.glGetString(GL10.GL_VERSION);
        String glslVersion = gl.glGetString(GLES32.GL_SHADING_LANGUAGE_VERSION);
        System.out.println("YIP: OpenGL-ES Version = " + glesVersion);
        System.out.println("YIP: GLSL Version = " + glslVersion);

        initialize(gl);
    }

    @Override
    public void onSurfaceChanged(GL10 unused, int width, int height)
    {
        resize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 unused)
    {
        render();
    }

    //overriden methods of OnDoubleTapListener
    @Override
    public boolean onTouchEvent(MotionEvent e)
    {
        int eventaction = e.getAction();
        if(!gestureDetector.onTouchEvent(e))
        {
            super.onTouchEvent(e);
        }
        
        return (true);
    }

    @Override
    public boolean onDoubleTap(MotionEvent e)
    {
        if(double_tap == 0)
        {
            double_tap = 1;
        }
        else
        {
            double_tap = 0;
        }

        return (true);
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e)
    {
        return (true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e)
    {
        return (true);
    }

    //overriden methods from onGestureListener
    @Override
    public boolean onDown(MotionEvent e)
    {
        return (true);
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
    {
        return (true);
    }

    @Override
    public void onLongPress(MotionEvent e)
    {
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY)
    {
        uninitialize();
        System.exit(0);
        return (true);
    }

    @Override
    public void onShowPress(MotionEvent e)
    {
    }

    @Override
    public boolean onSingleTapUp(MotionEvent e)
    {
        return (true);
    }

    private void initialize(GL10 gl)
    {
        //Vertex Shader

        //create shader
        vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

        //vertex shader source code
        final String vertexShaderSourceCode = String.format
        (
            "#version 320 es"                                   +
            "\n"                                                +
            
            "precision mediump float;"                          +

            "in vec4 vPosition;"                                +
            "in vec3 vNormal;"                                  +
            
            "uniform mat4 u_modelMatrix;"                       +
            "uniform mat4 u_viewMatrix;"                        +
            "uniform mat4 u_projectionMatrix;"                  +   
            "uniform vec3 u_lightAmbient;"                      +
            "uniform vec3 u_lightDiffuse;"                      +
            "uniform vec3 u_lightSpecular;"                     +
            "uniform vec4 u_lightPosition;"                     +
            "uniform vec3 u_materialAmbient;"                   +
            "uniform vec3 u_materialDiffuse;"                   +
            "uniform vec3 u_materialSpecular;"                  +   
            "uniform float u_materialShininess;"                +
            "uniform int u_doubleTap;"                          +

            "out vec3 phong_ads_light;"                         +

            "void main(void)"                                                                                                                               +
            "{"                                                                                                                                             +
            "   if(u_doubleTap == 1)"                                                                                                                       +
            "   {"                                                                                                                                          +
            "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                                                                            +
            "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix)));"                                                           +
            "       vec3 transformed_normal = normalize(normal_matrix * vNormal);"                                                                          +
            "       vec3 light_direction = normalize(vec3(u_lightPosition - eye_coords));"                                                                  +
            "       vec3 reflection_vector = reflect(-light_direction, transformed_normal);"                                                                +
            "       vec3 view_vector = normalize(-eye_coords.xyz);"                                                                                         +

            "       vec3 ambient = u_lightAmbient * u_materialAmbient;"                                                                                     +
            "       vec3 diffuse = u_lightDiffuse * u_materialDiffuse * max(dot(light_direction, transformed_normal), 0.0f);"                               +
            "       vec3 specular = u_lightSpecular * u_materialSpecular * pow(max(dot(reflection_vector, view_vector), 0.0f), u_materialShininess);"       +

            "       phong_ads_light = ambient + diffuse + specular;"                                                                                        +
            "   }"                                                                                                                                          +
            "   else"                                                                                                                                       +
            "   {"                                                                                                                                          +
            "       phong_ads_light = vec3(1.0f, 1.0f, 1.0f);"                                                                                              +
            "   }"                                                                                                                                          +

            "   gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * vPosition;"                                                               +
            "}"
        );

        //provide source code to shader
        GLES32.glShaderSource(vertexShaderObject, vertexShaderSourceCode);

        //compile shader
        GLES32.glCompileShader(vertexShaderObject);

        //error checking
        int[] iShaderCompiledStatus = new int[1];
        int[] iInfoLogLength = new int[1];
        String szInfoLog = null;

        GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompiledStatus, 0);
        if(iShaderCompiledStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(vertexShaderObject);
                System.out.println("YIP: Vertex Shader Compilation Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //Fragment Shader

        //create shader
        fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

        //fragment shader source code
        final String fragmentShaderSourceCode = String.format
        (
            "#version 320 es"                                   +
            "\n"                                                +
            "precision highp float;"                            +

            "in vec3 phong_ads_light;"                          +
            "out vec4 FragColor;"                               +

            "void main(void)"                                   +
            "{"                                                 +
            "   FragColor = vec4(phong_ads_light, 1.0f);"       +
            "}"
        );

        //provide source code to shader
        GLES32.glShaderSource(fragmentShaderObject, fragmentShaderSourceCode);

        //compile shader
        GLES32.glCompileShader(fragmentShaderObject);

        //error checking
        iShaderCompiledStatus[0] = 0;
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompiledStatus, 0);
        if(iShaderCompiledStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject);
                System.out.println("YIP: Fragment Shader Compilation Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //Shader Program

        //create shader program
        shaderProgramObject = GLES32.glCreateProgram();

        //attach vertex shader to shader program
        GLES32.glAttachShader(shaderProgramObject, vertexShaderObject);

        //attach fragment shader to shader program 
        GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject);
        
        //pre-link binding of shader program object with vertex shader attributes
        GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_VERTEX, "vPosition");
        GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

        //link shader program 
        GLES32.glLinkProgram(shaderProgramObject);

        //error checking
        int[] iShaderProgramLinkStatus = new int[1];
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_LINK_STATUS, iShaderProgramLinkStatus, 0);
        if(iShaderProgramLinkStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetProgramInfoLog(shaderProgramObject);
                System.out.println("YIP: Shader Program Link Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //get uniform locations
        modelMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
        viewMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
        projectionMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");
        lightAmbientUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lightAmbient");
        lightDiffuseUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lightDiffuse");
        lightSpecularUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lightSpecular");
        lightPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lightPosition");
        materialAmbientUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_materialAmbient");
        materialDiffuseUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_materialDiffuse");
        materialSpecularUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_materialSpecular");
        materialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_materialShininess");
        doubleTapUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_doubleTap");

        //sphere data
        float sphere_vertices[] = new float[1146];
        float sphere_normals[] = new float[1146];
        float sphere_textures[] = new float[764];
        short sphere_elements[] = new short[2280];

        Sphere sphere = new Sphere();
        sphere.getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
        numVertices = sphere.getNumberOfSphereVertices();
        numElements = sphere.getNumberOfSphereElements();

        //setup vao and vbo
        GLES32.glGenVertexArrays(1, vao_sphere, 0);
        GLES32.glBindVertexArray(vao_sphere[0]);
            //position vbo
            GLES32.glGenBuffers(1, vbo_sphere_vertices, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_sphere_vertices[0]);

            ByteBuffer byteBuffer = ByteBuffer.allocateDirect(sphere_vertices.length * 4);
            byteBuffer.order(ByteOrder.nativeOrder());
            FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
            verticesBuffer.put(sphere_vertices);
            verticesBuffer.position(0);

            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, sphere_vertices.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_VERTEX);

            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        
            //normal vbo
            GLES32.glGenBuffers(1, vbo_sphere_normals, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_sphere_normals[0]);

            byteBuffer = ByteBuffer.allocateDirect(sphere_normals.length * 4);
            byteBuffer.order(ByteOrder.nativeOrder());
            FloatBuffer normalBuffer = byteBuffer.asFloatBuffer();
            normalBuffer.put(sphere_normals);
            normalBuffer.position(0);

            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, sphere_normals.length * 4, normalBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);

            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
            
            //element vbo
            GLES32.glGenBuffers(1, vbo_sphere_elements, 0);
            GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        
            byteBuffer = ByteBuffer.allocateDirect(sphere_elements.length * 4);
            byteBuffer.order(ByteOrder.nativeOrder());
            ShortBuffer elementBuffer = byteBuffer.asShortBuffer();
            elementBuffer.put(sphere_elements);
            elementBuffer.position(0);

            GLES32.glBufferData(GLES32.GL_ELEMENT_ARRAY_BUFFER, sphere_elements.length * 2, elementBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        //OpenGL-ES states 
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);
        GLES32.glEnable(GLES32.GL_CULL_FACE);

        //set background clearing color
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        //set projection matrix to identity
        Matrix.setIdentityM(perspectiveProjectionMatrix, 0);
    }

    private void resize(int width, int height)
    {
        if(height == 0)
        {
            height = 1;
        }

        GLES32.glViewport(0, 0, width, height);

        Matrix.perspectiveM(perspectiveProjectionMatrix, 0, 45.0f, (float)width / (float)height, 0.1f, 100.0f);
    }

    private void render()
    {
        float modelMatrix[] = new float[16];
        float viewMatrix[] = new float[16];

        //clear buffers
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgramObject);

        //set matrices to identity
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);
        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -3.0f);

        //pass the uniform variables to shaders
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

        if(double_tap == 1)
        {
            GLES32.glUniform1i(doubleTapUniform, 1);
            GLES32.glUniform3f(lightAmbientUniform, 0.0f, 0.0f, 0.0f);
            GLES32.glUniform3f(lightDiffuseUniform, 1.0f, 1.0f, 1.0f);
            GLES32.glUniform3f(lightSpecularUniform, 1.0f, 1.0f, 1.0f);
            GLES32.glUniform4f(lightPositionUniform, 100.0f, 100.0f, 100.0f, 1.0f);
            GLES32.glUniform3f(materialAmbientUniform, 0.0f, 0.0f, 0.0f);
            GLES32.glUniform3f(materialDiffuseUniform, 1.0f, 1.0f, 1.0f);
            GLES32.glUniform3f(materialSpecularUniform, 1.0f, 1.0f, 1.0f);
            GLES32.glUniform1f(materialShininessUniform, 50.0f);
        }
        else
        {
            GLES32.glUniform1i(doubleTapUniform, 0);
        }

        GLES32.glBindVertexArray(vao_sphere[0]);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        GLES32.glBindVertexArray(0);
        GLES32.glUseProgram(0);

        requestRender();
    }

    private void uninitialize()
    {
        if(vao_sphere[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_sphere, 0);
            vao_sphere[0] = 0;
        }

        if(vbo_sphere_vertices[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_vertices, 0);
            vbo_sphere_vertices[0] = 0;
        }

        if(vbo_sphere_normals[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_normals, 0);
            vbo_sphere_normals[0] = 0;
        }

        if(vbo_sphere_elements[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_elements, 0);
            vbo_sphere_elements[0] = 0;
        }

        if(shaderProgramObject != 0)
        {
            if(vertexShaderObject != 0)
            {
                GLES32.glDetachShader(shaderProgramObject, vertexShaderObject);
                GLES32.glDeleteShader(vertexShaderObject);
                vertexShaderObject = 0;
            }
            
            if(fragmentShaderObject != 0)
            {
                GLES32.glDetachShader(shaderProgramObject, fragmentShaderObject);
                GLES32.glDeleteShader(fragmentShaderObject);
                fragmentShaderObject = 0;
            }
        
            GLES32.glDeleteProgram(shaderProgramObject);
            shaderProgramObject = 0;
        }
    }
}
