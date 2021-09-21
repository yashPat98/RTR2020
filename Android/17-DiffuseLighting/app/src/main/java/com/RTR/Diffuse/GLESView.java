package com.RTR.Diffuse;

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

    private int modelViewMatrixUniform;
    private int projectionMatrixUniform;
    private int doubleTapUniform;
    private int lightPositionUniform;
    private int lightDiffuseUniform;
    private int materialDiffuseUniform;

    private int[] vao_cube = new int[1];
    private int[] vbo_cube_vertices = new int[1];
    private int[] vbo_cube_normals = new int[1];

    private float perspectiveProjectionMatrix[] = new float[16];
    private int double_tap = 0;

    private float cube_rotation_angle = 0.0f;

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
            "#version 320 es"                                                                           +
            "\n"                                                                                        +
            
            "in vec4 vPosition;"                                                                        +
            "in vec3 vNormal;"                                                                          +

            "uniform mat4 u_modelViewMatrix;"                                                           +
            "uniform mat4 u_projectionMatrix;"                                                          +
            "uniform int u_doubleTap;"                                                                  +   
            "uniform vec3 u_lightDiffuse;"                                                              +
            "uniform vec3 u_materialDiffuse;"                                                           +
            "uniform vec4 u_lightPosition;"                                                             +

            "out vec3 diffuse_light;"                                                                   +

            "void main(void)"                                                                           +
            "{"                                                                                         +
            "   if(u_doubleTap == 1)"                                                                   +
            "   {"                                                                                      +
            "       vec4 eye_coords = u_modelViewMatrix * vPosition;"                                   +
            "       mat3 normal_matrix = mat3(transpose(inverse(u_modelViewMatrix)));"                  +
            "       vec3 tnorm = normalize(normal_matrix * vNormal);"                                   +
            "       vec3 s = normalize(vec3(u_lightPosition - eye_coords));"                            +
            "       diffuse_light = u_lightDiffuse * u_materialDiffuse * max(dot(s, tnorm), 0.0f);"     +
            "   }"                                                                                      +

            "   gl_Position = u_projectionMatrix * u_modelViewMatrix * vPosition;"                      +
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
            "in vec3 diffuse_light;"                            +
            "uniform int u_doubleTap;"                          +
            "out vec4 FragColor;"                               +
            "void main(void)"                                   +
            "{"                                                 +
            "   if(u_doubleTap == 1)"                           +
            "   {"                                              +
            "       FragColor = vec4(diffuse_light, 1.0f);"     +
            "   }"                                              +
            "   else"                                           +
            "   {"                                              +
            "       FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);"  +
            "   }"                                              +
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
        modelViewMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_modelViewMatrix");
        projectionMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");
        lightPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lightPosition");
        lightDiffuseUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lightDiffuse");
        materialDiffuseUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_materialDiffuse");
        doubleTapUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_doubleTap");

        //vertex data
        final float cubeVertices[] = new float[]
        {
            //near 
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f, 
            1.0f, -1.0f, 1.0f,

            //right
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,

            //far
            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,

            //left
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f, 
            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,

            //top
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,

            //bottom
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f
        };

        //normal data
        final float cubeNormals[] = new float[] 
        {
            //near 
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
    
            //right
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
    
            //far
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
    
            //left
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
    
            //top 
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
    
            //bottom
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f
        };

        //setup vao and vbo
        GLES32.glGenVertexArrays(1, vao_cube, 0);
        GLES32.glBindVertexArray(vao_cube[0]);
            GLES32.glGenBuffers(1, vbo_cube_vertices, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_cube_vertices[0]);

            ByteBuffer byteBuffer = ByteBuffer.allocateDirect(cubeVertices.length * 4);
            byteBuffer.order(ByteOrder.nativeOrder());
            FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
            verticesBuffer.put(cubeVertices);
            verticesBuffer.position(0);

            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeVertices.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_VERTEX);

            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        
            GLES32.glGenBuffers(1, vbo_cube_normals, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_cube_normals[0]);

            byteBuffer = ByteBuffer.allocateDirect(cubeNormals.length * 4);
            byteBuffer.order(ByteOrder.nativeOrder());
            FloatBuffer normalBuffer = byteBuffer.asFloatBuffer();
            normalBuffer.put(cubeNormals);
            normalBuffer.position(0);

            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeNormals.length * 4, normalBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);

            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
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
        float modelViewMatrix[] = new float[16];
        float rotationMatrix[] = new float[16];

        //clear buffers
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgramObject);

        //set matrices to identity
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(rotationMatrix, 0);

        Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -5.0f);

        Matrix.setRotateM(rotationMatrix, 0, cube_rotation_angle, 1.0f, 0.0f, 0.0f);
        Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, rotationMatrix, 0);
        Matrix.setRotateM(rotationMatrix, 0, cube_rotation_angle, 0.0f, 1.0f, 0.0f);
        Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, rotationMatrix, 0);
        Matrix.setRotateM(rotationMatrix, 0, cube_rotation_angle, 0.0f, 0.0f, 1.0f);
        Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, rotationMatrix, 0);

        //pass the uniform variables to shaders
        GLES32.glUniformMatrix4fv(modelViewMatrixUniform, 1, false, modelViewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

        if(double_tap == 1)
        {
            GLES32.glUniform1i(doubleTapUniform, 1);
            GLES32.glUniform4f(lightPositionUniform, 0.0f, 0.0f, 2.0f, 1.0f);
            GLES32.glUniform3f(lightDiffuseUniform, 1.0f, 1.0f, 1.0f);
            GLES32.glUniform3f(materialDiffuseUniform, 0.5f, 0.5f, 0.5f);
        }
        else
        {
            GLES32.glUniform1i(doubleTapUniform, 0);
        }

        GLES32.glBindVertexArray(vao_cube[0]);

        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 4, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 8, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 12, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 16, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 20, 4);

        GLES32.glBindVertexArray(0);
        GLES32.glUseProgram(0);

        //update
        cube_rotation_angle += 0.5f;
        if(cube_rotation_angle >= 360.0f)
            cube_rotation_angle = 0.0f;

        requestRender();
    }

    private void uninitialize()
    {
        if(vao_cube[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_cube, 0);
            vao_cube[0] = 0;
        }

        if(vbo_cube_vertices[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_cube_vertices, 0);
            vbo_cube_vertices[0] = 0;
        }

        if(vbo_cube_normals[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_cube_normals, 0);
            vbo_cube_normals[0] = 0;
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
