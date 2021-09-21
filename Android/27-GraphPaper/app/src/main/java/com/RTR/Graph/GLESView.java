package com.RTR.Graph;

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
import java.util.Vector;

//view for OpenGLES which also recives touch events
public class GLESView extends GLSurfaceView 
                      implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener
{
    private final Context context;
    private GestureDetector gestureDetector;

    private int vertexShaderObject;
    private int fragmentShaderObject;
    private int shaderProgramObject;

    private int[] vao = new int[1];
    private int[] vbo = new int[1];

    private int [] vao_x = new int[1];
    private int [] vbo_x_position = new int[1];

    private int [] vao_y = new int[1];
    private int [] vbo_y_position = new int[1];

    private int [] vao_circle = new int[1];
    private int [] vbo_circle_position = new int[1];

    private int [] vao_square = new int[1];
    private int [] vbo_square_position = new int[1];

    private int [] vao_triangle = new int[1];
    private int [] vbo_triangle_position = new int[1];

    private int [] vao_incircle = new int[1];
    private int [] vbo_incircle_position = new int[1];

    private int mvpUniform;
    private int colorUniform;

    private float perspectiveProjectionMatrix[] = new float[16];

    private int vertices_line_count;
    private int vertices_circle_count;
    private int vertices_incircle_count;

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
            "uniform mat4 u_mvpMatrix;"                     +
            "void main(void)"                               +
            "{"                                             +
            "   gl_Position = u_mvpMatrix * vPosition;"     +
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
            "out vec4 FragColor;"                               +
            "uniform vec3 color;"                               +
            "void main(void)"                                   +
            "{"                                                 +
            "   FragColor = vec4(color, 1.0f);"                 +
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
        colorUniform = GLES32.glGetUniformLocation(shaderProgramObject, "color");

        //vertex data
        Vector vertices_line_buffer = new Vector();
        float fInterval = 0.05f;
        for(float fStep = -20.0f; fStep <= 20.0f; fStep++)
        {   
            vertices_line_buffer.add(-1.0f);
            vertices_line_buffer.add(fInterval * fStep);
            vertices_line_buffer.add(0.0f);

            vertices_line_buffer.add(1.0f);
            vertices_line_buffer.add(fInterval * fStep);
            vertices_line_buffer.add(0.0f);

            vertices_line_buffer.add(fInterval * fStep);
            vertices_line_buffer.add(-1.0f);
            vertices_line_buffer.add(0.0f);

            vertices_line_buffer.add(fInterval * fStep);
            vertices_line_buffer.add(1.0f);
            vertices_line_buffer.add(0.0f);
        }
        vertices_line_count = vertices_line_buffer.size();
        float vertices_line[] = new float[vertices_line_count];
        for(int i = 0; i < vertices_line_count; i++)
        {
            vertices_line[i] = (float)vertices_line_buffer.get(i);
        }
        vertices_line_buffer.clear();

        final float x_axis[] = new float[]
        {
            -1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f
        };

        final float y_axis[] = new float[]
        {
            0.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        };
        
        final float square[] = 
        {
            (float)Math.cos(0.785375), (float)Math.sin(0.785375), 0.0f,
            (float)Math.cos(Math.PI - 0.785375), (float)Math.sin(Math.PI - 0.785375), 0.0f,
            -(float)Math.cos(0.785375), -(float)Math.sin(0.785375), 0.0f,
            (float)Math.sin(Math.PI - 0.785375), (float)Math.cos(Math.PI - 0.785375), 0.0f
        };

        final float triangle[] = 
        {
            0.0f, ((float)Math.cos(0.785375) - (float)Math.cos(Math.PI - 0.785375)) / 2.0f, 0.0f,
            -(float)Math.cos(0.785375), -(float)Math.sin(0.785375), 0.0f,
            (float)Math.sin(Math.PI - 0.785375), (float)Math.cos(Math.PI - 0.785375), 0.0f
        };

