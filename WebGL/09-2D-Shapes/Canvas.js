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

var vao_triangle;
var vbo_triangle_position;
var vbo_triangle_color;

var vao_square;
var vbo_square_position;
var vbo_square_color;

var mvpUniform;
var perspectiveProjectionMatrix;

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
    "in vec4 vColor;"                               +
    "uniform mat4 u_mvp_matrix;"                    +
    "out vec4 out_color;"                           +
    "void main(void)"                               +
    "{"                                             +
    "   gl_Position = u_mvp_matrix * vPosition;"    +
    "   out_color = vColor;"                        +
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
    "#version 300 es"                           +
    "\n"                                        +
    "precision highp float;"                    +
    "in vec4 out_color;"                        +
    "out vec4 FragColor;"                       +
    "void main(void)"                           +
    "{"                                         +
    "   FragColor = out_color;"                 +
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
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_COLOR, "vColor");

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

    //setup vao and vbo
    var triangleVertices = new Float32Array([
                                            0.0, 1.0, 0.0,
                                            -1.0, -1.0, 0.0,
                                            1.0, -1.0, 0.0
                                        ]);
    
    var triangleColors = new Float32Array([
                                            1.0, 0.0, 0.0,
                                            0.0, 1.0, 0.0,
                                            0.0, 0.0, 1.0
                                        ]);        

    var squareVertices = new Float32Array([
                                            1.0, 1.0, 0.0,
                                            -1.0, 1.0, 0.0,
                                            -1.0, -1.0, 0.0,
                                            1.0, -1.0, 0.0
                                        ]);
    
    var squareColors = new Float32Array([
                                            0.0, 0.0, 1.0,
                                            0.0, 0.0, 1.0,
                                            0.0, 0.0, 1.0,
                                            0.0, 0.0, 1.0
                                        ]);

    vao_triangle = gl.createVertexArray();
    gl.bindVertexArray(vao_triangle);
        vbo_triangle_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_triangle_position);
            gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        vbo_triangle_color = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_triangle_color);
            gl.bufferData(gl.ARRAY_BUFFER, triangleColors, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
        gl.bindBuffer(gl.ARRAY_BUFFER, null); 
    gl.bindVertexArray(null);

    vao_square = gl.createVertexArray();
    gl.bindVertexArray(vao_square);
        vbo_square_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_position);
            gl.bufferData(gl.ARRAY_BUFFER, squareVertices, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        vbo_square_color = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_color);
            gl.bufferData(gl.ARRAY_BUFFER, squareColors, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
        gl.bindBuffer(gl.ARRAY_BUFFER, null); 
    gl.bindVertexArray(null);

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
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    //code
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);
        mat4.translate(modelViewMatrix, modelViewMatrix, [-1.5, 0.0, -6.0]);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
        gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

        gl.bindVertexArray(vao_triangle);
        gl.drawArrays(gl.TRIANGLES, 0, 3);
        gl.bindVertexArray(null);

        modelViewMatrix = mat4.create();
        modelViewProjectionMatrix = mat4.create();

        mat4.translate(modelViewMatrix, modelViewMatrix, [1.5, 0.0, -6.0]);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
        gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

        gl.bindVertexArray(vao_square);
        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
        gl.bindVertexArray(null);
    gl.useProgram(null);

    //animation loop
    requestAnimationFrame(render, canvas);
}

function uninitialize()
{
    if(vao_triangle)
    {
        gl.deleteVertexArray(vao_triangle);
        vao_triangle = null;
    }

    if(vbo_triangle_position)
    {
        gl.deleteBuffer(vbo_triangle_position);
        vbo_triangle_position = null;
    }

    if(vbo_triangle_color)
    {
        gl.deleteBuffer(vbo_triangle_color);
        vbo_triangle_color = null;
    }

    if(vao_square)
    {
        gl.deleteVertexArray(vao_square);
        vao_square = null;
    }

    if(vbo_square_position)
    {
        gl.deleteBuffer(vbo_square_position);
        vbo_square_position = null;
    }

    if(vbo_square_color)
    {
        gl.deleteBuffer(vbo_square_color);
        vbo_square_color = null;
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
    }
}

function mouseDown()
{
    //code
    
}
