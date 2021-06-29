package com.RTR.Smiley;

import android.content.Context;

import android.opengl.GLES32;
import android.opengl.GLSurfaceView;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGLConfig;

import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.opengl.GLUtils;

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

    private int[] vao_square = new int[1];
    private int[] vbo_square_vertices = new int[1];
    private int[] vbo_square_texcoords = new int[1];

    private int smiley_texture;

    private int mvpUniform;
    private int textureSamplerUniform;
    private int textureToggleUniform;

    private float perspectiveProjectionMatrix[] = new float[16];

    private int single_tap = 0;

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
        single_tap++;
        if(single_tap > 4)
        {
            single_tap = 0;
        }

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
            "in vec2 vTexCoord;"                            +
            "uniform mat4 u_mvpMatrix;"                     +
            "out vec2 out_texcoord;"                        +
            "void main(void)"                               +
            "{"                                             +
            "   gl_Position = u_mvpMatrix * vPosition;"     +
            "   out_texcoord = vTexCoord;"                  +
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
            "#version 320 es"                                                   +
            "\n"                                                                +
            "precision highp float;"                                            +
            "in vec2 out_texcoord;"                                             +

            "uniform sampler2D u_textureSampler;"                               +
            "uniform int u_textureToggle;"                                      +

            "out vec4 FragColor;"                                               +

            "void main(void)"                                                   +
            "{"                                                                 +
            "   if(u_textureToggle == 0)"                                       +
            "   {"                                                              +
            "       FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);"                  +
            "   }"                                                              +
            "   else"                                                           +
            "   {"                                                              +
            "       FragColor = texture(u_textureSampler, out_texcoord);"       +
            "   }"                                                              +
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
        GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_TEXCOORD, "vTexCoord");

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
        textureSamplerUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_textureSampler");
        textureToggleUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_textureToggle");

        //pyramid vertex data
        final float squareVertices[] = new float[]
        {
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f, 
            1.0f, -1.0f, 0.0f
        };

        //setup vao and vbo
        GLES32.glGenVertexArrays(1, vao_square, 0);
        GLES32.glBindVertexArray(vao_square[0]);
            GLES32.glGenBuffers(1, vbo_square_vertices, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_square_vertices[0]);

            ByteBuffer byteBuffer = ByteBuffer.allocateDirect(squareVertices.length * 4);
            byteBuffer.order(ByteOrder.nativeOrder());
            FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
            verticesBuffer.put(squareVertices);
            verticesBuffer.position(0);

            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, squareVertices.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_VERTEX);

            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        
            GLES32.glGenBuffers(1, vbo_square_texcoords, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_square_texcoords[0]);

            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, 8 * 4, null, GLES32.GL_DYNAMIC_DRAW);
            GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_TEXCOORD, 2, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_TEXCOORD);

            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        //OpenGL-ES states 
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        //set background clearing color
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        //load textures
        smiley_texture = loadGLTexture(R.raw.smiley);

        //set projection matrix to identity
        Matrix.setIdentityM(perspectiveProjectionMatrix, 0);
    }

    private int loadGLTexture(int imageFileResourceID)
    {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;
        Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(), imageFileResourceID, options);

        int texture[] = new int[1];

        GLES32.glPixelStorei(GLES32.GL_UNPACK_ALIGNMENT, 1);
        GLES32.glGenTextures(1, texture, 0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texture[0]);
    
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MAG_FILTER, GLES32.GL_LINEAR);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MIN_FILTER, GLES32.GL_LINEAR_MIPMAP_LINEAR);

        //push the data to texture memory
        GLUtils.texImage2D(GLES32.GL_TEXTURE_2D, 0, bitmap, 0);
        GLES32.glGenerateMipmap(GLES32.GL_TEXTURE_2D);

        return (texture[0]);
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
        float squareTexCoords[] = new float[8];

        //code
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgramObject);

        //Square
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);

        Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -3.0f);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
    
        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);

        GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, smiley_texture);
        GLES32.glUniform1i(textureSamplerUniform, 0);
        
        if(single_tap == 0)
        {   
            GLES32.glUniform1i(textureToggleUniform, 0);
        }
        else 
        {
            GLES32.glUniform1i(textureToggleUniform, 1);
        }

        GLES32.glBindVertexArray(vao_square[0]);

        switch(single_tap)
        {
            case 1:
                squareTexCoords[0] = 1.0f;
                squareTexCoords[1] = 1.0f;

                squareTexCoords[2] = 0.0f;
                squareTexCoords[3] = 1.0f;

                squareTexCoords[4] = 0.0f;
                squareTexCoords[5] = 0.0f;

                squareTexCoords[6] = 1.0f;
                squareTexCoords[7] = 0.0f;
                break;
        
            case 2:
                squareTexCoords[0] = 0.5f;
                squareTexCoords[1] = 0.5f;

                squareTexCoords[2] = 0.0f;
                squareTexCoords[3] = 0.5f;

                squareTexCoords[4] = 0.0f;
                squareTexCoords[5] = 0.0f;

                squareTexCoords[6] = 0.5f;
                squareTexCoords[7] = 0.0f;
                break;

            case 3:
                squareTexCoords[0] = 2.0f;
                squareTexCoords[1] = 2.0f;

                squareTexCoords[2] = 0.0f;
                squareTexCoords[3] = 2.0f;

                squareTexCoords[4] = 0.0f;
                squareTexCoords[5] = 0.0f;

                squareTexCoords[6] = 2.0f;
                squareTexCoords[7] = 0.0f;
                break;

            case 4:
                squareTexCoords[0] = 0.5f;
                squareTexCoords[1] = 0.5f;

                squareTexCoords[2] = 0.5f;
                squareTexCoords[3] = 0.5f;

                squareTexCoords[4] = 0.5f;
                squareTexCoords[5] = 0.5f;

                squareTexCoords[6] = 0.5f;
                squareTexCoords[7] = 0.5f;
                break;

            default:
                break;
        }

        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(squareTexCoords.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer texcoordBuffer = byteBuffer.asFloatBuffer();
        texcoordBuffer.put(squareTexCoords);
        texcoordBuffer.position(0);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_square_texcoords[0]);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, squareTexCoords.length * 4, texcoordBuffer, GLES32.GL_DYNAMIC_DRAW);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
        
        //unbind
        GLES32.glBindVertexArray(0);
        GLES32.glUseProgram(0);

        requestRender();
    }

    private void uninitialize()
    {
        if(vao_square[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_square, 0);
            vao_square[0] = 0;
        }

        if(vbo_square_vertices[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_square_vertices, 0);
            vbo_square_vertices[0] = 0;
        }

        if(vbo_square_texcoords[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_square_texcoords, 0);
            vbo_square_texcoords[0] = 0;
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
