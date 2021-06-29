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

var noise_vertexShaderObject;
var noise_fragmentShaderObject;
var noise_shaderProgramObject;

var vao_rectangle;
var vbo_rectangle_position;
var vbo_rectangle_texcoord;

var noise_time_uniform;
var noise_resolution_uniform;
var noise_alpha_uniform;
var noise_samplerUniform;

var perspectiveProjectionMatrix;
var texture_vitthal;

var timer = 0.0;
var noise_alpha_val = 0.0;

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
    var noise_vertexShaderSourceCode = 
        "#version 300 es"                                       +
		"\n"                                                    +
        "precision mediump float;"                              +

		"in vec4 vPosition;"                                    +   
		"in vec2 vTexcoord;"                                    +

		"out vec4 out_non_transformed_pos;"                     +
		"out vec2 out_texcoord;"                                +

		"void main(void)"                                       +
		"{"                                                     +
			"out_non_transformed_pos	= vPosition;"           +
			"out_texcoord				= vTexcoord;"           +   
			"gl_Position				= vPosition;"           +
		"}";

    noise_vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(noise_vertexShaderObject, noise_vertexShaderSourceCode);
    gl.compileShader(noise_vertexShaderObject);
    if(gl.getShaderParameter(noise_vertexShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(noise_vertexShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //fragment shader 
    var noise_fragmentShaderSourceCode = 
        "#version 300 es"                                       +
		"\n"                                                    +
        "precision highp float;"                                +

		"uniform float u_time;"                                 +
		"uniform vec2 u_resolution;"                            +
		"uniform float u_noise_alpha;"                          +

		"in vec4 out_non_transformed_pos;"                      +
		"in vec2 out_texcoord;"                                 +

		"uniform sampler2D u_sampler;"                          +

		"int iterations = 4;"                                   +

		"out vec4 FragColor;"                                   +

		"vec3 hash_val(vec3 p)"                                 +
		"{"                                                     +
		    "vec3 q = vec3(dot(p, vec3(127.1, 311.7, 189.2)),dot(p, vec3(269.5, 183.3, 324.7)),dot(p, vec3(419.2, 371.9, 128.5)));"     +
		    "return fract(sin(q) * 43758.5453);"                +
		"}"                                                     +


		"float noise_val(vec3 x, float v) "                     +
		"{"                                                     +
		    "vec3 p		= floor(x);"                                +
		    "vec3 f		= fract(x);"                                +
		    "float s	= 1.0 + 0.0 * v;"                           +
		    "float va	= 0.0;"                                     +
		    "float wt	= 0.0;"                                     +   
		    "for (int k = -2; k <= 1; k++)"                         +
		    "{"                                                     +
		        "for (int j = -2; j <= 1; j++)"                                             +
		        "{"                                                                         +
		            "for (int i = -2; i <= 1; i++) "                                        +
		            "{"                                                                     +
		                "vec3 g		= vec3(float(i), float(j), float(k));"                  +
		                "vec3 o		= hash_val(p + g);"                                     +
		                "vec3 r		= g - f + o + 0.5;"                                     +
		                "float d	= dot(r, r);"                                           +
		                "float w	= pow(1.0 - smoothstep(0.0, 1.214, sqrt(d)), s);"       + 
		                "va			+= o.z * w;"                                            +
		                "wt			+= w;"                                                  +
		            "}"                                                                     +
		        "}"                                                                         +
		    "}"                                                                             +
		    "return va / wt;"                                                               +
		"}"                                                     +

		"float fBm(vec3 p, float v)"                            +
		"{"                                                     +
		    "float sum = 0.0;"                                  +
		    "float amp = 1.0;"                                  +
		    "for (int i = 0; i < iterations; i++)"              +
		    "{"                                                 +
		        "sum	+= amp * noise_val(p, v);"              +
		        "amp	= amp * 0.5;"                           +
		        "p		= p * 2.0;"                             +
		    "}"                                                 +
		    "return sum;"                                       +
		"}"                                                     +

		"void main(void)"                                       +
		"{"                                                     +
			"vec2 p			= out_non_transformed_pos.xy;"      +
			"vec3 rd		= normalize(vec3(p.xy, 1.0));	"   +
			/*pos1 : for behavour of noise animation*/  
			"vec3 pos1		= vec3(0.0, 0.0, 1.0) * u_time + rd * 15.0;"                                    + 
			"vec3 color		= vec3(0.75 * fBm(pos1, 0.2), 0.75 * fBm(pos1, 0.2), 0.75 * fBm(pos1, 0.2));"   + 
			"vec4 vitthal_texture = texture(u_sampler,out_texcoord);"                                       +
			"if(vitthal_texture.r < 0.1 && vitthal_texture.g < 0.1 && vitthal_texture.b < 0.1)"             +
			"{"                                                                                             +
				"vitthal_texture.r = 0.575;"                                                                +
				"vitthal_texture.g = 0.50;"                                                                 +
				"vitthal_texture.b = 0.50;"                                                                 +
			"}"                                                                                             +
			"else"                                                                                          +
			"{"                                                                                             +
				"vitthal_texture.r = 0.5;"                                                                  +
				"vitthal_texture.g = 0.5;"                                                                  +
				"vitthal_texture.b = 0.5;"                                                                  +
			"}"                                                                                             +
			"FragColor		= (vec4(color,1.0) * vec4(0.8,0.8,0.8,1.0) * u_noise_alpha) * vitthal_texture;" +
		"}";                    

    noise_fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(noise_fragmentShaderObject, noise_fragmentShaderSourceCode);
    gl.compileShader(noise_fragmentShaderObject);
    if(gl.getShaderParameter(noise_fragmentShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(noise_fragmentShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //shader program
    noise_shaderProgramObject = gl.createProgram();
    gl.attachShader(noise_shaderProgramObject, noise_vertexShaderObject);
    gl.attachShader(noise_shaderProgramObject, noise_fragmentShaderObject);

    //pre-linking binding of shader program object with vertex shader attributes
    gl.bindAttribLocation(noise_shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_VERTEX, "vPosition");
    gl.bindAttribLocation(noise_shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_TEXTURE, "vTexcoord");

    //linking 
    gl.linkProgram(noise_shaderProgramObject);
    if(!gl.getProgramParameter(noise_shaderProgramObject, gl.LINK_STATUS))
    {
        var error = gl.getProgramInfoLog(noise_shaderProgramObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //get MVP uniform location
    noise_time_uniform = gl.getUniformLocation(noise_shaderProgramObject, "u_time");
    noise_resolution_uniform = gl.getUniformLocation(noise_shaderProgramObject, "u_resolution");
    noise_alpha_uniform = gl.getUniformLocation(noise_shaderProgramObject, "u_noise_alpha");
    noise_samplerUniform = gl.getUniformLocation(noise_shaderProgramObject, "u_sampler");

    //setup vao and vbo for noise rectangle
    var rectangleVertices = new Float32Array([
        1.0, 1.0, 0.0,
		-1.0, 1.0, 0.0,
		-1.0,-1.0, 0.0,
		 1.0,-1.0, 0.0
    ]);

    var rectangleTexcoord = new Float32Array([
        1.65, 1.0,
		-0.65, 1.0,
		-0.65, 0.0,
		 1.65, 0.0
    ]);

    vao_rectangle = gl.createVertexArray();
    gl.bindVertexArray(vao_rectangle);
        
        vbo_rectangle_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_rectangle_position);
            gl.bufferData(gl.ARRAY_BUFFER, rectangleVertices, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    
        vbo_rectangle_texcoord = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_rectangle_texcoord);
            gl.bufferData(gl.ARRAY_BUFFER, rectangleTexcoord, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXTURE, 2, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXTURE);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    //load vithal texture
    texture_vitthal = gl.createTexture();
    texture_vitthal.image = new Image();
    texture_vitthal.image.src = "vitthal.png";
    texture_vitthal.image.onload = function()
    {
        gl.bindTexture(gl.TEXTURE_2D, texture_vitthal);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1);

        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);

        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texture_vitthal.image);
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
    //code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(noise_shaderProgramObject);
        gl.uniform1f(noise_time_uniform, timer);
        gl.uniform1f(noise_alpha_uniform, noise_alpha_val);
        gl.uniform2f(noise_resolution_uniform, canvas.width, canvas.height);

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, texture_vitthal);
        gl.uniform1i(noise_samplerUniform, 0);

        gl.bindVertexArray(vao_rectangle);
        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
        gl.bindVertexArray(null);
    gl.useProgram(null);

    //update
    timer = timer + 0.1;

    if(noise_alpha_val < 1.0)
        noise_alpha_val = noise_alpha_val + 0.01;

    //animation loop
    requestAnimationFrame(render, canvas);
}

function uninitialize()
{
    //release texture
    if(texture_vitthal)
    {
        gl.deleteTexture(texture_vitthal);
        texture_vitthal = 0;
    }

    //release vao and vbo
    if(vao_rectangle)
    {
        gl.deleteVertexArray(vao_rectangle);
        vao_rectangle = null;
    }

    if(vbo_rectangle_position)
    {
        gl.deleteBuffer(vbo_rectangle_position);
        vbo_rectangle_position = null;
    }

    if(vbo_rectangle_texcoord)
    {
        gl.deleteBuffer(vbo_rectangle_texcoord);
        vbo_rectangle_texcoord = null;
    }

    //release shaders
    if(noise_shaderProgramObject)
    {
        if(noise_fragmentShaderObject)
        {
            gl.detachShader(noise_shaderProgramObject, noise_fragmentShaderObject);
            gl.deleteShader(noise_fragmentShaderObject);
            noise_fragmentShaderObject = null;
        }

        if(noise_vertexShaderObject)
        {
            gl.detachShader(noise_shaderProgramObject, noise_vertexShaderObject);
            gl.deleteShader(noise_vertexShaderObject);
            noise_vertexShaderObject = null;
        }

        gl.deleteProgram(noise_shaderProgramObject);
        noise_shaderProgramObject = null;
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
