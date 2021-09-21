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

var vao_square;
var vbo_square_position;
var vbo_square_texcoord;

var mvpUniform;
var textureSamplerUniform;
var textureToggleUniform;
var perspectiveProjectionMatrix;

var keypress = 0;
var texcoords;

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

var smiley_texture;

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
    "#version 300 es"                                               +
    "\n"                                                            +
    "precision highp float;"                                        +
    "in vec2 out_texcoord;"                                         +
    "uniform sampler2D u_textureSampler;"                           +
    "uniform int texture_toggle;"                                   +
    "out vec4 FragColor;"                                           +
    "void main(void)"                                               +
    "{"                                                             +
    "   if(texture_toggle == 0)"                                    +
    "   {"                                                          +
    "       FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);"              +
    "   }"                                                          +
    "   else"                                                       +
    "   {"                                                          +
    "       FragColor = texture(u_textureSampler, out_texcoord);"   +
    "   }"                                                          +
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
    textureToggleUniform = gl.getUniformLocation(shaderProgramObject, "texture_toggle");

    //setup vao and vbo
    var squareVertices = new Float32Array([
        1.0, 1.0, 1.0,
        -1.0, 1.0, 1.0,
        -1.0, -1.0, 1.0, 
        1.0, -1.0, 1.0,
    ]);
    
    texcoords = new Float32Array(8);

    vao_square = gl.createVertexArray();
    gl.bindVertexArray(vao_square);
        vbo_square_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_position);
            gl.bufferData(gl.ARRAY_BUFFER, squareVertices, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        vbo_square_texcoord = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_texcoord);
            gl.bufferData(gl.ARRAY_BUFFER, texcoords, gl.DYNAMIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD, 2, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD);
        gl.bindBuffer(gl.ARRAY_BUFFER, null); 
    gl.bindVertexArray(null);

    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    //load pyramid texture
    smiley_texture = gl.createTexture();
    smiley_texture.image = new Image();
    smiley_texture.image.src = "smiley.png";
    smiley_texture.image.onload = function()
    {
        gl.bindTexture(gl.TEXTURE_2D, smiley_texture);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1);

        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);

        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, smiley_texture.image);
        gl.generateMipmap(gl.TEXTURE_2D);

        gl.bindTexture(gl.TEXTURE_2D, null);
    }

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
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);
        //square
        modelViewMatrix = mat4.create();
        modelViewProjectionMatrix = mat4.create();

        mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.0]);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
        gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

        gl.uniform1i(textureToggleUniform, keypress);

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, smiley_texture);
        gl.uniform1i(textureSamplerUniform, 0);

        gl.bindVertexArray(vao_square);

        switch(keypress)
        {
            case 1:
                texcoords[0] = 1.0;
                texcoords[1] = 1.0;

                texcoords[2] = 0.0;
                texcoords[3] = 1.0;

                texcoords[4] = 0.0;
                texcoords[5] = 0.0;

                texcoords[6] = 1.0;
                texcoords[7] = 0.0;
                break;
            
            case 2:
                texcoords[0] = 0.5;
                texcoords[1] = 0.5;

                texcoords[2] = 0.0;
                texcoords[3] = 0.5;

                texcoords[4] = 0.0;
                texcoords[5] = 0.0;

                texcoords[6] = 0.5;
                texcoords[7] = 0.0;
                break;

            case 3:
                texcoords[0] = 2.0;
                texcoords[1] = 2.0;

                texcoords[2] = 0.0;
                texcoords[3] = 2.0;

                texcoords[4] = 0.0;
                texcoords[5] = 0.0;

                texcoords[6] = 2.0;
                texcoords[7] = 0.0;
                break;

            case 4:
                texcoords[0] = 0.5;
                texcoords[1] = 0.5;

                texcoords[2] = 0.5;
                texcoords[3] = 0.5;

                texcoords[4] = 0.5;
                texcoords[5] = 0.5;

                texcoords[6] = 0.5;
                texcoords[7] = 0.5;
                break;

            default:
                break;
        }

        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_texcoord);
        gl.bufferData(gl.ARRAY_BUFFER, texcoords, gl.DYNAMIC_DRAW);
        
        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
        gl.bindVertexArray(null);
    gl.useProgram(null);

    //animation loop
    requestAnimationFrame(render, canvas);
}

function uninitialize()
{
    //release textures
    if(smiley_texture)
    {
        gl.deleteTexture(stone_texture);
        stone_texture = null;
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
    
        case 49:
            keypress = 1;
            break;
        
        case 50:
            keypress = 2;
            break;
        
        case 51:
            keypress = 3;
            break;

        case 52:
            keypress = 4;
            break;
        
        default:
            keypress = 0;
            break;
    }
}

function mouseDown()
{
    //code
    
}
