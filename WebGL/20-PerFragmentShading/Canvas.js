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
var LaUniform;
var LdUniform;
var LsUniform;
var lightPositionUniform;
var KaUniform;
var KdUniform;
var KsUniform;
var materialShininessUniform;
var LKeyPressedUniform;

var perspectiveProjectionMatrix;
var lightAmbient = [];
var lightDiffuse = [];
var lightSpecular = [];
var lightPosition = [];
var materialAmbient = [];
var materialDiffuse = [];
var materialSpecular = [];
var materialShininess;
var LKeyPressed;

var sphere = null;

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

        "in vec3 vPosition;"                                +
        "in vec3 vNormal;"                                  +
        
        "uniform mat4 u_modelMatrix;"                       +
        "uniform mat4 u_viewMatrix;"                        +
        "uniform mat4 u_projectionMatrix;"                  +   
        "uniform vec4 u_lightPosition;"                     +
        "uniform mediump int u_LKeyPressed;"                +

        "out vec3 transformed_normal;"                      +
        "out vec3 light_direction;"                         +
        "out vec3 view_vector;"                             +

        "void main(void)"                                                                              +
        "{"                                                                                            +
        "   if(u_LKeyPressed == 1)"                                                                    +
        "   {"                                                                                         +
        
        "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vec4(vPosition, 1.0f);"               +
        "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix)));"          +
        "       transformed_normal = normal_matrix * vNormal;"                                         +
        "       light_direction = vec3(u_lightPosition - eye_coords);"                                 +
        "       view_vector = -eye_coords.xyz;"                                                        +
        
        "   }"                                                                                         +
        
        "   gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * vec4(vPosition, 1.0f);"  +
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
        "uniform mediump int u_LKeyPressed;"                +

        "out vec4 FragColor;"                               +

        "void main(void)"                                                                                                                                       +
        "{"                                                                                                                                                     +
        "   vec3 phong_ads_light;"                                                                                                                              +
        "   if(u_LKeyPressed == 1)"                                                                                                                             +
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

    LaUniform = gl.getUniformLocation(shaderProgramObject, "u_lightAmbient");
    LdUniform = gl.getUniformLocation(shaderProgramObject, "u_lightDiffuse");
    LsUniform = gl.getUniformLocation(shaderProgramObject, "u_lightSpecular");
    lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_lightPosition");

    KaUniform = gl.getUniformLocation(shaderProgramObject, "u_materialAmbient");
    KdUniform = gl.getUniformLocation(shaderProgramObject, "u_materialDiffuse");
    KsUniform = gl.getUniformLocation(shaderProgramObject, "u_materialSpecular");
    materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_materialShininess");

    LKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "u_LKeyPressed");

    //setup sphere data
    sphere = new Mesh();
    makeSphere(sphere, 2.0, 30, 30);

    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjectionMatrix = mat4.create();

    lightAmbient = new Float32Array([0.0, 0.0, 0.0]);
    lightDiffuse = new Float32Array([1.0, 1.0, 1.0]);
    lightSpecular = new Float32Array([1.0, 1.0, 1.0]);
    lightPosition = new Float32Array([100.0, 100.0, 100.0, 1.0])
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

    //code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -5.0]);

        gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
        gl.uniformMatrix4fv(viewMatrixUniform, false, viewMaterix);
        gl.uniformMatrix4fv(perspectiveProjectionMatrixUniform, false, perspectiveProjectionMatrix);

        gl.uniform3fv(LaUniform, lightAmbient);
        gl.uniform3fv(LdUniform, lightDiffuse);
        gl.uniform3fv(LsUniform, lightSpecular);
        gl.uniform4fv(lightPositionUniform, lightPosition);

        gl.uniform3fv(KaUniform, materialAmbient);
        gl.uniform3fv(KdUniform, materialDiffuse);
        gl.uniform3fv(KsUniform, materialSpecular);
        gl.uniform1f(materialShininessUniform, materialShininess);

        gl.uniform1i(LKeyPressedUniform, LKeyPressed);

        sphere.draw();
    gl.useProgram(null);

    //animation loop
    requestAnimationFrame(render, canvas);
}

function uninitialize()
{
    if(sphere)
    {
        sphere.deallocate();
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
