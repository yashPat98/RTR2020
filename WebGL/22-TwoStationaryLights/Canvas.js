//global variables 
var canvas = null;
var gl = null;
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

const WebGLMacros = 
{
    AMC_ATTRIBUTE_VERTEX:0,
    AMC_ATTRIBUTE_COLOR:1,
    AMC_ATTRIBUTE_NORMAL:2,
    AMC_ATTRIBUTE_TEXCOORD:3,
};

var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var modelMatrixUniform;
var viewMatrixUniform;
var perspectiveProjectionMatrixUniform;

var LaUniform1;
var LdUniform1;
var LsUniform1;
var lightPositionUniform1;

var LaUniform2;
var LdUniform2;
var LsUniform2;
var lightPositionUniform2;

var KaUniform;
var KdUniform;
var KsUniform;
var materialShininessUniform;
var LKeyPressedUniform;

var perspectiveProjectionMatrix;
var lightAmbient1 = [];
var lightDiffuse1 = [];
var lightSpecular1 = [];
var lightPosition1 = [];

var lightAmbient2 = [];
var lightDiffuse2 = [];
var lightSpecular2 = [];
var lightPosition2 = [];

var materialAmbient = [];
var materialDiffuse = [];
var materialSpecular = [];
var materialShininess;
var LKeyPressed;

var vao_pyramid;
var vbo_pyramid_position;
var vbo_pyramid_normal;

var pyramid_rotation_angle = 0.0;

var requestAnimationFrame = 
window.requestAnimationFrame        ||
window.webkitRequestAnimationFrame  ||
window.mozRequestAnimationFrame     ||
window.oRequestAnimationFrame       ||
window.msRequestAnimationFrame;

var cancelAnimationFrame = 
window.cancelAnimationFrame ||
window.webkitCancelRequestAnimationFrame    || window.webkitCancelAnimationFrame    ||
window.mozCancelRequestAnimationFrame       || window.mozCancelAnimationFrame       ||
window.oCancelRequestAnimationFrame         || window.oCancelAnimationFrame         ||
window.msCancelRequestAnimationFrame        || window.msCancelAnimationFrame;

//onload function
function main()
{
    //get <canvas> element
    canvas = document.getElementById("AMC");
    if(!canvas)
        console.log("Obtaining Canvas Failed\n");
    else
        console.log("Obtaining Canvas Succeeded\n");
    canvas_original_width = canvas.width;
    canvas_original_height = canvas.height;

    //register keyboard's keydown event handler
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseDown, false);
    window.addEventListener("resize", resize, false);

    //initialize WebGL
    initialize();

    //warm up call
    resize();

    //display
    render();
}

function toggleFullscreen()
{
    //code
    var fullscreen_element = 
    document.fullscreenElement          ||
    document.webkitFullscreenElement    ||
    document.mozFullscreenElement       ||
    document.msFullscreenElement        ||
    null;

    //if not fullscreen
    if(fullscreen_element == null)
    {
        if(canvas.requestFullscreen)   
            canvas.requestFullscreen();
        else if(canvas.mozRequestFullscreen)
            canvas.mozRequestFullscreen();
        else if(canvas.webkitRequestFullscreen)
            canvas.webkitRequestFullscreen();
        else if(canvas.msRequestFullscreen)
            canvas.msRequestFullscreen();
        
        bFullscreen = true;
    }
    else    //if already fullscreen
    {
        if(document.exitFullscreen)
            document.exitFullscreen();
        else if(document.mozCancelFullscreen)
            document.mozCancelFullscreen();
        else if(document.webkitExitFullscreen)
            document.webkitExitFullscreen();
        else if(document.msExitFullscreen)
            document.msExitFullscreen();
        
        bFullscreen = false;
    }
}

