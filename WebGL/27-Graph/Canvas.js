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

var vao;                        
var vbo_position;     

var vao_x;
var vbo_x_position;

var vao_y;
var vbo_y_position;

var vao_circle;
var vbo_circle_position;

var vao_square;
var vbo_square_position;

var vao_triangle;
var vbo_triangle_position;

var vao_incircle;
var vbo_incircle_position;

var mvpUniform;
var materialUniform;
var perspectiveProjectionMatrix;

var vertices_line_count = 0;
var vertices_circle_count = 0;
var vertices_incircle_count = 0;

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

    //setup data
    var vertices_line_buffer = [];
    var fInterval = 0.05;
    for(var fStep = -20.0; fStep <= 20.0; fStep++)
    {  
        vertices_line_buffer.push(-1.0);
        vertices_line_buffer.push(fInterval * fStep);
        vertices_line_buffer.push(0.0);

        vertices_line_buffer.push(1.0);
        vertices_line_buffer.push(fInterval * fStep);
        vertices_line_buffer.push(0.0);

        vertices_line_buffer.push(fInterval * fStep);
        vertices_line_buffer.push(-1.0);
        vertices_line_buffer.push(0.0);

        vertices_line_buffer.push(fInterval * fStep);
        vertices_line_buffer.push(1.0);
        vertices_line_buffer.push(0.0);

        vertices_line_count += 12;
    }
    var vertices_line = new Float32Array(vertices_line_buffer);

    var x_axis = new Float32Array([
        -1.0, 0.0, 0.0,
        1.0, 0.0, 0.0
    ]);

    var y_axis = new Float32Array([
        0.0, -1.0, 0.0,
        0.0, 1.0, 0.0
    ]);

    var square = new Float32Array([
        Math.cos(0.785375), Math.sin(0.785375), 0.0,
        Math.cos(Math.PI - 0.785375), Math.sin(Math.PI - 0.785375), 0.0,
        -Math.cos(0.785375), -Math.sin(0.785375), 0.0,
        Math.sin(Math.PI - 0.785375), Math.cos(Math.PI - 0.785375), 0.0
    ]);

    var triangle = new Float32Array([
        0.0, (Math.cos(0.785375) - Math.cos(Math.PI - 0.785375)) / 2.0, 0.0,
        -Math.cos(0.785375), -Math.sin(0.785375), 0.0,
        Math.sin(Math.PI - 0.785375), Math.cos(Math.PI - 0.785375), 0.0
    ]);

    var vertices_circle_buffer = [];
    for(var angle = 0.0; angle <= (2.0 * Math.PI); angle += 0.1)
    {
        var x = Math.sin(angle);
        var y = Math.cos(angle);

        vertices_circle_buffer.push(x);
        vertices_circle_buffer.push(y);
        vertices_circle_buffer.push(0.0); 
        
        vertices_circle_count += 3;
    }
    var vertices_circle = new Float32Array(vertices_circle_buffer);

    //incircle
    var lab = distance(0.0, (Math.cos(0.785375) - Math.cos(Math.PI - 0.785375)) / 2.0, -Math.cos(0.785375), -Math.sin(0.785375));
    var lbc = distance(-Math.cos(0.785375), -Math.sin(0.785375), Math.sin(Math.PI - 0.785375), Math.cos(Math.PI - 0.785375));
    var lac = distance(0.0, (Math.cos(0.785375) - Math.cos(Math.PI - 0.785375)) / 2.0, Math.sin(Math.PI - 0.785375), Math.cos(Math.PI - 0.785375));
    var sum = lab + lbc + lac;

    var xin = ((lbc * 0.0) + (lac * (-Math.cos(0.785375))) + (lab * Math.sin(Math.PI - 0.785375))) / sum;
    var yin = ((lbc * ((Math.cos(0.785375) - Math.cos(Math.PI - 0.785375)) / 2.0)) + (lac * (-Math.sin(0.785375))) + (lab * Math.cos(Math.PI - 0.785375))) / sum;

    var semi = (lab + lbc + lac) / 2;
    var radius = Math.sqrt(semi * (semi - lab) * (semi - lbc) * (semi - lac)) / semi;

    var vertices_incircle_buffer = [];
    for(var angle = 0.0; angle <= (2 * Math.PI); angle += 0.1)
    {
        var x = radius * Math.sin(angle);
        var y = radius * Math.cos(angle);

        vertices_incircle_buffer.push(x + xin);
        vertices_incircle_buffer.push(y + yin);
        vertices_incircle_buffer.push(0.0);

        vertices_incircle_count += 3;
    }
    var vertices_incircle = new Float32Array(vertices_incircle_buffer);

    vao = gl.createVertexArray();
    gl.bindVertexArray(vao);
        vbo_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position);
            gl.bufferData(gl.ARRAY_BUFFER, vertices_line, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    vao_x = gl.createVertexArray();
    gl.bindVertexArray(vao_x);
        vbo_x_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_x_position);
            gl.bufferData(gl.ARRAY_BUFFER, x_axis, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    vao_y = gl.createVertexArray();
    gl.bindVertexArray(vao_y);
        vbo_y_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_y_position);
            gl.bufferData(gl.ARRAY_BUFFER, y_axis, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    vao_circle = gl.createVertexArray();
    gl.bindVertexArray(vao_circle);
        vbo_circle_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_circle_position);
            gl.bufferData(gl.ARRAY_BUFFER, vertices_circle, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    vao_square = gl.createVertexArray();
    gl.bindVertexArray(vao_square);
        vbo_square_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_position);
            gl.bufferData(gl.ARRAY_BUFFER, square, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    vao_triangle = gl.createVertexArray();
    gl.bindVertexArray(vao_triangle);
        vbo_triangle_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_triangle_position);
            gl.bufferData(gl.ARRAY_BUFFER, triangle, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    vao_incircle = gl.createVertexArray();
    gl.bindVertexArray(vao_incircle);
        vbo_incircle_position = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_incircle_position);
            gl.bufferData(gl.ARRAY_BUFFER, vertices_incircle, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjectionMatrix = mat4.create();
}

