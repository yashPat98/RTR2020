package com.RTR.Animation;

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

    private int[] vao_pyramid = new int[1];
    private int[] vbo_pyramid_vertices = new int[1];
    private int[] vbo_pyramid_colors = new int[1];

    private int[] vao_cube = new int[1];
    private int[] vbo_cube_vertices = new int[1];
    private int[] vbo_cube_colors = new int[1];

    float pyramid_rotation_angle = 0.0f;
    float cube_rotation_angle = 0.0f;

    private int mvpUniform;

    private float perspectiveProjectionMatrix[] = new float[16];

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
            "#version 320 es"                               +
            "\n"                                            +
            "in vec4 vPosition;"                            +
            "in vec4 vColor;"                               +
            "uniform mat4 u_mvpMatrix;"                     +
            "out vec4 out_color;"                           +
            "void main(void)"                               +
            "{"                                             +
            "   gl_Position = u_mvpMatrix * vPosition;"     +
            "   out_color = vColor;"                        +
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
            "in vec4 out_color;"                                +
            "out vec4 FragColor;"                               +
            "void main(void)"                                   +
            "{"                                                 +
            "   FragColor = out_color;"                         +
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
        GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_COLOR, "vColor");

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

        //get MVP uniform location
        mvpUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_mvpMatrix");
    
        //pyramid vertex data
        final float pyramidVertices[] = new float[]
        {
            //near
            0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,

            //right
            0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
    
            //far
            0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,

            //left
            0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f
        };

        //pyramid color data
        final float pyramidColors[] = new float[]
        {
            //near 
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f,

            //right
            1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f,

            //far
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f,

            //left
            1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f
        };

        //cube vertex data
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

        //cube color data
        final float cubeColor[] = new float[]
        {
            //near
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,

            //right 
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,

            //far
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,

            //left
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,

            //top
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,

            //bottom
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f
        };

        //setup vao and vbo
        GLES32.glGenVertexArrays(1, vao_pyramid, 0);
        GLES32.glBindVertexArray(vao_pyramid[0]);
            GLES32.glGenBuffers(1, vbo_pyramid_vertices, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_pyramid_vertices[0]);

            ByteBuffer byteBuffer = ByteBuffer.allocateDirect(pyramidVertices.length * 4);
            byteBuffer.order(ByteOrder.nativeOrder());
            FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
            verticesBuffer.put(pyramidVertices);
            verticesBuffer.position(0);

            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, pyramidVertices.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_VERTEX);

            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        
            GLES32.glGenBuffers(1, vbo_pyramid_colors, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_pyramid_colors[0]);

            byteBuffer = ByteBuffer.allocateDirect(pyramidColors.length * 4);
            byteBuffer.order(ByteOrder.nativeOrder());
            FloatBuffer colorBuffer = byteBuffer.asFloatBuffer();
            colorBuffer.put(pyramidColors);
            colorBuffer.position(0);

            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, pyramidColors.length * 4, colorBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);

            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        GLES32.glGenVertexArrays(1, vao_cube, 0);
        GLES32.glBindVertexArray(vao_cube[0]);
            GLES32.glGenBuffers(1, vbo_cube_vertices, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_cube_vertices[0]);

            byteBuffer = ByteBuffer.allocateDirect(cubeVertices.length * 4);
            byteBuffer.order(ByteOrder.nativeOrder());
            verticesBuffer = byteBuffer.asFloatBuffer();
            verticesBuffer.put(cubeVertices);
            verticesBuffer.position(0);

            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeVertices.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_VERTEX);

            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        
            GLES32.glGenBuffers(1, vbo_cube_colors, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_cube_colors[0]);

            byteBuffer = ByteBuffer.allocateDirect(cubeColor.length * 4);
            byteBuffer.order(ByteOrder.nativeOrder());
            colorBuffer = byteBuffer.asFloatBuffer();
            colorBuffer.put(cubeColor);
            colorBuffer.position(0);

            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeColor.length * 4, colorBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);

            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        //OpenGL-ES states 
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

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
        //variable declarations
        float modelViewMatrix[] = new float[16];
        float modelViewProjectionMatrix[] = new float[16];
        float translateMatrix[] = new float[16];
        float rotationMatrix_x[] = new float[16];
        float rotationMatrix_y[] = new float[16];
        float rotationMatrix_z[] = new float[16];
        float scaleMatrix[] = new float[16];

        //code
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgramObject);

        //Pyramid
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
        Matrix.setIdentityM(translateMatrix, 0);
        Matrix.setIdentityM(rotationMatrix_y, 0);

        Matrix.translateM(translateMatrix, 0, -1.5f, 0.0f, -6.0f);
        Matrix.setRotateM(rotationMatrix_y, 0, cube_rotation_angle, 0.0f, 1.0f, 0.0f);
        Matrix.multiplyMM(modelViewMatrix, 0, translateMatrix, 0, rotationMatrix_y, 0);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
    
        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);

        GLES32.glBindVertexArray(vao_pyramid[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 12);

        //Cube
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
        Matrix.setIdentityM(translateMatrix, 0);
        Matrix.setIdentityM(rotationMatrix_x, 0);
        Matrix.setIdentityM(rotationMatrix_y, 0);
        Matrix.setIdentityM(rotationMatrix_z, 0);
        Matrix.setIdentityM(scaleMatrix, 0);

        Matrix.translateM(translateMatrix, 0, 1.5f, 0.0f, -6.0f);
        Matrix.setRotateM(rotationMatrix_x, 0, cube_rotation_angle, 1.0f, 0.0f, 0.0f);
        Matrix.setRotateM(rotationMatrix_y, 0, cube_rotation_angle, 0.0f, 1.0f, 0.0f);
        Matrix.setRotateM(rotationMatrix_z, 0, cube_rotation_angle, 0.0f, 0.0f, 1.0f);
        Matrix.scaleM(scaleMatrix, 0, 0.85f, 0.85f, 0.85f);
        Matrix.multiplyMM(modelViewMatrix, 0, translateMatrix, 0, rotationMatrix_x, 0);
        Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, rotationMatrix_y, 0);
        Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, rotationMatrix_z, 0);
        Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, scaleMatrix, 0);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
    
        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);

        GLES32.glBindVertexArray(vao_cube[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 4, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 8, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 12, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 16, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 20, 4);

        //unbind
        GLES32.glBindVertexArray(0);
        GLES32.glUseProgram(0);

        //update 
        pyramid_rotation_angle += 0.5f;
        if(pyramid_rotation_angle >= 360.0f)
            pyramid_rotation_angle = 0.0f;
    
        cube_rotation_angle += 0.5f;
        if(cube_rotation_angle >= 360.0f)
            cube_rotation_angle = 0.0f;

        requestRender();
    }

    private void uninitialize()
    {
        if(vao_pyramid[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_pyramid, 0);
            vao_pyramid[0] = 0;
        }

        if(vbo_pyramid_vertices[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_pyramid_vertices, 0);
            vbo_pyramid_vertices[0] = 0;
        }

        if(vbo_pyramid_colors[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_pyramid_colors, 0);
            vbo_pyramid_colors[0] = 0;
        }
        
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

        if(vbo_cube_colors[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_cube_colors, 0);
            vbo_cube_colors[0] = 0;
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
