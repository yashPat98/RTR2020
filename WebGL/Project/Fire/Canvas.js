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

var ps_vertexShaderObject;
var ps_fragmentShaderObject;
var ps_shaderProgramObject;

var ps_timeUniform;
var ps_PointSizeUniform;
var ps_sTextureUniform;

var ps_modelMatrixUniform;
var ps_viewMatrixUniform;
var ps_projectionMatrixUniform;

var ps_fadeinFactorUniform;
var ps_fadeoutFactorUniform;

var vao_particle;
var vbo_particle_lifeTime;
var vbo_particle_xPos;
var vbo_particle_YSpeed;
var vbo_particle_color;

var attrib_lifetime = 0;
var attrib_xPos = 1;
var attrib_YSpeed = 2;
var attrib_color = 3;

var texture_fire;

var perspectiveProjectionMatrix;

var num_particles = 150;
var FadeInFactor = 1.0;
var FadeOutFactor = 1.0;
var t = 0.0;

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
    var ps_vertexShaderSourceCode = 
        "#version 300 es"                           +
		"\n"                                        +

		"in float aLifetime;"                       +
		"in float aXPos;"                           +
		"in float aYSpeed;"                         +
		"in vec2 aColor;"                           +

		"uniform float uTime;"                      +
		"uniform float uPointSize;"                 +

		"uniform mat4 u_model_matrix;"              +
		"uniform mat4 u_view_matrix;"               +
		"uniform mat4 u_projection_matrix;"         +

		"out float vLifetime;"                      +
		"out vec2 color;"                           +

		"void main(void)"                                                                                                       +
		"{"                                                                                                                     +
            "gl_PointSize = 30.0;"                                                                                              +
            "vLifetime		= mod(uTime, aLifetime);"                                                                           +
			"float ti		= 1.0 - vLifetime/aLifetime;"                                                                       +

			"mat4 mv_matrix = u_view_matrix * u_model_matrix;"                                                                  +

			"gl_Position	= u_projection_matrix * mv_matrix * vec4(aXPos * ti, aYSpeed * vLifetime - 1.0, 1.0, 1.0);"                                           +
			"vLifetime		= 4.0 * ti * (1.0 - ti);"                                                                           +
			"color			= aColor;"                                                                                          +
        "}";

    ps_vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(ps_vertexShaderObject, ps_vertexShaderSourceCode);
    gl.compileShader(ps_vertexShaderObject);
    if(gl.getShaderParameter(ps_vertexShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(ps_vertexShaderObject);
        if(error.length > 0)
        {
            alert("Vertex Shader : " + error);
            uninitialize();
        }
    }

    //fragment shader 
    var ps_fragmentShaderSourceCode = 
        "#version 300 es"                       +
		"\n"                                    +
        "precision mediump float;"              +

		"uniform sampler2D sTexture;"           +

		"in float vLifetime;"                   +
		"in vec2 color;"                        +

		"out vec4 FragColor;"                   +

		"uniform float fadeinFactor;"           +
		"uniform float fadeoutFactor;"          +

		"void main(void)"                                                           +
		"{"                                                                         +
			"vec4 ColorTemp;"                                                       +
			"vec4 texColor	= texture(sTexture, gl_PointCoord);"                    +
			"ColorTemp		= vec4(color, 0.0, 1.0) * texColor ;"                   +
			"ColorTemp.a	= vLifetime + 0.75;"                                    +
            "if(ColorTemp.r < 0.1 && ColorTemp.g < 0.1 && ColorTemp.b < 0.1)"       +
			"discard;"                                                              +
			"FragColor		= ColorTemp * fadeinFactor * fadeoutFactor;"            +
		"}";

    ps_fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(ps_fragmentShaderObject, ps_fragmentShaderSourceCode);
    gl.compileShader(ps_fragmentShaderObject);
    if(gl.getShaderParameter(ps_fragmentShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(ps_fragmentShaderObject);
        if(error.length > 0)
        {
            alert("Fragment Shader : " + error);
            uninitialize();
        }
    }

    //shader program
    ps_shaderProgramObject = gl.createProgram();
    gl.attachShader(ps_shaderProgramObject, ps_vertexShaderObject);
    gl.attachShader(ps_shaderProgramObject, ps_fragmentShaderObject);

    //pre-linking binding of shader program object with vertex shader attributes
    gl.bindAttribLocation(ps_shaderProgramObject, attrib_lifetime, "aLifetime");
    gl.bindAttribLocation(ps_shaderProgramObject, attrib_xPos, "aXPos");
    gl.bindAttribLocation(ps_shaderProgramObject, attrib_YSpeed, "aYSpeed");
    gl.bindAttribLocation(ps_shaderProgramObject, attrib_color, "aColor");

    //linking 
    gl.linkProgram(ps_shaderProgramObject);
    if(!gl.getProgramParameter(ps_shaderProgramObject, gl.LINK_STATUS))
    {
        var error = gl.getProgramInfoLog(ps_shaderProgramObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //get MVP uniform location
    ps_timeUniform = gl.getUniformLocation(ps_shaderProgramObject, "uTime");
    ps_PointSizeUniform = gl.getUniformLocation(ps_shaderProgramObject, "uPointSize");
    ps_sTextureUniform = gl.getUniformLocation(ps_shaderProgramObject, "sTexture");

    ps_modelMatrixUniform = gl.getUniformLocation(ps_shaderProgramObject, "u_model_matrix");
    ps_viewMatrixUniform = gl.getUniformLocation(ps_shaderProgramObject, "u_view_matrix");
    ps_projectionMatrixUniform = gl.getUniformLocation(ps_shaderProgramObject, "u_projection_matrix");

    ps_fadeinFactorUniform = gl.getUniformLocation(ps_shaderProgramObject, "fadeinFactor");
    ps_fadeoutFactorUniform = gl.getUniformLocation(ps_shaderProgramObject, "fadeoutFactor");

    //set up vao and vbo for particle system
    var lifetimes = [];
    var xPos = [];
    var ySpeed = [];
    var colors = [];
    
    for(var i = 0; i < num_particles; i++)
    {
      lifetimes.push(2.0 * Math.random() + 1.0);
      xPos.push(1.5 * (Math.random() - 1.5));
      ySpeed.push(0.7 * Math.random());
      colors.push(Math.random());  
      colors.push(0.2 * Math.random());
    }

    vao_particle = gl.createVertexArray();
    gl.bindVertexArray(vao_particle);
        
        //lifetimes
        vbo_particle_lifeTime = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_particle_lifeTime);
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(lifetimes), gl.STATIC_DRAW);
            gl.vertexAttribPointer(attrib_lifetime, 1, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(attrib_lifetime);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        //xPos
        vbo_particle_xPos = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_particle_xPos);
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(xPos), gl.STATIC_DRAW);
            gl.vertexAttribPointer(attrib_xPos, 1, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(attrib_xPos);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        //YSpeed
        vbo_particle_YSpeed = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_particle_YSpeed);
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(ySpeed), gl.STATIC_DRAW);
            gl.vertexAttribPointer(attrib_YSpeed, 1, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(attrib_YSpeed);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        //colors
        vbo_particle_color = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_particle_color);
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);
            gl.vertexAttribPointer(attrib_color, 2, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(attrib_color);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    gl.enable(gl.BLEND);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE);

    //gl.enable(gl.POINT_SPRITE);

    //load fire texture
    texture_fire = gl.createTexture();
    texture_fire.image = new Image();
    texture_fire.image.src = "particle.png";
    texture_fire.image.onload = function()
    {
        gl.bindTexture(gl.TEXTURE_2D, texture_fire);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1);

        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);

        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texture_fire.image);
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
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var scaleMatrix = mat4.create();
    var rotateMatrix = mat4.create();

    //code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(ps_shaderProgramObject);
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -5.0]);
        mat4.scale(modelMatrix, modelMatrix, [0.1, 0.2, 1.0]);
        
        gl.uniformMatrix4fv(ps_modelMatrixUniform, false, modelMatrix);
        gl.uniformMatrix4fv(ps_viewMatrixUniform, false, viewMatrix);
        gl.uniformMatrix4fv(ps_projectionMatrixUniform, false, perspectiveProjectionMatrix);

        gl.uniform1f(ps_fadeoutFactorUniform, FadeOutFactor);
        gl.uniform1f(ps_fadeinFactorUniform, FadeInFactor);        
        gl.uniform1f(ps_timeUniform, t);

        //gl.pointSize(35);
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, texture_fire);
        gl.uniform1i(ps_sTextureUniform, 0);

        gl.bindVertexArray(vao_particle);
        gl.drawArrays(gl.POINTS, 0, num_particles);
        
        gl.bindTexture(gl.TEXTURE_2D, null);
        gl.bindVertexArray(null);
    gl.useProgram(null);
    
    //update
    t += 0.1;
    if(t > 360.0)
        t = 0.0;

    //animation loop
    requestAnimationFrame(render, canvas);
}

