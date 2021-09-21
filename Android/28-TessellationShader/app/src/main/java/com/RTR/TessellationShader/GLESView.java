package com.RTR.TessellationShader;

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
    private int tessellationControlShaderObject;
    private int tessellationEvaluationShaderObject;
    private int fragmentShaderObject;
    private int shaderProgramObject;

    private int[] vao = new int[1];
    private int[] vbo = new int[1];
    private int mvpUniform;
    private int numberOfSegmentsUniform;
    private int numberOfStripsUniform;
    private int lineColorUniform;

    private float perspectiveProjectionMatrix[] = new float[16];
    private int iNumberOfSegments = 1;

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
        iNumberOfSegments++;
        if(iNumberOfSegments > 30)
            iNumberOfSegments = 30;
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
        iNumberOfSegments--;
        if(iNumberOfSegments < 1)
            iNumberOfSegments = 1;
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
            "in vec2 vPosition;"                                +
            "void main(void)"                                   +
            "{"                                                 +
            "   gl_Position =  vec4(vPosition, 0.0f, 1.0f);"    +
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

        //tessellation control shader

        //create shader
        tessellationControlShaderObject = GLES32.glCreateShader(GLES32.GL_TESS_CONTROL_SHADER);

        //fragment shader source code
        final String tessellationControlShaderSourceCode = String.format
        (
            "#version 320 es"                                                               +
            "\n"                                                                            +
            "layout(vertices = 4)out;"                                                      +
            "uniform int u_numberOfSegments;"                                               +
            "uniform int u_numberOfStrips;"                                                 +

            "void main(void)"                                                               +
            "{"                                                                             +
            "   gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;"  +
            "   gl_TessLevelOuter[0] = float(u_numberOfStrips);"                            +
            "   gl_TessLevelOuter[1] = float(u_numberOfSegments);"                          +
            "}"
        );

        //provide source code to shader
        GLES32.glShaderSource(tessellationControlShaderObject, tessellationControlShaderSourceCode);

        //compile shader
        GLES32.glCompileShader(tessellationControlShaderObject);

        //error checking
        iShaderCompiledStatus[0] = 0;
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetShaderiv(tessellationControlShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompiledStatus, 0);
        if(iShaderCompiledStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(tessellationControlShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(tessellationControlShaderObject);
                System.out.println("YIP: Tessellation Control Shader Compilation Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //tessellation evaluation shader

        //create shader
        tessellationEvaluationShaderObject = GLES32.glCreateShader(GLES32.GL_TESS_EVALUATION_SHADER);

        //fragment shader source code
        final String tessellationEvaluationShaderSourceCode = String.format
        (
            "#version 320 es"                                       +
            "\n"                                                    +
            "layout(isolines)in;"                                   +
            "uniform mat4 u_mvpMatrix;"                             +
            "void main(void)"                                       +
            "{"                                                     +
            "   float tessCoord = gl_TessCoord.x;"                  +
            "   vec3 p0 = gl_in[0].gl_Position.xyz;"                +
            "   vec3 p1 = gl_in[1].gl_Position.xyz;"                +
            "   vec3 p2 = gl_in[2].gl_Position.xyz;"                +
            "   vec3 p3 = gl_in[3].gl_Position.xyz;"                +
            "   vec3 p = (p0 * (1.0f - tessCoord) * (1.0f - tessCoord) * (1.0f - tessCoord)) + (p1 * 3.0f * tessCoord * (1.0f - tessCoord) * (1.0f - tessCoord)) + (p2 * 3.0f * tessCoord * tessCoord * (1.0f - tessCoord)) + (p3 * tessCoord * tessCoord * tessCoord);"    +
            "   gl_Position = u_mvpMatrix * vec4(p, 1.0f);"         +
            "}"
        );

        //provide source code to shader
        GLES32.glShaderSource(tessellationEvaluationShaderObject, tessellationEvaluationShaderSourceCode);

        //compile shader
        GLES32.glCompileShader(tessellationEvaluationShaderObject);

        //error checking
        iShaderCompiledStatus[0] = 0;
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetShaderiv(tessellationEvaluationShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompiledStatus, 0);
        if(iShaderCompiledStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(tessellationEvaluationShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(tessellationEvaluationShaderObject);
                System.out.println("YIP: Tessellation Evaluation Shader Compilation Log = " + szInfoLog);
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
            "uniform vec4 u_lineColor;"                         +
            "void main(void)"                                   +
            "{"                                                 +
            "   FragColor = u_lineColor;"                       +
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

        //attach shaders to shader program
        GLES32.glAttachShader(shaderProgramObject, vertexShaderObject); 
        GLES32.glAttachShader(shaderProgramObject, tessellationControlShaderObject); 
        GLES32.glAttachShader(shaderProgramObject, tessellationEvaluationShaderObject); 
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
        numberOfSegmentsUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_numberOfSegments");
        numberOfStripsUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_numberOfStrips");
        lineColorUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lineColor");

        //vertex data
        final float vertices[] = new float[]
        {
            -1.0f, -1.0f,
            -0.5f, 1.0f,
            0.5f, -1.0f,
            1.0f, 1.0f
        };

        //setup vao and vbo
        GLES32.glGenVertexArrays(1, vao, 0);
        GLES32.glBindVertexArray(vao[0]);

        GLES32.glGenBuffers(1, vbo, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo[0]);

        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(vertices.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
        verticesBuffer.put(vertices);
        verticesBuffer.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, vertices.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 2, GLES32.GL_FLOAT, false, 0, 0);
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

    private void render()
    {
        float modelViewMatrix[] = new float[16];
        float modelViewProjectionMatrix[] = new float[16];

        //clear buffers
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgramObject);
            Matrix.setIdentityM(modelViewMatrix, 0);
            Matrix.setIdentityM(modelViewProjectionMatrix, 0);
            Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -3.0f);
            Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
        
            GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);
            GLES32.glUniform1i(numberOfSegmentsUniform, iNumberOfSegments);
            GLES32.glUniform1i(numberOfStripsUniform, 1);
            GLES32.glUniform4f(lineColorUniform, 1.0f, 1.0f, 1.0f, 1.0f);

            GLES32.glBindVertexArray(vao[0]);
            GLES32.glPatchParameteri(GLES32.GL_PATCH_VERTICES, 4);
            GLES32.glDrawArrays(GLES32.GL_PATCHES, 0, 4);
            GLES32.glBindVertexArray(0);
        GLES32.glUseProgram(0);

        //flush buffers
        requestRender();
    }

    private void uninitialize()
    {
        if(vao[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao, 0);
            vao[0] = 0;
        }

        if(vbo[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo, 0);
            vbo[0] = 0;
        }

        if(shaderProgramObject != 0)
        {
            if(vertexShaderObject != 0)
            {
                GLES32.glDetachShader(shaderProgramObject, vertexShaderObject);
                GLES32.glDeleteShader(vertexShaderObject);
                vertexShaderObject = 0;
            }
            
            if(tessellationControlShaderObject != 0)
            {
                GLES32.glDetachShader(shaderProgramObject, tessellationControlShaderObject);
                GLES32.glDeleteShader(tessellationControlShaderObject);
                tessellationControlShaderObject = 0;
            }

            if(tessellationEvaluationShaderObject != 0)
            {
                GLES32.glDetachShader(shaderProgramObject, tessellationEvaluationShaderObject);
                GLES32.glDeleteShader(tessellationEvaluationShaderObject);
                tessellationEvaluationShaderObject = 0;
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
