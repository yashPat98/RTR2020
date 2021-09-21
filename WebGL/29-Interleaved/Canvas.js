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

var vao;
var vbo;

var modelViewMatrixUniform;
var projectionMatrixUniform;
var TKeyPressedUniform;
var LTKeyPressedUniform;
var lightDiffuseUniform;
var lightPositionUniform;
var diffuseTextureUniform;

var marble_texture;

var perspectiveProjectionMatrix;
var LKeyPressed = 0;
var TKeyPressed = 0;
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
        "#version 300 es"                                                                           +
        "\n"                                                                                        +
        
        "in vec3 vPosition;"                                                                        +
        "in vec3 vColor;"                                                                           +
        "in vec3 vNormal;"                                                                          +
        "in vec2 vTexCoord;"                                                                        +

        "uniform mat4 u_modelViewMatrix;"                                                           +
        "uniform mat4 u_projectionMatrix;"                                                          +   
        "uniform vec3 u_lightDiffuse;"                                                              +
        "uniform vec4 u_lightPosition;"                                                             +
        "uniform mediump int u_LKeyPressed;"                                                        +

        "out vec3 diffuse_light;"                                                                   +
        "out vec2 out_texcoord;"                                                                    +

        "void main(void)"                                                                           +
        "{"                                                                                         +
        "   if(u_LKeyPressed == 1)"                                                                 +
        "   {"                                                                                      +
        "       vec4 eye_coords = u_modelViewMatrix * vec4(vPosition, 1.0f);"                       +
        "       mat3 normal_matrix = mat3(transpose(inverse(u_modelViewMatrix)));"                  +
        "       vec3 tnorm = normalize(normal_matrix * vNormal);"                                   +
        "       vec3 s = normalize(vec3(u_lightPosition - eye_coords));"                            +
        "       diffuse_light = u_lightDiffuse * vColor * max(dot(s, tnorm), 0.0f);"                +
        "   }"                                                                                      +

        "   out_texcoord = vTexCoord;"                                                              +
        "   gl_Position = u_projectionMatrix * u_modelViewMatrix * vec4(vPosition, 1.0f);"          +
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
        "#version 300 es"                                                   +
        "\n"                                                                +
        "precision highp float;"                                            +
        
        "in vec3 diffuse_light;"                                            +
        "in vec2 out_texcoord;"                                             +
        "uniform sampler2D u_diffuseTexture;"                               +
        "uniform mediump int u_LKeyPressed;"                                +         
        "uniform int u_TKeyPressed;"                                        +
        "out vec4 FragColor;"                                               +
        
        "void main(void)"                                                   +
        "{"                                                                 +
        "   if(u_LKeyPressed == 1)"                                         +
        "   {"                                                              +
        "       FragColor = vec4(diffuse_light, 1.0f);"                     +
        "   }"                                                              +
        "   else"                                                           +
        "   {"                                                              +
        "       FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);"                  +
        "   }"                                                              +
        
        "   if(u_TKeyPressed == 1)"                                         +
        "   {"                                                              +
        "       FragColor *= texture(u_diffuseTexture, out_texcoord);"      +
        "   }"                                                              +
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
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_TEXTURE, "vTexCoord");

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
    modelViewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_modelViewMatrix");
    projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projectionMatrix");
    lightDiffuseUniform = gl.getUniformLocation(shaderProgramObject, "u_lightDiffuse");
    lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_lightPosition");
    diffuseTextureUniform = gl.getUniformLocation(shaderProgramObject, "u_diffuseTexture");
    TKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "u_TKeyPressed");
    LKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "u_LKeyPressed");

    //cube data
    var cubePCNT = new Float32Array([
        //near 
        +1.0, +1.0, +1.0,    +1.0, +0.0, +0.0,    +0.0, +0.0, +1.0,    +1.0, +1.0,   
        -1.0, +1.0, +1.0,    +1.0, +0.0, +0.0,    +0.0, +0.0, +1.0,    +0.0, +1.0,
        -1.0, -1.0, +1.0,    +1.0, +0.0, +0.0,    +0.0, +0.0, +1.0,    +0.0, +0.0,
        +1.0, -1.0, +1.0,    +1.0, +0.0, +0.0,    +0.0, +0.0, +1.0,    +1.0, +0.0,

        //right
        +1.0, +1.0, -1.0,    +0.0, +1.0, +0.0,    +1.0, +0.0, +0.0,    +1.0, +1.0,
        +1.0, +1.0, +1.0,    +0.0, +1.0, +0.0,    +1.0, +0.0, +0.0,    +0.0, +1.0,
        +1.0, -1.0, +1.0,    +0.0, +1.0, +0.0,    +1.0, +0.0, +0.0,    +0.0, +0.0,
        +1.0, -1.0, -1.0,    +0.0, +1.0, +0.0,    +1.0, +0.0, +0.0,    +1.0, +0.0,

        //ar
        -1.0, +1.0, -1.0,    +0.0, +0.0, +1.0,    +0.0, +0.0, -1.0,    +1.0, +1.0,
        +1.0, +1.0, -1.0,    +0.0, +0.0, +1.0,    +0.0, +0.0, -1.0,    +0.0, +1.0,
        +1.0, -1.0, -1.0,    +0.0, +0.0, +1.0,    +0.0, +0.0, -1.0,    +0.0, +0.0,
        -1.0, -1.0, -1.0,    +0.0, +0.0, +1.0,    +0.0, +0.0, -1.0,    +1.0, +0.0,

        //let
        -1.0, +1.0, -1.0,    +1.0, +0.0, +1.0,    -1.0, +0.0, +0.0,    +1.0, +1.0,
        -1.0, +1.0, +1.0,    +1.0, +0.0, +1.0,    -1.0, +0.0, +0.0,    +0.0, +1.0,
        -1.0, -1.0, +1.0,    +1.0, +0.0, +1.0,    -1.0, +0.0, +0.0,    +0.0, +0.0,
        -1.0, -1.0, -1.0,    +1.0, +0.0, +1.0,    -1.0, +0.0, +0.0,    +1.0, +0.0,

        //top
        +1.0, +1.0, -1.0,    +1.0, +1.0, +0.0,    +0.0, +1.0, +0.0,    +1.0, +1.0,
        -1.0, +1.0, -1.0,    +1.0, +1.0, +0.0,    +0.0, +1.0, +0.0,    +0.0, +1.0,
        -1.0, +1.0, +1.0,    +1.0, +1.0, +0.0,    +0.0, +1.0, +0.0,    +0.0, +0.0,
        +1.0, +1.0, +1.0,    +1.0, +1.0, +0.0,    +0.0, +1.0, +0.0,    +1.0, +0.0,

        //bottom
        -1.0, -1.0, -1.0,    +0.0, +1.0, +1.0,    +0.0, -1.0, +0.0,    +1.0, +1.0,
        +1.0, -1.0, -1.0,    +0.0, +1.0, +1.0,    +0.0, -1.0, +0.0,    +0.0, +1.0,
        +1.0, -1.0, +1.0,    +0.0, +1.0, +1.0,    +0.0, -1.0, +0.0,    +0.0, +0.0,
        -1.0, -1.0, +1.0,    +0.0, +1.0, +1.0,    +0.0, -1.0, +0.0,    +1.0, +0.0,
    ]);

    //setup vao and vbo
    vao = gl.createVertexArray();
    gl.bindVertexArray(vao);
        vbo = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
            gl.bufferData(gl.ARRAY_BUFFER, cubePCNT, gl.STATIC_DRAW);
            //position
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 11 * 4, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
            //color
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 11 * 4, 3 * 4);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
            //normal
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 11 * 4, 6 * 4);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_NORMAL);
            //texcoord
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXTURE, 2, gl.FLOAT, false, 11 * 4, 9 * 4);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXTURE);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    //load marble texture
    marble_texture = gl.createTexture();
    marble_texture.image = new Image();
    marble_texture.image.src = "marble.png";
    marble_texture.image.onload = function()
    {
        gl.bindTexture(gl.TEXTURE_2D, marble_texture);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1);

        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);

        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, marble_texture.image);
        gl.generateMipmap(gl.TEXTURE_2D);

        gl.bindTexture(gl.TEXTURE_2D, null);
    }

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
        mat4.rotateX(modelViewMatrix, modelViewMatrix, degreeToRad(cube_angle));
        mat4.rotateY(modelViewMatrix, modelViewMatrix, degreeToRad(cube_angle));
        mat4.rotateZ(modelViewMatrix, modelViewMatrix, degreeToRad(cube_angle));

        gl.uniformMatrix4fv(modelViewMatrixUniform, false, modelViewMatrix);
        gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

        gl.uniform1i(LKeyPressedUniform, LKeyPressed);
        gl.uniform1i(TKeyPressedUniform, TKeyPressed);
        
        if(LKeyPressed == 1)
        {
            gl.uniform3f(lightDiffuseUniform, 1.0, 1.0, 1.0);
            gl.uniform4f(lightPositionUniform, 0.0, 0.0, 5.0, 1.0);
        }

        if(TKeyPressed == 1)
        {
            gl.activeTexture(gl.TEXTURE0);
            gl.bindTexture(gl.TEXTURE_2D, marble_texture);
            gl.uniform1i(diffuseTextureUniform, 0);
        }


        gl.bindVertexArray(vao);
        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);
        gl.bindVertexArray(null);

        gl.bindTexture(gl.TEXTURE_2D, null);
    gl.useProgram(null);

    //update 
    cube_angle += 0.5;
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
    if(marble_texture)
    {
        gl.deleteTexture(marble_texture);
        marble_texture = null;
    }

    if(vao)
    {
        gl.deleteVertexArray(vao);
        vao = null;
    }

    if(vbo)
    {
        gl.deleteBuffer(vbo);
        vbo = null;
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

        case 84:
            if(TKeyPressed == 0)
            {
                TKeyPressed = 1;
            }
            else 
            {
                TKeyPressed = 0;
            }
            break;
    }
}

function mouseDown()
{
    //code
    
}