function uninitialize()
{
    //release texture
    if(texture_fire)
    {
        gl.deleteTexture(texture_fire);
        texture_fire = null;
    }

    //release vao and vbo for particle
    if(vao_particle)
    {
        gl.deleteVertexArray(vao_particle);
        vao_particle = null;
    }

    if(vbo_particle_lifeTime)
    {
        gl.deleteBuffer(vbo_particle_lifeTime);
        vbo_particle_lifeTime = null;
    }

    if(vbo_particle_xPos)
    {
        gl.deleteBuffer(vbo_particle_xPos);
        vbo_particle_xPos = null;
    }

    if(vbo_particle_YSpeed)
    {
        gl.deleteBuffer(vbo_particle_YSpeed);
        vbo_particle_YSpeed = null;
    }

    if(vbo_particle_color)
    {
        gl.deleteBuffer(vbo_particle_color);
        vbo_particle_color = null;
    }

    //release shaders
    if(ps_shaderProgramObject)
    {
        if(ps_fragmentShaderObject)
        {
            gl.detachShader(ps_shaderProgramObject, ps_fragmentShaderObject);
            gl.deleteShader(ps_fragmentShaderObject);
            ps_fragmentShaderObject = null;
        }

        if(ps_vertexShaderObject)
        {
            gl.detachShader(ps_shaderProgramObject, ps_vertexShaderObject);
            gl.deleteShader(ps_vertexShaderObject);
            ps_vertexShaderObject = null;
        }

        gl.deleteProgram(ps_shaderProgramObject);
        ps_shaderProgramObject = null;
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
