package com.RTR.MovingLights;

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
    private int pv_lightAmbientUniform[] = new int[3];
    private int pv_lightDiffuseUniform[] = new int[3];
    private int pv_lightSpecularUniform[] = new int[3];
    private int pv_lightPositionUniform[] = new int[3];
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
    private int pf_lightAmbientUniform[] = new int[3];
    private int pf_lightDiffuseUniform[] = new int[3];
    private int pf_lightSpecularUniform[] = new int[3];
    private int pf_lightPositionUniform[] = new int[4];
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
    private float lightAngle = 0.0f;
    int numVertices;
    int numElements;

    private class Light 
    {
        float ambient[] = new float[3];
        float diffuse[] = new float[3];
        float specular[] = new float[3];
        float position[] = new float[4];
    };

    private Light light[] = new Light[3];

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
            "uniform vec3 u_lightAmbient[3];"                   +
            "uniform vec3 u_lightDiffuse[3];"                   +
            "uniform vec3 u_lightSpecular[3];"                  +
            "uniform vec4 u_lightPosition[3];"                  +
            "uniform vec3 u_materialAmbient;"                   +
            "uniform vec3 u_materialDiffuse;"                   +
            "uniform vec3 u_materialSpecular;"                  +   
            "uniform float u_materialShininess;"                +
            "uniform bool u_doubleTap;"                         +

            "out vec3 phong_ads_light;"                         +

            "void main(void)"                                                                                                                                   +
            "{"                                                                                                                                                 +
            "   phong_ads_light = vec3(0.0f, 0.0f, 0.0f);"                                                                                                      +
            "   if(u_doubleTap == true)"                                                                                                                        +
            "   {"                                                                                                                                              +
            "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                                                                                +
            "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix)));"                                                               +
            "       vec3 transformed_normal = normalize(normal_matrix * vNormal);"                                                                              +
            "       vec3 view_vector = normalize(-eye_coords.xyz);"                                                                                             +

            "       vec3 light_direction[3];"                                                                                                                   +
            "       vec3 reflection_vector[3];"                                                                                                                 +
            "       vec3 ambient[3];"                                                                                                                           +   
            "       vec3 diffuse[3];"                                                                                                                           +
            "       vec3 specular[3];"                                                                                                                          +

            "       for(int i = 0; i < 3; i++)"                                                                                                                 +
            "       {"                                                                                                                                          +
            "           light_direction[i] = normalize(vec3(u_lightPosition[i] - eye_coords));"                                                                 +
            "           reflection_vector[i] = reflect(-light_direction[i], transformed_normal);"                                                               +
            
            "           ambient[i] = u_lightAmbient[i] * u_materialAmbient;"                                                                                    +
            "           diffuse[i] = u_lightDiffuse[i] * u_materialDiffuse * max(dot(light_direction[i], transformed_normal), 0.0f);"                           +
            "           specular[i] = u_lightSpecular[i] * u_materialSpecular * pow(max(dot(reflection_vector[i], view_vector), 0.0f), u_materialShininess);"   +
            
            "           phong_ads_light += ambient[i] + diffuse[i] + specular[i];"                                                                              +
            "       }"                                                                                                                                          +
            "   }"                                                                                                                                              +
            "   else"                                                                                                                                           +
            "   {"                                                                                                                                              +
            "       phong_ads_light = vec3(1.0f, 1.0f, 1.0f);"                                                                                                  +
            "   }"                                                                                                                                              +

            "   gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * vPosition;"                                                                   +
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
        
        pv_lightAmbientUniform[0] = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightAmbient[0]");
        pv_lightDiffuseUniform[0] = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightDiffuse[0]");
        pv_lightSpecularUniform[0] = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightSpecular[0]");
        pv_lightPositionUniform[0] = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightPosition[0]");

        pv_lightAmbientUniform[1] = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightAmbient[1]");
        pv_lightDiffuseUniform[1] = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightDiffuse[1]");
        pv_lightSpecularUniform[1] = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightSpecular[1]");
        pv_lightPositionUniform[1] = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightPosition[1]");

        pv_lightAmbientUniform[2] = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightAmbient[2]");
        pv_lightDiffuseUniform[2] = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightDiffuse[2]");
        pv_lightSpecularUniform[2] = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightSpecular[2]");
        pv_lightPositionUniform[2] = GLES32.glGetUniformLocation(pv_shaderProgramObject, "u_lightPosition[2]");
        
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
            "uniform vec4 u_lightPosition[3];"                  +
            "uniform bool u_doubleTap;"                         +

            "out vec3 transformed_normal;"                      +
            "out vec3 light_direction[3];"                      +
            "out vec3 view_vector;"                             +

            "void main(void)"                                                                               +
            "{"                                                                                             +
            "   if(u_doubleTap == true)"                                                                    +
            "   {"                                                                                          +
            
            "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                            +
            "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix)));"           +
            "       transformed_normal = normal_matrix * vNormal;"                                          +
            "       for(int i = 0; i < 3; i++)"                                                             +
            "       {"                                                                                      +
            "           light_direction[i] = vec3(u_lightPosition[i] - eye_coords);"                        +
            "       }"                                                                                      +
            "       view_vector = -eye_coords.xyz;"                                                         +
            
            "   }"                                                                                          +
            
            "   gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * vPosition;"               +
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
            "in vec3 light_direction[3];"                       +
            "in vec3 view_vector;"                              +

            "uniform vec3 u_lightAmbient[3];"                   +
            "uniform vec3 u_lightDiffuse[3];"                   +
            "uniform vec3 u_lightSpecular[3];"                  +
            "uniform vec3 u_materialAmbient;"                   +
            "uniform vec3 u_materialDiffuse;"                   +
            "uniform vec3 u_materialSpecular;"                  +   
            "uniform float u_materialShininess;"                +
            "uniform bool u_doubleTap;"                         +

            "out vec4 FragColor;"                               +

            "void main(void)"                                                                                                                                       +
            "{"                                                                                                                                                     +
            "   vec3 phong_ads_light;"                                                                                                                              +
            "   if(u_doubleTap == true)"                                                                                                                            +
            "   {"                                                                                                                                                  +
            
            "       vec3 normalized_transformed_normal = normalize(transformed_normal);"                                                                            +
            "       vec3 normalized_view_vector = normalize(view_vector);"                                                                                          +
            "       vec3 normalized_light_direction;"                                                                                                               +
            "       vec3 reflection_vector;"                                                                                                                        +
            "       vec3 ambient[3];"                                                                                                                               +
            "       vec3 diffuse[3];"                                                                                                                               +
            "       vec3 specular[3];"                                                                                                                              +

            "       for(int i = 0; i < 3; i++)"                                                                                                                     +
            "       {"                                                                                                                                              +
            "           normalized_light_direction = normalize(light_direction[i]);"                                                                                +
            "           reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normal);"                                                   +
            "           ambient[i] = u_lightAmbient[i] * u_materialAmbient;"                                                                                        +
            "           diffuse[i] = u_lightDiffuse[i] * u_materialDiffuse * max(dot(normalized_light_direction, normalized_transformed_normal), 0.0f);"            +
            "           specular[i] = u_lightSpecular[i] * u_materialSpecular * pow(max(dot(reflection_vector, normalized_view_vector), 0.0f), u_materialShininess);"    +
            "           phong_ads_light = phong_ads_light + ambient[i] + diffuse[i] + specular[i];"                                                                 +
            "       }"                                                                                                                                              +

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
        
        pf_lightAmbientUniform[0] = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightAmbient[0]");
        pf_lightDiffuseUniform[0] = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightDiffuse[0]");
        pf_lightSpecularUniform[0] = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightSpecular[0]");
        pf_lightPositionUniform[0] = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightPosition[0]");

        pf_lightAmbientUniform[1] = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightAmbient[1]");
        pf_lightDiffuseUniform[1] = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightDiffuse[1]");
        pf_lightSpecularUniform[1] = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightSpecular[1]");
        pf_lightPositionUniform[1] = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightPosition[1]");

        pf_lightAmbientUniform[2] = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightAmbient[2]");
        pf_lightDiffuseUniform[2] = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightDiffuse[2]");
        pf_lightSpecularUniform[2] = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightSpecular[2]");
        pf_lightPositionUniform[2] = GLES32.glGetUniformLocation(pf_shaderProgramObject, "u_lightPosition[2]");
        
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

        //set up light 1 
        light[0] = new Light();

        light[0].ambient[0] = 0.0f;
        light[0].ambient[1] = 0.0f;
        light[0].ambient[2] = 0.0f;

        light[0].diffuse[0] = 1.0f;
        light[0].diffuse[1] = 0.0f;
        light[0].diffuse[2] = 0.0f;

        light[0].specular[0] = 1.0f;
        light[0].specular[1] = 0.0f;
        light[0].specular[2] = 0.0f;

        light[0].position[0] = 0.0f;
        light[0].position[1] = 0.0f;
        light[0].position[2] = 0.0f;
        light[0].position[3] = 1.0f;

        //set up light 2
        light[1] = new Light();

        light[1].ambient[0] = 0.0f;
        light[1].ambient[1] = 0.0f;
        light[1].ambient[2] = 0.0f;

        light[1].diffuse[0] = 0.0f;
        light[1].diffuse[1] = 1.0f;
        light[1].diffuse[2] = 0.0f;

        light[1].specular[0] = 0.0f;
        light[1].specular[1] = 1.0f;
        light[1].specular[2] = 0.0f;

        light[1].position[0] = 0.0f;
        light[1].position[1] = 0.0f;
        light[1].position[2] = 0.0f;
        light[1].position[3] = 1.0f;

        //set up light 3
        light[2] = new Light();

        light[2].ambient[0] = 0.0f;
        light[2].ambient[1] = 0.0f;
        light[2].ambient[2] = 0.0f;

        light[2].diffuse[0] = 0.0f;
        light[2].diffuse[1] = 0.0f;
        light[2].diffuse[2] = 1.0f;

        light[2].specular[0] = 0.0f;
        light[2].specular[1] = 0.0f;
        light[2].specular[2] = 1.0f;

        light[2].position[0] = 0.0f;
        light[2].position[1] = 0.0f;
        light[2].position[2] = 0.0f;
        light[2].position[3] = 1.0f;

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
        float radius = 5.0f;

        //clear buffers
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);        

        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);
        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -3.0f);

        light[0].position[0] = 0.0f;
        light[0].position[1] = radius * (float)Math.sin((float)Math.toRadians(lightAngle));
        light[0].position[2] = radius * (float)Math.cos((float)Math.toRadians(lightAngle));
        light[0].position[3] = 1.0f;

        light[1].position[0] = radius * (float)Math.sin((float)Math.toRadians(lightAngle));
        light[1].position[1] = 0.0f;
        light[1].position[2] = radius * (float)Math.cos((float)Math.toRadians(lightAngle));
        light[1].position[3] = 1.0f;

        light[2].position[0] = radius * (float)Math.sin((float)Math.toRadians(lightAngle));
        light[2].position[1] = radius * (float)Math.cos((float)Math.toRadians(lightAngle));
        light[2].position[2] = 0.0f;
        light[2].position[3] = 1.0f;

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

                for(int i = 0; i < 3; i++)
                {
                    GLES32.glUniform3f(pv_lightAmbientUniform[i], light[i].ambient[0], light[i].ambient[1], light[i].ambient[2]);
                    GLES32.glUniform3f(pv_lightDiffuseUniform[i], light[i].diffuse[0], light[i].diffuse[1], light[i].diffuse[2]);
                    GLES32.glUniform3f(pv_lightSpecularUniform[i], light[i].specular[0], light[i].specular[1], light[i].specular[2]);
                    GLES32.glUniform4f(pv_lightPositionUniform[i], light[i].position[0], light[i].position[1], light[i].position[2], light[i].position[3]);
                }

                GLES32.glUniform3f(pv_materialAmbientUniform, 0.0f, 0.0f, 0.0f);
                GLES32.glUniform3f(pv_materialDiffuseUniform, 1.0f, 1.0f, 1.0f);
                GLES32.glUniform3f(pv_materialSpecularUniform, 1.0f, 1.0f, 1.0f);
                GLES32.glUniform1f(pv_materialShininessUniform, 128.0f);
            }
            else
            {
                GLES32.glUniform1i(pv_doubleTapUniform, 0);
            }
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
                
                for(int i = 0; i < 3; i++)
                {
                    GLES32.glUniform3f(pf_lightAmbientUniform[i], light[i].ambient[0], light[i].ambient[1], light[i].ambient[2]);
                    GLES32.glUniform3f(pf_lightDiffuseUniform[i], light[i].diffuse[0], light[i].diffuse[1], light[i].diffuse[2]);
                    GLES32.glUniform3f(pf_lightSpecularUniform[i], light[i].specular[0], light[i].specular[1], light[i].specular[2]);
                    GLES32.glUniform4f(pf_lightPositionUniform[i], light[i].position[0], light[i].position[1], light[i].position[2], light[i].position[3]);
                }
                
                GLES32.glUniform3f(pf_materialAmbientUniform, 0.0f, 0.0f, 0.0f);
                GLES32.glUniform3f(pf_materialDiffuseUniform, 1.0f, 1.0f, 1.0f);
                GLES32.glUniform3f(pf_materialSpecularUniform, 1.0f, 1.0f, 1.0f);
                GLES32.glUniform1f(pf_materialShininessUniform, 50.0f);
            }
            else
            {
                GLES32.glUniform1i(pf_doubleTapUniform, 0);
            }
        }

        GLES32.glBindVertexArray(vao_sphere[0]);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        GLES32.glBindVertexArray(0);
        GLES32.glUseProgram(0);

        //update 
        lightAngle += 0.5f;
        if(lightAngle >= 360.0f)
        {
            lightAngle = 0.0f;
        }

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