function initialize()
{
    //code
    //get WebGL 2.0 context
    gl = canvas.getContext("webgl2");
    if(gl == null)
    {
        console.log("Failed to get the rendering context for WebGL");
        return;
    }
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    //vertex shader
    var vertexShaderSourceCode = 
        "#version 300 es"                                   +
        "\n"                                                +
        
        "precision mediump float;"                          +

        "in vec4 vPosition;"                                +
        "in vec3 vNormal;"                                  +
        
        "uniform mat4 u_modelMatrix;"                       +
        "uniform mat4 u_viewMatrix;"                        +
        "uniform mat4 u_projectionMatrix;"                  +   
        "uniform vec3 u_lightAmbient[2];"                      +
        "uniform vec3 u_lightDiffuse[2];"                      +
        "uniform vec3 u_lightSpecular[2];"                     +
        "uniform vec4 u_lightPosition[2];"                     +
        "uniform vec3 u_materialAmbient;"                   +
        "uniform vec3 u_materialDiffuse;"                   +
        "uniform vec3 u_materialSpecular;"                  +   
        "uniform float u_materialShininess;"                +
        "uniform int u_LKeyPressed;"                        +

        "out vec3 phong_ads_light;"                         +

        "void main(void)"                                                                                                                                   +
        "{"                                                                                                                                                 +
        "   phong_ads_light = vec3(0.0f, 0.0f, 0.0f);"                                                                                                      +
        "   if(u_LKeyPressed == 1)"                                                                                                                         +
        "   {"                                                                                                                                              +
        "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                                                                                +
        "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix)));"                                                               +
        "       vec3 transformed_normal = normalize(normal_matrix * vNormal);"                                                                              +
        "       vec3 view_vector = normalize(-eye_coords.xyz);"                                                                                             +

        "       vec3 light_direction[2];"                                                                                                                   +
        "       vec3 reflection_vector[2];"                                                                                                                 +
        "       vec3 ambient[2];"                                                                                                                           +
        "       vec3 diffuse[2];"                                                                                                                           +
        "       vec3 specular[2];"                                                                                                                          +

        "       for(int i = 0; i < 2; i++)"                                                                                                                 +
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
        "}";

    vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObject, vertexShaderSourceCode);
    gl.compileShader(vertexShaderObject);
    if(gl.getShaderParameter(vertexShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(vertexShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //fragment shader 
    var fragmentShaderSourceCode = 
        "#version 300 es"                                   +
        "\n"                                                +
        "precision highp float;"                            +

        "in vec3 phong_ads_light;"                          +
        "out vec4 FragColor;"                               +

        "void main(void)"                                   +
        "{"                                                 +
        "   FragColor = vec4(phong_ads_light, 1.0f);"       +
        "}";

    fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject, fragmentShaderSourceCode);
    gl.compileShader(fragmentShaderObject);
    if(gl.getShaderParameter(fragmentShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(fragmentShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //shader program
    shaderProgramObject = gl.createProgram();
    gl.attachShader(shaderProgramObject, vertexShaderObject);
    gl.attachShader(shaderProgramObject, fragmentShaderObject);

    //pre-linking binding of shader program object with vertex shader attributes
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_VERTEX, "vPosition");
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

    //linking 
    gl.linkProgram(shaderProgramObject);
    if(!gl.getProgramParameter(shaderProgramObject, gl.LINK_STATUS))
    {
        var error = gl.getProgramInfoLog(shaderProgramObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //get MVP uniform location
    modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_modelMatrix");
    viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_viewMatrix");
    perspectiveProjectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projectionMatrix");

    LaUniform1 = gl.getUniformLocation(shaderProgramObject, "u_lightAmbient[0]");
    LdUniform1 = gl.getUniformLocation(shaderProgramObject, "u_lightDiffuse[0]");
    LsUniform1 = gl.getUniformLocation(shaderProgramObject, "u_lightSpecular[0]");
    lightPositionUniform1 = gl.getUniformLocation(shaderProgramObject, "u_lightPosition[0]");

    LaUniform2 = gl.getUniformLocation(shaderProgramObject, "u_lightAmbient[1]");
    LdUniform2 = gl.getUniformLocation(shaderProgramObject, "u_lightDiffuse[1]");
    LsUniform2 = gl.getUniformLocation(shaderProgramObject, "u_lightSpecular[1]");
    lightPositionUniform2 = gl.getUniformLocation(shaderProgramObject, "u_lightPosition[1]");

    KaUniform = gl.getUniformLocation(shaderProgramObject, "u_materialAmbient");
    KdUniform = gl.getUniformLocation(shaderProgramObject, "u_materialDiffuse");
    KsUniform = gl.getUniformLocation(shaderProgramObject, "u_materialSpecular");
    materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_materialShininess");

    LKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "u_LKeyPressed");

    //setup sphere data
    var pyramidVertices = new Float32Array([
        0.0, 1.0, 0.0,
        -1.0, -1.0, 1.0,
        1.0, -1.0, 1.0,

        0.0, 1.0, 0.0,
        1.0, -1.0, 1.0,
        1.0, -1.0, -1.0,
    
        0.0, 1.0, 0.0,
        1.0, -1.0, -1.0,
        -1.0, -1.0, -1.0,

        0.0, 1.0, 0.0,
        -1.0, -1.0, -1.0,
        -1.0, -1.0, 1.0
    ]);

    var pyramidNormals = new Float32Array([
        0.0, 0.447214, 0.894427,
        0.0, 0.447214, 0.894427,
        0.0, 0.447214, 0.894427,

        0.894427, 0.447214, 0.0,
        0.894427, 0.447214, 0.0,
        0.894427, 0.447214, 0.0,

        0.0, 0.447214, -0.894427,
        0.0, 0.447214, -0.894427,
        0.0, 0.447214, -0.894427,

        -0.894427, 0.447214, 0.0,
        -0.894427, 0.447214, 0.0,
        -0.894427, 0.447214, 0.0
    ])

    //setup vao and vbo
    vao_pyramid = gl.createVertexArray();
    gl.bindVertexArray(vao_pyramid);
        vbo_pyramid_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid_position);
            gl.bufferData(gl.ARRAY_BUFFER, pyramidVertices, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        vbo_pyramid_normal = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid_normal);
            gl.bufferData(gl.ARRAY_BUFFER, pyramidNormals, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_NORMAL);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjectionMatrix = mat4.create();

    lightAmbient1 = new Float32Array([0.0, 0.0, 0.0]);
    lightDiffuse1 = new Float32Array([1.0, 0.0, 0.0]);
    lightSpecular1 = new Float32Array([1.0, 0.0, 0.0]);
    lightPosition1 = new Float32Array([2.0, 0.0, 0.0, 1.0]);

    lightAmbient2 = new Float32Array([0.0, 0.0, 0.0]);
    lightDiffuse2 = new Float32Array([0.0, 0.0, 1.0]);
    lightSpecular2 = new Float32Array([0.0, 0.0, 1.0]);
    lightPosition2 = new Float32Array([-2.0, 0.0, 0.0, 1.0]);

    materialAmbient = new Float32Array([0.0, 0.0, 0.0]);
    materialDiffuse = new Float32Array([1.0, 1.0, 1.0]);
    materialSpecular = new Float32Array([1.0, 1.0, 1.0]);
    materialShininess = 128.0;
    LKeyPressed = 0;
}

function resize()
{
    if(bFullscreen == true)
    {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    else
    {
        canvas.width = canvas_original_width;
        canvas.height = canvas_original_height;
    }

    //set the viewport 
    gl.viewport(0, 0, canvas.width, canvas.height);

    //set projection
    mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width / canvas.height), 0.1, 100.0);
}

function render()
{
    //variable declarations
    var modelMatrix = mat4.create();
    var viewMaterix = mat4.create();
    var rotationMatrix = mat4.create();

    //code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -5.0]);
        mat4.rotateY(modelMatrix, modelMatrix, degreeToRad(pyramid_rotation_angle));

        gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
        gl.uniformMatrix4fv(viewMatrixUniform, false, viewMaterix);
        gl.uniformMatrix4fv(perspectiveProjectionMatrixUniform, false, perspectiveProjectionMatrix);

        gl.uniform3fv(LaUniform1, lightAmbient1);
        gl.uniform3fv(LdUniform1, lightDiffuse1);
        gl.uniform3fv(LsUniform1, lightSpecular1);
        gl.uniform4fv(lightPositionUniform1, lightPosition1);

        gl.uniform3fv(LaUniform2, lightAmbient2);
        gl.uniform3fv(LdUniform2, lightDiffuse2);
        gl.uniform3fv(LsUniform2, lightSpecular2);
        gl.uniform4fv(lightPositionUniform2, lightPosition2);

        gl.uniform3fv(KaUniform, materialAmbient);
        gl.uniform3fv(KdUniform, materialDiffuse);
        gl.uniform3fv(KsUniform, materialSpecular);
        gl.uniform1f(materialShininessUniform, materialShininess);

        gl.uniform1i(LKeyPressedUniform, LKeyPressed);

        gl.bindVertexArray(vao_pyramid);
        gl.drawArrays(gl.TRIANGLES, 0, 12);
    gl.useProgram(null);

    //update
    pyramid_rotation_angle += 1.0;
    if(pyramid_rotation_angle >= 360.0)
        pyramid_rotation_angle = 0.0;

    //animation loop
    requestAnimationFrame(render, canvas);
}

function degreeToRad(angle)
{
    return (angle * 3.141592 / 180.0);
}

function uninitialize()
{
    if(vbo_pyramid_normal)
    {
        gl.deleteBuffer(vbo_pyramid_normal);
        vbo_pyramid_normal = null;
    }
    if(vbo_pyramid_position)
    {
        gl.deleteBuffer(vbo_pyramid_position);
        vbo_pyramid_position = null;
    }

    if(vao_pyramid)
    {
        gl.deleteVertexArray(vao_pyramid);
        vao_pyramid = null;
    }

    if(shaderProgramObject)
    {
        if(fragmentShaderObject)
        {
            gl.detachShader(shaderProgramObject, fragmentShaderObject);
            gl.deleteShader(fragmentShaderObject);
            fragmentShaderObject = null;
        }

        if(vertexShaderObject)
        {
            gl.detachShader(shaderProgramObject, vertexShaderObject);
            gl.deleteShader(vertexShaderObject);
            vertexShaderObject = null;
        }

        gl.deleteProgram(shaderProgramObject);
        shaderProgramObject = null;
    }
}

function keyDown(event)
{
    //code
    switch(event.keyCode)
    {
        //escape
        case 27:
            uninitialize();
            window.close();
            break;
        
        //'F' or 'f'
        case 70:
            toggleFullscreen();
            break;

        case 76:
            if(LKeyPressed == 0)
            {
                LKeyPressed = 1;
            }
            else
            {
                LKeyPressed = 0;
            }
            break;
    }
}

function mouseDown()
{
    //code
    
}