function distance(x1, y1, x2, y2)
{
    //code
    var result = ((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1));
    return (Math.sqrt(result));
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
    var mvpMatrix = mat4.create();

    //code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
       
        mat4.multiply(mvpMatrix, perspectiveProjectionMatrix, viewMatrix);
        mat4.multiply(mvpMatrix, mvpMatrix, modelMatrix);
        gl.uniformMatrix4fv(mvpUniform, false, mvpMatrix);
        
        gl.uniform3f(materialUniform, 0.0, 0.0, 1.0);
        gl.bindVertexArray(vao);
        gl.drawArrays(gl.LINES, 0, vertices_line_count / 3);
        gl.bindVertexArray(null);

        gl.uniform3f(materialUniform, 1.0, 0.0, 0.0);
        gl.bindVertexArray(vao_x);
        gl.drawArrays(gl.LINES, 0, 2);
        gl.bindVertexArray(null);

        gl.uniform3f(materialUniform, 0.0, 1.0, 0.0);
        gl.bindVertexArray(vao_y);
        gl.drawArrays(gl.LINES, 0, 2);
        gl.bindVertexArray(null);

        gl.uniform3f(materialUniform, 1.0, 1.0, 0.0);
        gl.bindVertexArray(vao_circle);
        gl.drawArrays(gl.LINE_LOOP, 0, vertices_circle_count / 3);
        gl.bindVertexArray(null);

        gl.bindVertexArray(vao_square);
        gl.drawArrays(gl.LINE_LOOP, 0, 4);
        gl.bindVertexArray(null);

        gl.bindVertexArray(vao_triangle);
        gl.drawArrays(gl.LINE_LOOP, 0, 3);
        gl.bindVertexArray(null);

        gl.bindVertexArray(vao_incircle);
        gl.drawArrays(gl.LINE_LOOP, 0, vertices_incircle_count / 3);
        gl.bindVertexArray(null);
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
    if(vao)
    {
        gl.deleteVertexArrays(vao);
        vao = 0;
    }

    //release vbo
    if(vbo_position)
    {
        gl.deleteBuffers(vbo_position);
        vbo_position = 0;
    }

    if(vao_x)
    {
        gl.deleteVertexArrays(vao_x);
        vao_x = 0;
    }

    //release vbo
    if(vbo_x_position)
    {
        gl.deleteBuffers(vbo_x_position);
        vbo_x_position = 0;
    }

    if(vao_y)
    {
        gl.deleteVertexArrays(vao_y);
        vao_y = 0;
    }

    //release vbo
    if(vbo_y_position)
    {
        gl.deleteBuffers(vbo_y_position);
        vbo_y_position = 0;
    }

    if(vao_square)
    {
        gl.deleteVertexArrays(vao_square);
        vao_square = 0;
    }

    //release vbo
    if(vbo_square_position)
    {
        gl.deleteBuffers(vbo_square_position);
        vbo_square_position = 0;
    }

    if(vao_triangle)
    {
        gl.deleteVertexArrays(vao_triangle);
        vao_triangle = 0;
    }

    //release vbo
    if(vbo_triangle_position)
    {
        gl.deleteBuffers(vbo_triangle_position);
        vbo_triangle_position = 0;
    }

    if(vao_incircle)
    {
        gl.deleteVertexArrays(vao_incircle);
        vao_incircle = 0;
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

        case 68:
            day = (day + 6) % 360;
            break;

        case 100:
            day = (day - 6) % 360;
            break;

        case 89:
            year = (year + 3) % 360;
            break;
        
        case 121:
            year = (year - 3) % 360;
            break;
        
        default:
            break;
    }
}

function mouseDown()
{
    //code
    
}
