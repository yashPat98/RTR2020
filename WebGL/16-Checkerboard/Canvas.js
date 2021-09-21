//global variables 
var canvas = null;
var gl = null;
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

var image_width = 64;
var image_height = 64;

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

var vao_square;
var vbo_square_position;
var vbo_square_texcoord;

var mvpUniform;
var textureSamplerUniform;
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

var checker_texture;

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
    "in vec2 vTexCoord;"                            +
    "uniform mat4 u_mvp_matrix;"                    +
    "out vec2 out_texcoord;"                        +
    "void main(void)"                               +
    "{"                                             +
    "   gl_Position = u_mvp_matrix * vPosition;"    +
    "   out_texcoord = vTexCoord;"                  +
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
    "#version 300 es"                                           +
    "\n"                                                        +
    "precision highp float;"                                    +
    "in vec2 out_texcoord;"                                     +
    "uniform sampler2D u_textureSampler;"                       +
    "out vec4 FragColor;"                                       +
    "void main(void)"                                           +
    "{"                                                         +
    "   FragColor = texture(u_textureSampler, out_texcoord);"   +
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
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_TEXCOORD, "vTexCoord");

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
    textureSamplerUniform = gl.getUniformLocation(shaderProgramObject, "u_textureSampler");

    //setup vao and vbo
    var squareTexCoords = new Float32Array([
        1.0, 1.0, 
        0.0, 1.0,
        0.0, 0.0, 
        1.0, 0.0,
    ]);

    vao_square = gl.createVertexArray();
    gl.bindVertexArray(vao_square);
        vbo_square_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_position);
            gl.bufferData(gl.ARRAY_BUFFER, null, gl.DYNAMIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        vbo_square_texcoord = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_texcoord);
            gl.bufferData(gl.ARRAY_BUFFER, squareTexCoords, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD, 2, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD);
        gl.bindBuffer(gl.ARRAY_BUFFER, null); 
    gl.bindVertexArray(null);

    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    //create checker texture
    var checkImage = new Uint8Array(image_width * image_height * 4);
    var index = 0;
    for(var i = 0; i < 64; i++)
    {
        for(var j = 0; j < 64; j++)
        {
            var c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0)) * 255;
            
            checkImage[index + 0] = c;
            checkImage[index + 1] = c;
            checkImage[index + 2] = c;
            checkImage[index + 3] = 255;
            
            index = index + 4;
        }
    }

    checker_texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, checker_texture);
        gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, image_width, image_height, 0, gl.RGBA, gl.UNSIGNED_BYTE, checkImage);
        gl.generateMipmap(gl.TEXTURE_2D);
    gl.bindTexture(gl.TEXTURE_2D, null);

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

    var squareVertices = new Float32Array(12);

    //code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);
        //simple square
        modelViewMatrix = mat4.create();
        modelViewProjectionMatrix = mat4.create();

        mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.0]);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
        gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, checker_texture);
        gl.uniform1i(textureSamplerUniform, 0);

        gl.bindVertexArray(vao_square);

        squareVertices[0] = -2.0;
        squareVertices[1] = -1.0;
        squareVertices[2] = 0.0;

        squareVertices[3] = -2.0;
        squareVertices[4] = 1.0;
        squareVertices[5] = 0.0;

        squareVertices[6] = 0.0;
        squareVertices[7] = 1.0;
        squareVertices[8] = 0.0;

        squareVertices[9] = 0.0;
        squareVertices[10] = -1.0;
        squareVertices[11] = 0.0;

        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_position);
        gl.bufferData(gl.ARRAY_BUFFER, squareVertices, gl.DYNAMIC_DRAW);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);

        squareVertices[0] = 1.0;
        squareVertices[1] = -1.0;
        squareVertices[2] = 0.0;

        squareVertices[3] = 1.0;
        squareVertices[4] = 1.0;
        squareVertices[5] = 0.0;

        squareVertices[6] = 2.41421;
        squareVertices[7] = 1.0;
        squareVertices[8] = -1.41421;

        squareVertices[9] = 2.41421;
        squareVertices[10] = -1.0;
        squareVertices[11] = -1.41421;

        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_position);
        gl.bufferData(gl.ARRAY_BUFFER, squareVertices, gl.DYNAMIC_DRAW);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);

        gl.bindVertexArray(null);
    gl.useProgram(null);

    //animation loop
    requestAnimationFrame(render, canvas);
}

function uninitialize()
{
    //release textures
    if(checker_texture)
    {
        gl.deleteTexture(checker_texture);
        checker_texture = null;
    }

    //release vao and vbo
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

    if(vbo_square_texcoord)
    {
        gl.deleteBuffer(vbo_square_texcoord);
        vbo_square_texcoord = null;
    }

    //release sahders
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

function degreeToRad(angle)
{
    return (angle * 3.141592 / 180.0);
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
