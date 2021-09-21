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
    AMC_ATTRIBUTE_TEXTURE:3,
};

var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var vao_cube;
var vbo_cube_position;
var vbo_cube_normal;

var modelViewMatrixUniform;
var projectionMatrixUniform;
var vsLKeyPressedUniform;
var fsLKeyPressedUniform;
var lightDiffuseUniform;
var matrialDiffuseUniform; 
var lightPositionUniform;

var perspectiveProjectionMatrix;
var LKeyPressed = 0;
var cube_angle = 0.0;

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
        "#version 300 es"                                                                       +
        "\n"                                                                                    +
        "precision mediump float;"                                                              +

        "in vec4 vPosition;"                                                                    +
        "in vec3 vNormal;"                                                                      +

        "out vec3 diffuse_light;"                                                               +

        "uniform mat4 modelview_matrix;"                                                        +
        "uniform mat4 projection_matrix;"                                                       +
        "uniform int vs_LKeyPressed;"                                                           +
        "uniform vec3 light_diffuse;"                                                           +
        "uniform vec3 material_diffuse;"                                                        +
        "uniform vec4 light_position;"                                                          +

        "void main(void)"                                                                       +
        "{"                                                                                     +
        "   if(vs_LKeyPressed == 1)"                                                            +
        "   {"                                                                                  +
        "       vec4 eye_coords = modelview_matrix * vPosition;"                                +
        "       mat3 normal_matrix = mat3(transpose(inverse(modelview_matrix)));"               +
        "       vec3 tnorm = normalize(normal_matrix * vNormal);"                               +
        "       vec3 s = normalize(vec3(light_position - eye_coords));"                         +
        "       diffuse_light = light_diffuse * material_diffuse * max(dot(s, tnorm), 0.0);"    +
        "   }"                                                                                  +
        
        "   gl_Position = projection_matrix * modelview_matrix * vPosition;"                    +
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

        "in vec3 diffuse_light;"                            +
        "out vec4 FragColor;"                               +

        "uniform int fs_LKeyPressed;"                       +

        "void main(void)"                                   +
        "{"                                                 +
        "   if(fs_LKeyPressed == 1)"                        +
        "       FragColor = vec4(diffuse_light, 1.0f);"     +
        "   else"                                           +
        "       FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);"  +
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
    modelViewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "modelview_matrix");
    projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "projection_matrix");
    vsLKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "vs_LKeyPressed");
    fsLKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "fs_LKeyPressed");
    lightDiffuseUniform = gl.getUniformLocation(shaderProgramObject, "light_diffuse");
    matrialDiffuseUniform = gl.getUniformLocation(shaderProgramObject, "material_diffuse");
    lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "light_position");

    //cube vertices
    var cubeVertices = new Float32Array([
        1.0, 1.0, 1.0,
        -1.0, 1.0, 1.0,
        -1.0, -1.0, 1.0, 
        1.0, -1.0, 1.0,

        1.0, 1.0, -1.0,
        1.0, 1.0, 1.0,
        1.0, -1.0, 1.0,
        1.0, -1.0, -1.0,

        -1.0, 1.0, -1.0,
        1.0, 1.0, -1.0,
        1.0, -1.0, -1.0,
        -1.0, -1.0, -1.0,

        -1.0, 1.0, -1.0,
        -1.0, 1.0, 1.0, 
        -1.0, -1.0, 1.0,
        -1.0, -1.0, -1.0,

        1.0, 1.0, -1.0,
        -1.0, 1.0, -1.0,
        -1.0, 1.0, 1.0,
        1.0, 1.0, 1.0,

        -1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,
        1.0, -1.0, 1.0,
        -1.0, -1.0, 1.0
    ]);

    var cubeNormals = new Float32Array([
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,

        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,

        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,

        -1.0, 0.0, 0.0,
        -1.0, 0.0, 0.0,
        -1.0, 0.0, 0.0,
        -1.0, 0.0, 0.0,

        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,

        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0
    ]);

    //setup vao and vbo
    vao_cube = gl.createVertexArray();
    gl.bindVertexArray(vao_cube);
        vbo_cube_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_cube_position);
            gl.bufferData(gl.ARRAY_BUFFER, cubeVertices, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        vbo_cube_normal = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_cube_normal);
            gl.bufferData(gl.ARRAY_BUFFER, cubeNormals, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_NORMAL);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    gl.enable(gl.DEPTH_TEST);
    gl.clearDepth(1.0);
    gl.depthFunc(gl.LEQUAL);

    perspectiveProjectionMatrix = mat4.create();
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
    var modelViewMatrix = mat4.create();
    var rotationMatrix = mat4.create();

    //code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);
        mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -4.0]);
        mat4.rotateY(modelViewMatrix, modelViewMatrix, degreeToRad(cube_angle));

        gl.uniformMatrix4fv(modelViewMatrixUniform, false, modelViewMatrix);
        gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

        gl.uniform1i(vsLKeyPressedUniform, LKeyPressed);
        gl.uniform1i(fsLKeyPressedUniform, LKeyPressed);

        gl.uniform3f(lightDiffuseUniform, 1.0, 1.0, 1.0);
        gl.uniform3f(matrialDiffuseUniform, 1.0, 1.0, 1.0);
        gl.uniform4f(lightPositionUniform, 0.0, 0.0, 5.0, 1.0);

        gl.bindVertexArray(vao_cube);
        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);
        gl.bindVertexArray(null);
    gl.useProgram(null);

    //update 
    cube_angle += 0.1;
    if(cube_angle >= 360.0)
        cube_angle = 0.0;

    //animation loop
    requestAnimationFrame(render, canvas);
}

function degreeToRad(angle)
{
    return (angle * Math.PI / 180.0);
}

function uninitialize()
{
    if(vao_cube)
    {
        gl.deleteVertexArray(vao_cube);
        vao_cube = null;
    }

    if(vbo_cube_position)
    {
        gl.deleteBuffer(vbo_cube_position);
        vbo_cube_position = null;
    }

    if(vbo_cube_normal)
    {
        gl.deleteBuffer(vbo_cube_normal);
        vbo_cube_normal = null;
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