        Vector vertices_circle_buffer = new Vector();
        for(float angle = 0.0f; angle <= (2.0f * Math.PI); angle += 0.1f)
        {
            float x = (float)Math.sin(angle);
            float y = (float)Math.cos(angle);
    
            vertices_circle_buffer.add(x);
            vertices_circle_buffer.add(y);
            vertices_circle_buffer.add(0.0f);        
        }
        vertices_circle_count = vertices_circle_buffer.size();
        float vertices_circle[] = new float[vertices_circle_count];
        for(int i = 0; i < vertices_circle_count; i++)
        {
            vertices_circle[i] = (float)vertices_circle_buffer.get(i);
        }
        vertices_circle_buffer.clear();

        //incircle
        float lab = distance(0.0f, ((float)Math.cos(0.785375) - (float)Math.cos(Math.PI - 0.785375)) / 2.0f, -(float)Math.cos(0.785375), -(float)Math.sin(0.785375));
        float lbc = distance(-(float)Math.cos(0.785375), -(float)Math.sin(0.785375), (float)Math.sin(Math.PI - 0.785375), (float)Math.cos(Math.PI - 0.785375));
        float lac = distance(0.0f, ((float)Math.cos(0.785375) - (float)Math.cos(Math.PI - 0.785375)) / 2.0f, (float)Math.sin(Math.PI - 0.785375), (float)Math.cos(Math.PI - 0.785375));
        float sum = lab + lbc + lac;

        float xin = ((lbc * 0.0f) + (lac * (-(float)Math.cos(0.785375))) + (lab * (float)Math.sin(Math.PI - 0.785375))) / sum;
        float yin = ((lbc * (((float)Math.cos(0.785375) - (float)Math.cos(Math.PI - 0.785375)) / 2.0f)) + (lac * (-(float)Math.sin(0.785375))) + (lab * (float)Math.cos((float)Math.PI - 0.785375))) / sum;

        //radius of incircle = area / semi-perimeter;
        float semi = (lab + lbc + lac) / 2;
        float radius = (float)Math.sqrt(semi * (semi - lab) * (semi - lbc) * (semi - lac)) / semi;

        Vector vertices_incircle_buffer = new Vector();
        for(float angle = 0.0f; angle <= (2 * Math.PI); angle += 0.1f)
        {
            float x = radius * (float)Math.sin(angle);
            float y = radius * (float)Math.cos(angle);

            vertices_incircle_buffer.add(x + xin);
            vertices_incircle_buffer.add(y + yin);
            vertices_incircle_buffer.add(0.0f);
        }
        vertices_incircle_count = vertices_incircle_buffer.size();
        float vertices_incircle[] = new float[vertices_incircle_count];
        for(int i = 0; i < vertices_incircle_count; i++)
        {
            vertices_incircle[i] = (float)vertices_incircle_buffer.get(i);
        }
        vertices_incircle_buffer.clear();

        //setup vao and vbo
        GLES32.glGenVertexArrays(1, vao, 0);
        GLES32.glBindVertexArray(vao[0]);
            GLES32.glGenBuffers(1, vbo, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo[0]);
                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(vertices_line.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
                verticesBuffer.put(vertices_line);
                verticesBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, vertices_line.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 3, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_VERTEX);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        GLES32.glGenVertexArrays(1, vao_x, 0);
        GLES32.glBindVertexArray(vao_x[0]);
            GLES32.glGenBuffers(1, vbo_x_position, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_x_position[0]);
                byteBuffer = ByteBuffer.allocateDirect(x_axis.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                verticesBuffer = byteBuffer.asFloatBuffer();
                verticesBuffer.put(x_axis);
                verticesBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, x_axis.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 3, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_VERTEX);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        GLES32.glGenVertexArrays(1, vao_y, 0);
        GLES32.glBindVertexArray(vao_y[0]);
            GLES32.glGenBuffers(1, vbo_y_position, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_y_position[0]);
                byteBuffer = ByteBuffer.allocateDirect(y_axis.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                verticesBuffer = byteBuffer.asFloatBuffer();
                verticesBuffer.put(y_axis);
                verticesBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, y_axis.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 3, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_VERTEX);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        GLES32.glGenVertexArrays(1, vao_circle, 0);
        GLES32.glBindVertexArray(vao_circle[0]);
            GLES32.glGenBuffers(1, vbo_circle_position, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_circle_position[0]);
                byteBuffer = ByteBuffer.allocateDirect(vertices_circle.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                verticesBuffer = byteBuffer.asFloatBuffer();
                verticesBuffer.put(vertices_circle);
                verticesBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, vertices_circle.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 3, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_VERTEX);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        GLES32.glGenVertexArrays(1, vao_square, 0);
        GLES32.glBindVertexArray(vao_square[0]);
            GLES32.glGenBuffers(1, vbo_square_position, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_square_position[0]);
                byteBuffer = ByteBuffer.allocateDirect(square.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                verticesBuffer = byteBuffer.asFloatBuffer();
                verticesBuffer.put(square);
                verticesBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, square.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 3, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_VERTEX);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        GLES32.glGenVertexArrays(1, vao_triangle, 0);
        GLES32.glBindVertexArray(vao_triangle[0]);
            GLES32.glGenBuffers(1, vbo_triangle_position, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_triangle_position[0]);
                byteBuffer = ByteBuffer.allocateDirect(triangle.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                verticesBuffer = byteBuffer.asFloatBuffer();
                verticesBuffer.put(triangle);
                verticesBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, triangle.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 3, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_VERTEX);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        GLES32.glGenVertexArrays(1, vao_incircle, 0);
        GLES32.glBindVertexArray(vao_incircle[0]);
            GLES32.glGenBuffers(1, vbo_incircle_position, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_incircle_position[0]);
                byteBuffer = ByteBuffer.allocateDirect(vertices_incircle.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                verticesBuffer = byteBuffer.asFloatBuffer();
                verticesBuffer.put(vertices_incircle);
                verticesBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, vertices_incircle.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 3, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_VERTEX);
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

