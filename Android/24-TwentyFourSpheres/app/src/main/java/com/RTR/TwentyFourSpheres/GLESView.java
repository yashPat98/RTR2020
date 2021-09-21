package com.RTR.TwentyFourSpheres;

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

    private int pv_vertexShaderObject;
    private int pv_fragmentShaderObject;
    private int pv_shaderProgramObject;

    private int pv_modelMatrixUniform;
    private int pv_viewMatrixUniform;
    private int pv_projectionMatrixUniform;
    private int pv_lightAmbientUniform;
    private int pv_lightDiffuseUniform;
    private int pv_lightSpecularUniform;
    private int pv_lightPositionUniform;
    private int pv_materialAmbientUniform;
    private int pv_materialDiffuseUniform;
    private int pv_materialSpecularUniform;
    private int pv_materialShininessUniform;
    private int pv_doubleTapUniform;

    private int pf_vertexShaderObject;
    private int pf_fragmentShaderObject;
    private int pf_shaderProgramObject;

    private int pf_modelMatrixUniform;
    private int pf_viewMatrixUniform;
    private int pf_projectionMatrixUniform;
    private int pf_lightAmbientUniform;
    private int pf_lightDiffuseUniform;
    private int pf_lightSpecularUniform;
    private int pf_lightPositionUniform;
    private int pf_materialAmbientUniform;
    private int pf_materialDiffuseUniform;
    private int pf_materialSpecularUniform;
    private int pf_materialShininessUniform;
    private int pf_doubleTapUniform;

    private int[] vao_sphere = new int[1];
    private int[] vbo_sphere_vertices = new int[1];
    private int[] vbo_sphere_normals = new int[1];
    private int[] vbo_sphere_elements = new int[1];

    private float perspectiveProjectionMatrix[] = new float[16];
    private int double_tap = 0;
    private int single_tap = 0;
    private int numVertices;
    private int numElements;

    private float angle_for_x_rotation = 0.0f;
    private float angle_for_y_rotation = 0.0f;
    private float angle_for_z_rotation = 0.0f;
    private int long_press = 0;

    private int iWidth;
    private int iHeight;

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
        iWidth = width;
        iHeight = height;
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
        long_press++;
        if(long_press > 3)
        {
            long_press = 0;
        }

        switch(long_press)
        {
            case 1:
                angle_for_x_rotation = 0;
                break;
            
            case 2:
                angle_for_y_rotation = 0;
                break;
            
            case 3:
                angle_for_z_rotation = 0;
                break;
            
            default:
                break;
        }
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
        //Vertex Shader For Per Vertex Lighting

        //create shader
        pv_vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

        //vertex shader source code
        final String pv_vertexShaderSourceCode = String.format
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
        GLES32.glShaderSource(pv_vertexShaderObject, pv_vertexShaderSourceCode);

        //compile shader
        GLES32.glCompileShader(pv_vertexShaderObject);

        //error checking
        int[] iShaderCompiledStatus = new int[1];
        int[] iInfoLogLength = new int[1];
        String szInfoLog = null;

        GLES32.glGetShaderiv(pv_vertexShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompiledStatus, 0);
        if(iShaderCompiledStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(pv_vertexShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(pv_vertexShaderObject);
                System.out.println("YIP: Per Vertex Shading Vertex Shader Compilation Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //Fragment Shader For Per Vertex Lighting

        //create shader
        pv_fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

        //fragment shader source code
        final String pv_fragmentShaderSourceCode = String.format
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
        GLES32.glShaderSource(pv_fragmentShaderObject, pv_fragmentShaderSourceCode);

        //compile shader
        GLES32.glCompileShader(pv_fragmentShaderObject);

        //error checking
        iShaderCompiledStatus[0] = 0;
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetShaderiv(pv_fragmentShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompiledStatus, 0);
        if(iShaderCompiledStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(pv_fragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(pv_fragmentShaderObject);
                System.out.println("YIP: Per Vertex Shading Fragment Shader Compilation Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //Shader Program For Per Vertex Lighting

        //create shader program
        pv_shaderProgramObject = GLES32.glCreateProgram();

        //attach vertex shader to shader program
        GLES32.glAttachShader(pv_shaderProgramObject, pv_vertexShaderObject);

        //attach fragment shader to shader program 
        GLES32.glAttachShader(pv_shaderProgramObject, pv_fragmentShaderObject);
        
        //pre-link binding of shader program object with vertex shader attributes
        GLES32.glBindAttribLocation(pv_shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_VERTEX, "vPosition");
        GLES32.glBindAttribLocation(pv_shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

        //link shader program 
        GLES32.glLinkProgram(pv_shaderProgramObject);

        //error checking
        int[] iShaderProgramLinkStatus = new int[1];
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetProgramiv(pv_shaderProgramObject, GLES32.GL_LINK_STATUS, iShaderProgramLinkStatus, 0);
        if(iShaderProgramLinkStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(pv_shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetProgramInfoLog(pv_shaderProgramObject);
                System.out.println("YIP: Per Vertex Shading Shader Program Link Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //get uniform locations
        pv_modelMatrixUniform = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_modelMatrix");
        pv_viewMatrixUniform = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_viewMatrix");
        pv_projectionMatrixUniform = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_projectionMatrix");
        pv_lightAmbientUniform = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightAmbient");
        pv_lightDiffuseUniform = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightDiffuse");
        pv_lightSpecularUniform = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightSpecular");
        pv_lightPositionUniform = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightPosition");
        pv_materialAmbientUniform = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_materialAmbient");
        pv_materialDiffuseUniform = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_materialDiffuse");
        pv_materialSpecularUniform = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_materialSpecular");
        pv_materialShininessUniform = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_materialShininess");
        pv_doubleTapUniform = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_doubleTap");

        //Vertex Shader For Per Fragment Shading

        //create shader
        pf_vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

        //vertex shader source code
        final String pf_vertexShaderSourceCode = String.format
        (
            "#version 320 es"                                   +
            "\n"                                                +
            
            "precision mediump float;"                          +

            "in vec4 vPosition;"                                +
            "in vec3 vNormal;"                                  +
            
            "uniform mat4 u_modelMatrix;"                       +
            "uniform mat4 u_viewMatrix;"                        +
            "uniform mat4 u_projectionMatrix;"                  +   
            "uniform vec4 u_lightPosition;"                     +
            "uniform int u_doubleTap;"                          +

            "out vec3 transformed_normal;"                      +
            "out vec3 light_direction;"                         +
            "out vec3 view_vector;"                             +

            "void main(void)"                                                                              +
            "{"                                                                                            +
            "   if(u_doubleTap == 1)"                                                                      +
            "   {"                                                                                         +
            
            "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                           +
            "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix)));"          +
            "       transformed_normal = normal_matrix * vNormal;"                              +
            "       light_direction = vec3(u_lightPosition - eye_coords);"                      +
            "       view_vector = -eye_coords.xyz;"                                             +
            
            "   }"                                                                                         +
            
            "   gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * vPosition;"              +
            "}"
        );

        //provide source code to shader
        GLES32.glShaderSource(pf_vertexShaderObject, pf_vertexShaderSourceCode);

        //compile shader
        GLES32.glCompileShader(pf_vertexShaderObject);

        //error checking
        iShaderCompiledStatus[0] = 0;
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetShaderiv(pf_vertexShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompiledStatus, 0);
        if(iShaderCompiledStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(pf_vertexShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(pf_vertexShaderObject);
                System.out.println("YIP: Per Fragment Shading Vertex Shader Compilation Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //Fragment Shader For Per Fragment Shading

        //create shader
        pf_fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

        //fragment shader source code
        final String pf_fragmentShaderSourceCode = String.format
        (
            "#version 320 es"                                   +
            "\n"                                                +
            "precision highp float;"                            +

            "in vec3 transformed_normal;"                       +
            "in vec3 light_direction;"                          +
            "in vec3 view_vector;"                              +

            "uniform vec3 u_lightAmbient;"                      +
            "uniform vec3 u_lightDiffuse;"                      +
            "uniform vec3 u_lightSpecular;"                     +
            "uniform vec3 u_materialAmbient;"                   +
            "uniform vec3 u_materialDiffuse;"                   +
            "uniform vec3 u_materialSpecular;"                  +   
            "uniform float u_materialShininess;"                +
            "uniform int u_doubleTap;"                          +

            "out vec4 FragColor;"                               +

            "void main(void)"                                                                                                                                       +
            "{"                                                                                                                                                     +
            "   vec3 phong_ads_light;"                                                                                                                              +
            "   if(u_doubleTap == 1)"                                                                                                                               +
            "   {"                                                                                                                                                  +
            
            "       vec3 normalized_transformed_normal = normalize(transformed_normal);"                                                                            +
            "       vec3 normalized_light_direction = normalize(light_direction);"                                                                                  +
            "       vec3 normalized_view_vector = normalize(view_vector);"                                                                                          +
            "       vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normal);"                                                  +
            
            "       vec3 ambient = u_lightAmbient * u_materialAmbient;"                                                                                             +
            "       vec3 diffuse = u_lightDiffuse * u_materialDiffuse * max(dot(normalized_light_direction, normalized_transformed_normal), 0.0f);"                 +
            "       vec3 specular = u_lightSpecular * u_materialSpecular * pow(max(dot(reflection_vector, normalized_view_vector), 0.0f), u_materialShininess);"    +
            "       phong_ads_light = ambient + diffuse + specular;"                                                                                                +

            "   }"                                                                                                                                                  +
            "   else"                                                                                                                                               +
            "   {"                                                                                                                                                  +
            
            "       phong_ads_light = vec3(1.0f, 1.0f, 1.0f);"                                                                                                      +
            
            "   }"                                                                                                                                                  +
            
            "   FragColor = vec4(phong_ads_light, 1.0f);"                                                                                                           +
            "}"
        );

        //provide source code to shader
        GLES32.glShaderSource(pf_fragmentShaderObject, pf_fragmentShaderSourceCode);

        //compile shader
        GLES32.glCompileShader(pf_fragmentShaderObject);

        //error checking
        iShaderCompiledStatus[0] = 0;
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetShaderiv(pf_fragmentShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompiledStatus, 0);
        if(iShaderCompiledStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(pf_fragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(pf_fragmentShaderObject);
                System.out.println("YIP: Per Fragment Shading Fragment Shader Compilation Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //Shader Program For Per Fragment Shading

        //create shader program
        pf_shaderProgramObject = GLES32.glCreateProgram();

        //attach vertex shader to shader program
        GLES32.glAttachShader(pf_shaderProgramObject, pf_vertexShaderObject);

        //attach fragment shader to shader program 
        GLES32.glAttachShader(pf_shaderProgramObject, pf_fragmentShaderObject);
        
        //pre-link binding of shader program object with vertex shader attributes
        GLES32.glBindAttribLocation(pf_shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_VERTEX, "vPosition");
        GLES32.glBindAttribLocation(pf_shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

        //link shader program 
        GLES32.glLinkProgram(pf_shaderProgramObject);

        //error checking
        iShaderProgramLinkStatus[0] = 0;
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetProgramiv(pf_shaderProgramObject, GLES32.GL_LINK_STATUS, iShaderProgramLinkStatus, 0);
        if(iShaderProgramLinkStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(pf_shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetProgramInfoLog(pf_shaderProgramObject);
                System.out.println("YIP: Per Fragment Shading Shader Program Link Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //get uniform locations
        pf_modelMatrixUniform = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_modelMatrix");
        pf_viewMatrixUniform = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_viewMatrix");
        pf_projectionMatrixUniform = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_projectionMatrix");
        pf_lightAmbientUniform = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightAmbient");
        pf_lightDiffuseUniform = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightDiffuse");
        pf_lightSpecularUniform = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightSpecular");
        pf_lightPositionUniform = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightPosition");
        pf_materialAmbientUniform = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_materialAmbient");
        pf_materialDiffuseUniform = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_materialDiffuse");
        pf_materialSpecularUniform = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_materialSpecular");
        pf_materialShininessUniform = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_materialShininess");
        pf_doubleTapUniform = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_doubleTap");

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
        float lightPosition[] = new float[4];

        //clear buffers
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);
        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -3.0f);

        switch(long_press)
        {
            case 1:
                lightPosition[0] = 20.0f * (float)Math.sin(angle_for_x_rotation);
                lightPosition[1] = 20.0f * (float)Math.cos(angle_for_x_rotation);
                lightPosition[2] = 0.0f;
                lightPosition[3] = 1.0f;
                break;
            
            case 2:
                lightPosition[0] = 20.0f * (float)Math.sin(angle_for_y_rotation);
                lightPosition[1] = 0.0f;
                lightPosition[2] = 20.0f * (float)Math.cos(angle_for_y_rotation);
                lightPosition[3] = 1.0f;
                break;
            
            case 3:
                lightPosition[0] = 0.0f;
                lightPosition[1] = 20.0f * (float)Math.sin(angle_for_z_rotation);
                lightPosition[2] = 20.0f * (float)Math.cos(angle_for_z_rotation);
                lightPosition[3] = 1.0f;
                break;
            
            default:
                break;
        }

        if(single_tap == 0)
        {
            GLES32.glUseProgram(pv_shaderProgramObject);

            //pass the uniform variables to shaders
            GLES32.glUniformMatrix4fv(pv_modelMatrixUniform, 1, false, modelMatrix, 0);
            GLES32.glUniformMatrix4fv(pv_viewMatrixUniform, 1, false, viewMatrix, 0);
            GLES32.glUniformMatrix4fv(pv_projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

            if(double_tap == 1)
            {
                GLES32.glUniform1i(pv_doubleTapUniform, 1);
                GLES32.glUniform3f(pv_lightAmbientUniform, 0.0f, 0.0f, 0.0f);
                GLES32.glUniform3f(pv_lightDiffuseUniform, 1.0f, 1.0f, 1.0f);
                GLES32.glUniform3f(pv_lightSpecularUniform, 1.0f, 1.0f, 1.0f);
                GLES32.glUniform4f(pv_lightPositionUniform, lightPosition[0], lightPosition[1], lightPosition[2], lightPosition[3]);
            }
            else
            {
                GLES32.glUniform1i(pv_doubleTapUniform, 0);
            }

            drawSpheresPerVertex();
        }
        else
        {
            GLES32.glUseProgram(pf_shaderProgramObject);

            //pass the uniform variables to shaders
            GLES32.glUniformMatrix4fv(pf_modelMatrixUniform, 1, false, modelMatrix, 0);
            GLES32.glUniformMatrix4fv(pf_viewMatrixUniform, 1, false, viewMatrix, 0);
            GLES32.glUniformMatrix4fv(pf_projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

            if(double_tap == 1)
            {
                GLES32.glUniform1i(pf_doubleTapUniform, 1);
                GLES32.glUniform3f(pf_lightAmbientUniform, 0.0f, 0.0f, 0.0f);
                GLES32.glUniform3f(pf_lightDiffuseUniform, 1.0f, 1.0f, 1.0f);
                GLES32.glUniform3f(pf_lightSpecularUniform, 1.0f, 1.0f, 1.0f);
                GLES32.glUniform4f(pf_lightPositionUniform, lightPosition[0], lightPosition[1], lightPosition[2], lightPosition[3]);
            }
            else
            {
                GLES32.glUniform1i(pf_doubleTapUniform, 0);
            }

            drawSpheresPerFragment();
        }

        GLES32.glUseProgram(0);

        //update
        switch(long_press)
        {
            case 1:
                angle_for_x_rotation += 0.1f;
                if(angle_for_x_rotation >= 360.0f)
                    angle_for_x_rotation = 0.0f;
                break;
            
            case 2:
                angle_for_y_rotation += 0.1f;
                if(angle_for_y_rotation >= 360.0f)
                    angle_for_y_rotation = 0.0f;
                break;
            
            case 3:
                angle_for_z_rotation += 0.1f;
                if(angle_for_z_rotation >= 360.0f)
                    angle_for_z_rotation = 0.0f;
                break;
            
            default:
                break;
        }

        requestRender();
    }

    private void drawSpheresPerVertex()
    {
        float modelMatrix[] = new float[16];

        //code
        int width = iWidth;
        int height = iHeight;
        resize(width / 4, height / 6);

        GLES32.glBindVertexArray(vao_sphere[0]);

        //emrald
        GLES32.glViewport(0, height * 5 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.0215f, 0.1745f, 0.0215f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.07568f, 0.61424f, 0.07568f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.633f, 0.727811f, 0.633f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.6f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
        
        //jade 
        GLES32.glViewport(width / 4, height * 5 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.135f, 0.2225f, 0.1575f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.54f, 0.89f, 0.63f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.316228f, 0.316228f, 0.316228f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.1f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //obsidian
        GLES32.glViewport(width * 2 / 4, height * 5 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.05375f, 0.05f, 0.06625f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.18275f, 0.17f, 0.22525f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.332741f, 0.328634f, 0.346435f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.3f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //pearl
        GLES32.glViewport(width * 3 / 4, height * 5 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.25f, 0.20725f, 0.20725f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 1.0f, 0.829f, 0.829f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.296648f, 0.296648f, 0.296648f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.088f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //ruby
        GLES32.glViewport(0, height * 4 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.1745f, 0.01175f, 0.01175f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.61424f, 0.04136f, 0.04136f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.727811f, 0.626959f, 0.626959f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.6f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //turquiose
        GLES32.glViewport(width / 4, height * 4 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.1f, 0.18725f, 0.1745f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.396f, 0.74151f, 0.69102f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.297254f, 0.30829f, 0.306678f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.1f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //brass
        GLES32.glViewport(width * 2 / 4, height * 4 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.329412f, 0.223529f, 0.027451f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.780392f, 0.568627f, 0.113725f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.992157f, 0.941176f, 0.807843f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.21794872f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //bronze
        GLES32.glViewport(width * 3 / 4, height * 4 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.2125f, 0.1275f, 0.054f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.714f, 0.4284f, 0.18144f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.393548f, 0.271906f, 0.166721f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.2f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //chrome
        GLES32.glViewport(0, height * 3 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.25f, 0.25f, 0.25f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.4f, 0.4f, 0.4f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.774597f, 0.774597f, 0.774597f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.6f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //copper
        GLES32.glViewport(width / 4, height * 3 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.19125f, 0.0735f, 0.0225f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.7038f, 0.27048f, 0.0828f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.256777f, 0.137622f, 0.086014f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.1f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //gold
        GLES32.glViewport(width * 2 / 4, height * 3 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.24725f, 0.1995f, 0.0745f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.75164f, 0.60648f, 0.22648f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.628281f, 0.555802f, 0.366065f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.4f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //silver
        GLES32.glViewport(width * 3 / 4, height * 3 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.19225f, 0.19225f, 0.19225f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.50754f, 0.50754f, 0.50754f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.508273f, 0.508273f, 0.508273f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.4f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //black
        GLES32.glViewport(0, height * 2 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.0f, 0.0f, 0.0f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.01f, 0.01f, 0.01f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.5f, 0.5f, 0.5f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.25f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //cyan
        GLES32.glViewport(width / 4, height * 2 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.0f, 0.1f, 0.06f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.0f, 0.50980392f, 0.50980392f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.50196078f, 0.50196078f, 0.50196078f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.25f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //green
        GLES32.glViewport(width * 2 / 4, height * 2 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.0f, 0.0f, 0.0f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.1f, 0.35f, 0.1f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.45f, 0.55f, 0.45f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.25f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //red
        GLES32.glViewport(width * 3 / 4, height * 2 / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.0f, 0.0f, 0.0f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.5f, 0.0f, 0.0f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.7f, 0.6f, 0.6f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.25f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //white
        GLES32.glViewport(0, height / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.0f, 0.0f, 0.0f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.55f, 0.55f, 0.55f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.7f, 0.7f, 0.7f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.25f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //yellow plastic
        GLES32.glViewport(width / 4, height / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.0f, 0.0f, 0.0f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.5f, 0.5f, 0.0f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.6f, 0.6f, 0.5f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.25f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //black
        GLES32.glViewport(width * 2 / 4, height / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.02f, 0.02f, 0.02f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.01f, 0.01f, 0.01f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.4f, 0.4f, 0.4f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.078125f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //cyan 
        GLES32.glViewport(width * 3 / 4, height / 6, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.0f, 0.05f, 0.05f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.4f, 0.5f, 0.5f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.04f, 0.7f, 0.7f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.078125f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //green
        GLES32.glViewport(0, 0, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.0f, 0.05f, 0.0f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.4f, 0.5f, 0.4f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.04f, 0.7f, 0.04f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.078125f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //red
        GLES32.glViewport(width / 4, 0, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.05f, 0.0f, 0.0f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.5f, 0.4f, 0.4f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.7f, 0.04f, 0.04f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.078125f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //white
        GLES32.glViewport(width * 2 / 4, 0, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.05f, 0.05f, 0.05f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.5f, 0.5f, 0.5f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.7f, 0.7f, 0.7f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.078125f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //yellow rubber 
        GLES32.glViewport(width * 3 / 4, 0, width / 4, height / 6);

        GLES32.glUniform3f(pv_materialAmbientUniform, 0.05f, 0.05f, 0.0f);
        GLES32.glUniform3f(pv_materialDiffuseUniform, 0.5f, 0.5f, 0.5f);
        GLES32.glUniform3f(pv_materialSpecularUniform, 0.7f, 0.7f, 0.04f);
        GLES32.glUniform1f(pv_materialShininessUniform, 0.078125f * 128.0f);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        GLES32.glBindVertexArray(0);
    }

    private void drawSpheresPerFragment()
    {
        float modelMatrix[] = new float[16];
    
        //code
        int width = iWidth;
        int height = iHeight;
        resize(width / 4, height / 6);
    
        GLES32.glBindVertexArray(vao_sphere[0]);
    
        //emrald
        GLES32.glViewport(0, height * 5 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.0215f, 0.1745f, 0.0215f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.07568f, 0.61424f, 0.07568f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.633f, 0.727811f, 0.633f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.6f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
        
        //jade 
        GLES32.glViewport(width / 4, height * 5 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.135f, 0.2225f, 0.1575f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.54f, 0.89f, 0.63f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.316228f, 0.316228f, 0.316228f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.1f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //obsidian
        GLES32.glViewport(width * 2 / 4, height * 5 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.05375f, 0.05f, 0.06625f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.18275f, 0.17f, 0.22525f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.332741f, 0.328634f, 0.346435f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.3f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //pearl
        GLES32.glViewport(width * 3 / 4, height * 5 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.25f, 0.20725f, 0.20725f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 1.0f, 0.829f, 0.829f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.296648f, 0.296648f, 0.296648f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.088f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //ruby
        GLES32.glViewport(0, height * 4 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.1745f, 0.01175f, 0.01175f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.61424f, 0.04136f, 0.04136f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.727811f, 0.626959f, 0.626959f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.6f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //turquiose
        GLES32.glViewport(width / 4, height * 4 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.1f, 0.18725f, 0.1745f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.396f, 0.74151f, 0.69102f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.297254f, 0.30829f, 0.306678f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.1f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //brass
        GLES32.glViewport(width * 2 / 4, height * 4 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.329412f, 0.223529f, 0.027451f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.780392f, 0.568627f, 0.113725f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.992157f, 0.941176f, 0.807843f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.21794872f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //bronze
        GLES32.glViewport(width * 3 / 4, height * 4 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.2125f, 0.1275f, 0.054f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.714f, 0.4284f, 0.18144f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.393548f, 0.271906f, 0.166721f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.2f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //chrome
        GLES32.glViewport(0, height * 3 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.25f, 0.25f, 0.25f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.4f, 0.4f, 0.4f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.774597f, 0.774597f, 0.774597f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.6f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //copper
        GLES32.glViewport(width / 4, height * 3 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.19125f, 0.0735f, 0.0225f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.7038f, 0.27048f, 0.0828f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.256777f, 0.137622f, 0.086014f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.1f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //gold
        GLES32.glViewport(width * 2 / 4, height * 3 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.24725f, 0.1995f, 0.0745f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.75164f, 0.60648f, 0.22648f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.628281f, 0.555802f, 0.366065f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.4f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //silver
        GLES32.glViewport(width * 3 / 4, height * 3 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.19225f, 0.19225f, 0.19225f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.50754f, 0.50754f, 0.50754f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.508273f, 0.508273f, 0.508273f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.4f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //black
        GLES32.glViewport(0, height * 2 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.0f, 0.0f, 0.0f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.01f, 0.01f, 0.01f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.5f, 0.5f, 0.5f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.25f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //cyan
        GLES32.glViewport(width / 4, height * 2 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.0f, 0.1f, 0.06f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.0f, 0.50980392f, 0.50980392f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.50196078f, 0.50196078f, 0.50196078f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.25f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //green
        GLES32.glViewport(width * 2 / 4, height * 2 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.0f, 0.0f, 0.0f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.1f, 0.35f, 0.1f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.45f, 0.55f, 0.45f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.25f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //red
        GLES32.glViewport(width * 3 / 4, height * 2 / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.0f, 0.0f, 0.0f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.5f, 0.0f, 0.0f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.7f, 0.6f, 0.6f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.25f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //white
        GLES32.glViewport(0, height / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.0f, 0.0f, 0.0f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.55f, 0.55f, 0.55f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.7f, 0.7f, 0.7f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.25f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //yellow plastic
        GLES32.glViewport(width / 4, height / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.0f, 0.0f, 0.0f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.5f, 0.5f, 0.0f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.6f, 0.6f, 0.5f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.25f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //black
        GLES32.glViewport(width * 2 / 4, height / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.02f, 0.02f, 0.02f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.01f, 0.01f, 0.01f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.4f, 0.4f, 0.4f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.078125f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //cyan 
        GLES32.glViewport(width * 3 / 4, height / 6, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.0f, 0.05f, 0.05f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.4f, 0.5f, 0.5f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.04f, 0.7f, 0.7f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.078125f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //green
        GLES32.glViewport(0, 0, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.0f, 0.05f, 0.0f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.4f, 0.5f, 0.4f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.04f, 0.7f, 0.04f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.078125f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //red
        GLES32.glViewport(width / 4, 0, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.05f, 0.0f, 0.0f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.5f, 0.4f, 0.4f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.7f, 0.04f, 0.04f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.078125f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //white
        GLES32.glViewport(width * 2 / 4, 0, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.05f, 0.05f, 0.05f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.5f, 0.5f, 0.5f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.7f, 0.7f, 0.7f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.078125f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        //yellow rubber 
        GLES32.glViewport(width * 3 / 4, 0, width / 4, height / 6);
    
        GLES32.glUniform3f(pf_materialAmbientUniform, 0.05f, 0.05f, 0.0f);
        GLES32.glUniform3f(pf_materialDiffuseUniform, 0.5f, 0.5f, 0.5f);
        GLES32.glUniform3f(pf_materialSpecularUniform, 0.7f, 0.7f, 0.04f);
        GLES32.glUniform1f(pf_materialShininessUniform, 0.078125f * 128.0f);
    
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
    
        GLES32.glBindVertexArray(0);
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

        if(pv_shaderProgramObject != 0)
        {
            if(pv_vertexShaderObject != 0)
            {
                GLES32.glDetachShader(pv_shaderProgramObject, pv_vertexShaderObject);
                GLES32.glDeleteShader(pv_vertexShaderObject);
                pv_vertexShaderObject = 0;
            }
            
            if(pv_fragmentShaderObject != 0)
            {
                GLES32.glDetachShader(pv_shaderProgramObject, pv_fragmentShaderObject);
                GLES32.glDeleteShader(pv_fragmentShaderObject);
                pv_fragmentShaderObject = 0;
            }
        
            GLES32.glDeleteProgram(pv_shaderProgramObject);
            pv_shaderProgramObject = 0;
        }

        if(pf_shaderProgramObject != 0)
        {
            if(pf_vertexShaderObject != 0)
            {
                GLES32.glDetachShader(pf_shaderProgramObject, pf_vertexShaderObject);
                GLES32.glDeleteShader(pf_vertexShaderObject);
                pf_vertexShaderObject = 0;
            }
            
            if(pf_fragmentShaderObject != 0)
            {
                GLES32.glDetachShader(pf_shaderProgramObject, pf_fragmentShaderObject);
                GLES32.glDeleteShader(pf_fragmentShaderObject);
                pf_fragmentShaderObject = 0;
            }
        
            GLES32.glDeleteProgram(pf_shaderProgramObject);
            pf_shaderProgramObject = 0;
        }
    }
}
