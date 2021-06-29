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

var vao_pyramid;
var vbo_pyramid_position;
var vbo_pyramid_texcoord;

var vao_cube;
var vbo_cube_position;
var vbo_cube_texcoord;

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

var stone_texture;
var kundali_texture;
var pyramid_rotation_angle = 0.0;
var cube_rotation_angle = 0.0;

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
    var pyramidVertices = new Float32Array([
                                            //near
                                            0.0, 1.0, 0.0,
                                            -1.0, -1.0, 1.0,
                                            1.0, -1.0, 1.0,

                                            //right
                                            0.0, 1.0, 0.0,
                                            1.0, -1.0, 1.0,
                                            1.0, -1.0, -1.0,
                                        
                                            //far
                                            0.0, 1.0, 0.0,
                                            1.0, -1.0, -1.0,
                                            -1.0, -1.0, -1.0,

                                            //left
                                            0.0, 1.0, 0.0,
                                            -1.0, -1.0, -1.0,
                                            -1.0, -1.0, 1.0
                                        ]);
    
    var pyramidTexCoords = new Float32Array([
                                            //near 
                                            0.5, 1.0,
                                            0.0, 0.0,
                                            1.0, 0.0,

                                            //right
                                            0.5, 1.0, 
                                            0.0, 0.0,
                                            1.0, 0.0,

                                            //far
                                            0.5, 1.0, 
                                            0.0, 0.0, 
                                            1.0, 0.0,

                                            //left
                                            0.5, 1.0, 
                                            0.0, 0.0, 
                                            1.0, 0.0
                                        ]);        

    var cubeVertices = new Float32Array([
                                            //near 
                                            1.0, 1.0, 1.0,
                                            -1.0, 1.0, 1.0,
                                            -1.0, -1.0, 1.0, 
                                            1.0, -1.0, 1.0,

                                            //right
                                            1.0, 1.0, -1.0,
                                            1.0, 1.0, 1.0,
                                            1.0, -1.0, 1.0,
                                            1.0, -1.0, -1.0,

                                            //far
                                            -1.0, 1.0, -1.0,
                                            1.0, 1.0, -1.0,
                                            1.0, -1.0, -1.0,
                                            -1.0, -1.0, -1.0,

                                            //left
                                            -1.0, 1.0, -1.0,
                                            -1.0, 1.0, 1.0, 
                                            -1.0, -1.0, 1.0,
                                            -1.0, -1.0, -1.0,

                                            //top
                                            1.0, 1.0, -1.0,
                                            -1.0, 1.0, -1.0,
                                            -1.0, 1.0, 1.0,
                                            1.0, 1.0, 1.0,

                                            //bottom
                                            -1.0, -1.0, -1.0,
                                            1.0, -1.0, -1.0,
                                            1.0, -1.0, 1.0,
                                            -1.0, -1.0, 1.0
                                        ]);
    
    var cubeTexCoords = new Float32Array([
                                            //near
                                            1.0, 1.0, 
                                            0.0, 1.0,
                                            0.0, 0.0, 
                                            1.0, 0.0,

                                            //right
                                            1.0, 1.0, 
                                            0.0, 1.0,
                                            0.0, 0.0, 
                                            1.0, 0.0,

                                            //far 
                                            1.0, 1.0, 
                                            0.0, 1.0,
                                            0.0, 0.0, 
                                            1.0, 0.0,

                                            //left
                                            1.0, 1.0, 
                                            0.0, 1.0,
                                            0.0, 0.0, 
                                            1.0, 0.0,

                                            //top
                                            1.0, 1.0, 
                                            0.0, 1.0,
                                            0.0, 0.0, 
                                            1.0, 0.0,

                                            //bottom
                                            1.0, 1.0, 
                                            0.0, 1.0,
                                            0.0, 0.0, 
                                            1.0, 0.0,
                                        ]);

    vao_pyramid = gl.createVertexArray();
    gl.bindVertexArray(vao_pyramid);
        vbo_pyramid_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid_position);
            gl.bufferData(gl.ARRAY_BUFFER, pyramidVertices, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        vbo_pyramid_texcoord = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid_texcoord);
            gl.bufferData(gl.ARRAY_BUFFER, pyramidTexCoords, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD, 2, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD);
        gl.bindBuffer(gl.ARRAY_BUFFER, null); 
    gl.bindVertexArray(null);

    vao_cube = gl.createVertexArray();
    gl.bindVertexArray(vao_cube);
        vbo_cube_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_cube_position);
            gl.bufferData(gl.ARRAY_BUFFER, cubeVertices, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        vbo_cube_texcoord = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_cube_texcoord);
            gl.bufferData(gl.ARRAY_BUFFER, cubeTexCoords, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD, 2, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD);
        gl.bindBuffer(gl.ARRAY_BUFFER, null); 
    gl.bindVertexArray(null);

    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    //load pyramid texture
    stone_texture = gl.createTexture();
    stone_texture.image = new Image();
    stone_texture.image.src = "stone.png";
    stone_texture.image.onload = function()
    {
        gl.bindTexture(gl.TEXTURE_2D, stone_texture);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1);

        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);

        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, stone_texture.image);
        gl.generateMipmap(gl.TEXTURE_2D);

        gl.bindTexture(gl.TEXTURE_2D, null);
    }

    //load kundli texture
    kundali_texture = gl.createTexture();
    kundali_texture.image = new Image();
    kundali_texture.image.src = "Vijay_Kundali.png";
    kundali_texture.image.onload = function()
    {
        gl.bindTexture(gl.TEXTURE_2D, kundali_texture);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1);

        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);

        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, kundali_texture.image);
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
        //pyramid
        mat4.translate(modelViewMatrix, modelViewMatrix, [-2.0, 0.0, -6.0]);
        mat4.rotateY(modelViewMatrix, modelViewMatrix, degreeToRad(pyramid_rotation_angle));
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
        gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, stone_texture);
        gl.uniform1i(textureSamplerUniform, 0);

        gl.bindVertexArray(vao_pyramid);
        gl.drawArrays(gl.TRIANGLES, 0, 12);
        gl.bindVertexArray(null);

        //cube
        modelViewMatrix = mat4.create();
        modelViewProjectionMatrix = mat4.create();

        mat4.translate(modelViewMatrix, modelViewMatrix, [2.0, 0.0, -6.0]);
        mat4.rotateX(modelViewMatrix, modelViewMatrix, degreeToRad(cube_rotation_angle));
        mat4.rotateY(modelViewMatrix, modelViewMatrix, degreeToRad(cube_rotation_angle));
        mat4.rotateZ(modelViewMatrix, modelViewMatrix, degreeToRad(cube_rotation_angle));
        mat4.scale(modelViewMatrix, modelViewMatrix, [0.85, 0.85, 0.85]);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
        gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, kundali_texture);
        gl.uniform1i(textureSamplerUniform, 0);

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
    pyramid_rotation_angle += 1.0;
    if(pyramid_rotation_angle >= 360.0)
        pyramid_rotation_angle = 0.0;

    cube_rotation_angle += 1.0;
    if(cube_rotation_angle >= 360.0)
        cube_rotation_angle = 0.0;

    //animation loop
    requestAnimationFrame(render, canvas);
}

function uninitialize()
{
    //release textures
    if(stone_texture)
    {
        gl.deleteTexture(stone_texture);
        stone_texture = null;
    }

    if(kundali_texture)
    {
        gl.deleteTexture(kundali_texture);
        kundali_texture = null;
    }

    //release vao and vbo
    if(vao_pyramid)
    {
        gl.deleteVertexArray(vao_pyramid);
        vao_pyramid = null;
    }

    if(vbo_pyramid_position)
    {
        gl.deleteBuffer(vbo_pyramid_position);
        vbo_pyramid_position = null;
    }

    if(vbo_pyramid_texcoord)
    {
        gl.deleteBuffer(vbo_pyramid_texcoord);
        vbo_pyramid_texcoord = null;
    }

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

    if(vbo_cube_texcoord)
    {
        gl.deleteBuffer(vbo_cube_texcoord);
        vbo_cube_texcoord = null;
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
