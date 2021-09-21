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

var mvpUniform;
var materialUniform;
var perspectiveProjectionMatrix;
var sphere = null;

var shoulder = 0;
var elbow = 0;

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
        "#version 300 es"                               +
        "\n"                                            +
        "in vec4 vPosition;"                            +
        "uniform mat4 u_mvp_matrix;"                    +
        "void main(void)"                               +
        "{"                                             +
        "   gl_Position = u_mvp_matrix * vPosition;"    +
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
        "#version 300 es"                               +
        "\n"                                            +
        "precision highp float;"                        +
        "out vec4 FragColor;"                           +
        "uniform vec3 u_materialColor;"                 +
        "void main(void)"                               +
        "{"                                             +
        "   FragColor = vec4(u_materialColor, 1.0);"    +
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
    mvpUniform = gl.getUniformLocation(shaderProgramObject, "u_mvp_matrix");
    materialUniform = gl.getUniformLocation(shaderProgramObject, "u_materialColor");

    //setup sphere data
    sphere = new Mesh();
    makeSphere(sphere, 1.0, 30, 30);

    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);

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
    var modelMatrix1 = mat4.create();
    var modelMatrix2 = mat4.create();
    var viewMatrix = mat4.create();
    var mvpMatrix = mat4.create();

    //code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);
        mat4.translate(modelMatrix1, modelMatrix1, [0.0, 0.0, -10.0]);

        //shoulder
        mat4.rotateZ(modelMatrix1, modelMatrix1, degreeToRad(shoulder));
        mat4.translate(modelMatrix1, modelMatrix1, [2.0, 0.0, 0.0]);

        mat4.multiply(modelMatrix2, modelMatrix2, modelMatrix1);

        mat4.scale(modelMatrix1, modelMatrix1, [2.0, 0.5, 1.0]);

        mat4.multiply(mvpMatrix, perspectiveProjectionMatrix, viewMatrix);
        mat4.multiply(mvpMatrix, mvpMatrix, modelMatrix1);
        gl.uniformMatrix4fv(mvpUniform, false, mvpMatrix);

        gl.uniform3f(materialUniform, 0.5, 0.35, 0.05);
        sphere.draw();

        //elbow
        mat4.translate(modelMatrix2, modelMatrix2, [2.0, 0.0, 0.0]);
        mat4.rotateZ(modelMatrix2, modelMatrix2, degreeToRad(elbow));
        mat4.translate(modelMatrix2, modelMatrix2, [2.0, 0.0, 0.0]);
        mat4.scale(modelMatrix2, modelMatrix2, [2.0, 0.5, 1.0]);

        mat4.identity(mvpMatrix);
        mat4.multiply(mvpMatrix, perspectiveProjectionMatrix, viewMatrix);
        mat4.multiply(mvpMatrix, mvpMatrix, modelMatrix2);
        gl.uniformMatrix4fv(mvpUniform, false, mvpMatrix);

        sphere.draw();
    gl.useProgram(null);

    //animation loop
    requestAnimationFrame(render, canvas);
}

function degreeToRad(angle)
{
    return (angle * 3.141592 / 180.0);
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

        case 83:
            shoulder = (shoulder + 3) % 360;
            break;

        case 115:
            shoulder = (shoulder - 3) % 360;
            break;

        case 69:
            elbow = (elbow + 3) % 360;
            break;

        case 101:
            elbow = (elbow - 3) % 360;
            break;
        
        default:
            break;
    }
}

function mouseDown()
{
    //code
    
}
