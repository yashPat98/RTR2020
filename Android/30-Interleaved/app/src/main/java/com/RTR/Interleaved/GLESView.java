package com.RTR.Interleaved;

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

    private int modelViewMatrixUniform;
    private int projectionMatrixUniform;
    private int lightPositionUniform;
    private int lightDiffuseUniform;
    private int diffuseTextureUniform;
    private int doubleTapUniform;
    private int singleTapUniform;       

    private int[] vao = new int[1];
    private int[] vbo = new int[1];

    private int[] marble_texture = new int[1];

    private float perspectiveProjectionMatrix[] = new float[16];
    private int double_tap = 0;
    private int single_tap = 0;

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
        if(single_tap == 0)
        {
            single_tap = 1;
        }
        else
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
            "#version 320 es"                                                                           +
            "\n"                                                                                        +
            
            "in vec3 vPosition;"                                                                        +
            "in vec3 vColor;"                                                                           +
            "in vec3 vNormal;"                                                                          +
            "in vec2 vTexCoord;"                                                                        +

            "uniform mat4 u_modelViewMatrix;"                                                           +
            "uniform mat4 u_projectionMatrix;"                                                          +   
            "uniform vec3 u_lightDiffuse;"                                                              +
            "uniform vec4 u_lightPosition;"                                                             +
            "uniform mediump int u_doubleTap;"                                                          +

            "out vec3 diffuse_light;"                                                                   +
            "out vec2 out_texcoord;"                                                                    +

            "void main(void)"                                                                           +
            "{"                                                                                         +
            "   if(u_doubleTap == 1)"                                                                   +
            "   {"                                                                                      +
            "       vec4 eye_coords = u_modelViewMatrix * vec4(vPosition, 1.0f);"                       +
            "       mat3 normal_matrix = mat3(transpose(inverse(u_modelViewMatrix)));"                  +
            "       vec3 tnorm = normalize(normal_matrix * vNormal);"                                   +
            "       vec3 s = normalize(vec3(u_lightPosition - eye_coords));"                            +
            "       diffuse_light = u_lightDiffuse * vColor * max(dot(s, tnorm), 0.0f);"                +
            "   }"                                                                                      +

            "   out_texcoord = vTexCoord;"                                                              +
            "   gl_Position = u_projectionMatrix * u_modelViewMatrix * vec4(vPosition, 1.0f);"          +
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
            
            "in vec3 diffuse_light;"                                            +
            "in vec2 out_texcoord;"                                             +
            "uniform sampler2D u_diffuseTexture;"                               +
            "uniform mediump int u_doubleTap;"                                  +         
            "uniform int u_singleTap;"                                          +
            "out vec4 FragColor;"                                               +
            
            "void main(void)"                                                   +
            "{"                                                                 +
            "   if(u_doubleTap == 1)"                                           +
            "   {"                                                              +
            "       FragColor = vec4(diffuse_light, 1.0f);"                     +
            "   }"                                                              +
            "   else"                                                           +
            "   {"                                                              +
            "       FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);"                  +
            "   }"                                                              +
            
            "   if(u_singleTap == 1)"                                           +
            "   {"                                                              +
            "       FragColor *= texture(u_diffuseTexture, out_texcoord);"      +
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
        GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_COLOR, "vColor");
        GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");
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

        //get uniform locations
        modelViewMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_modelViewMatrix");
        projectionMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");
        lightPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lightPosition");
        lightDiffuseUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lightDiffuse");
        diffuseTextureUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_diffuseTexture");
        doubleTapUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_doubleTap");
        singleTapUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_singleTap");

        //cube data
        final float cubePCNT[] = new float[]
        {
            //near 
            +1.0f, +1.0f, +1.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f, +1.0f,    +1.0f, +1.0f,   
            -1.0f, +1.0f, +1.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +1.0f,
            -1.0f, -1.0f, +1.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f,
            +1.0f, -1.0f, +1.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f, +1.0f,    +1.0f, +0.0f,

            //right
            +1.0f, +1.0f, -1.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f, +0.0f,    +1.0f, +1.0f,
            +1.0f, +1.0f, +1.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +1.0f,
            +1.0f, -1.0f, +1.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f,
            +1.0f, -1.0f, -1.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f, +0.0f,    +1.0f, +0.0f,

            //far
            -1.0f, +1.0f, -1.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f, -1.0f,    +1.0f, +1.0f,
            +1.0f, +1.0f, -1.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f, -1.0f,    +0.0f, +1.0f,
            +1.0f, -1.0f, -1.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f, -1.0f,    +0.0f, +0.0f,
            -1.0f, -1.0f, -1.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f, -1.0f,    +1.0f, +0.0f,

            //left
            -1.0f, +1.0f, -1.0f,    +1.0f, +0.0f, +1.0f,    -1.0f, +0.0f, +0.0f,    +1.0f, +1.0f,
            -1.0f, +1.0f, +1.0f,    +1.0f, +0.0f, +1.0f,    -1.0f, +0.0f, +0.0f,    +0.0f, +1.0f,
            -1.0f, -1.0f, +1.0f,    +1.0f, +0.0f, +1.0f,    -1.0f, +0.0f, +0.0f,    +0.0f, +0.0f,
            -1.0f, -1.0f, -1.0f,    +1.0f, +0.0f, +1.0f,    -1.0f, +0.0f, +0.0f,    +1.0f, +0.0f,

            //top
            +1.0f, +1.0f, -1.0f,    +1.0f, +1.0f, +0.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +1.0f,
            -1.0f, +1.0f, -1.0f,    +1.0f, +1.0f, +0.0f,    +0.0f, +1.0f, +0.0f,    +0.0f, +1.0f,
            -1.0f, +1.0f, +1.0f,    +1.0f, +1.0f, +0.0f,    +0.0f, +1.0f, +0.0f,    +0.0f, +0.0f,
            +1.0f, +1.0f, +1.0f,    +1.0f, +1.0f, +0.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f,

            //bottom
            -1.0f, -1.0f, -1.0f,    +0.0f, +1.0f, +1.0f,    +0.0f, -1.0f, +0.0f,    +1.0f, +1.0f,
            +1.0f, -1.0f, -1.0f,    +0.0f, +1.0f, +1.0f,    +0.0f, -1.0f, +0.0f,    +0.0f, +1.0f,
            +1.0f, -1.0f, +1.0f,    +0.0f, +1.0f, +1.0f,    +0.0f, -1.0f, +0.0f,    +0.0f, +0.0f,
            -1.0f, -1.0f, +1.0f,    +0.0f, +1.0f, +1.0f,    +0.0f, -1.0f, +0.0f,    +1.0f, +0.0f,
        };
        
        //setup vao and vbo
        GLES32.glGenVertexArrays(1, vao, 0);
        GLES32.glBindVertexArray(vao[0]);
            GLES32.glGenBuffers(1, vbo, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo[0]);
                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(cubePCNT.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                FloatBuffer dataBuffer = byteBuffer.asFloatBuffer();
                dataBuffer.put(cubePCNT);
                dataBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubePCNT.length * 4, dataBuffer, GLES32.GL_STATIC_DRAW);
                //position
                GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_VERTEX, 3, GLES32.GL_FLOAT, false, 11 * 4, 0);
                GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_VERTEX);
                //color
                GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 11 * 4, 3 * 4);
                GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);
                //normal
                GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 11 * 4, 6 * 4);
                GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);
                //texcoord
                GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_TEXCOORD, 2, GLES32.GL_FLOAT, false, 11 * 4, 9 * 4);
                GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_TEXCOORD);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        //OpenGL-ES states 
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);
        GLES32.glEnable(GLES32.GL_CULL_FACE);

        //set background clearing color
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        //load texture
        marble_texture[0] = loadGLTexture(R.raw.marble);

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
            }
            else
            {
                GLES32.glUniform1i(doubleTapUniform, 0);
            }

            if(single_tap == 1)
            {
                GLES32.glUniform1i(singleTapUniform, 1);

                GLES32.glActiveTexture(GLES32.GL_TEXTURE_2D);
                GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, marble_texture[0]);
                GLES32.glUniform1i(diffuseTextureUniform, 0);
            }
            else
            {
                GLES32.glUniform1i(singleTapUniform, 0);
            }

            GLES32.glBindVertexArray(vao[0]);
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
        if(marble_texture[0] != 0)
        {
            GLES32.glDeleteTextures(1, marble_texture, 0);
            marble_texture[0] = 0;
        }

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