    private float distance(float x1, float y1, float x2, float y2)
    {
        //code
        float result = ((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1));
        return ((float)Math.sqrt(result));
    }

    private void render()
    {
        float modelMatrix[] = new float[16];
        float viewMatrix[] = new float[16];
        float modelViewProjectionMatrix[] = new float[16];

        //clear buffers
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgramObject);
            Matrix.setIdentityM(modelMatrix, 0);
            Matrix.setIdentityM(viewMatrix, 0);
            Matrix.setIdentityM(modelViewProjectionMatrix, 0);

            Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -2.5f);
            Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, viewMatrix, 0);
            Matrix.multiplyMM(modelViewProjectionMatrix, 0, modelViewProjectionMatrix, 0, modelMatrix, 0);

            GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);
            GLES32.glUniform3f(colorUniform, 0.0f, 0.0f, 1.0f);

            GLES32.glBindVertexArray(vao[0]);
            GLES32.glDrawArrays(GLES32.GL_LINES, 0, vertices_line_count / 3);
            GLES32.glBindVertexArray(0);

            GLES32.glUniform3f(colorUniform, 1.0f, 0.0f, 0.0f);
            GLES32.glBindVertexArray(vao_x[0]);
            GLES32.glDrawArrays(GLES32.GL_LINES, 0, 2);
            GLES32.glBindVertexArray(0);

            GLES32.glUniform3f(colorUniform, 0.0f, 1.0f, 0.0f);
            GLES32.glBindVertexArray(vao_y[0]);
            GLES32.glDrawArrays(GLES32.GL_LINES, 0, 2);
            GLES32.glBindVertexArray(0);

            GLES32.glUniform3f(colorUniform, 1.0f, 1.0f, 0.0f);
            GLES32.glBindVertexArray(vao_circle[0]);
            GLES32.glDrawArrays(GLES32.GL_LINE_LOOP, 0, vertices_circle_count/3);
            GLES32.glBindVertexArray(0);

            GLES32.glBindVertexArray(vao_square[0]);
            GLES32.glDrawArrays(GLES32.GL_LINE_LOOP, 0, 4);
            GLES32.glBindVertexArray(0);

            GLES32.glBindVertexArray(vao_triangle[0]);
            GLES32.glDrawArrays(GLES32.GL_LINE_LOOP, 0, 3);
            GLES32.glBindVertexArray(0);

            GLES32.glBindVertexArray(vao_incircle[0]);
            GLES32.glDrawArrays(GLES32.GL_LINE_LOOP, 0, vertices_incircle_count/3);
            GLES32.glBindVertexArray(0);
        GLES32.glUseProgram(0);

        requestRender();
    }

    private void uninitialize()
    {
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
