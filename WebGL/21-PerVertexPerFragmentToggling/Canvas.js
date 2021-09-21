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

var pv_vertexShaderObject;
var pv_fragmentShaderObject;
var pv_shaderProgramObject;

var pv_modelMatrixUniform;
var pv_viewMatrixUniform;
var pv_perspectiveProjectionMatrixUniform;
var pv_LaUniform;
var pv_LdUniform;
var pv_LsUniform;
var pv_lightPositionUniform;
var pv_KaUniform;
var pv_KdUniform;
var pv_KsUniform;
var pv_materialShininessUniform;
var pv_LKeyPressedUniform;

var pf_vertexShaderObject;
var pf_fragmentShaderObject;
var pf_shaderProgramObject;

var pf_modelMatrixUniform;
var pf_viewMatrixUniform;
var pf_perspectiveProjectionMatrixUniform;
var pf_LaUniform;
var pf_LdUniform;
var pf_LsUniform;
var pf_lightPositionUniform;
var pf_KaUniform;
var pf_KdUniform;
var pf_KsUniform;
var pf_materialShininessUniform;
var pf_LKeyPressedUniform;

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

var pv_pf_toggle = 0;
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

    //per vertex lighting

    //vertex shader
    var pv_vertexShaderSourceCode = 
        "#version 300 es"                                   +
        "\n"                                                +
        
        "precision mediump float;"                          +

        "in vec4 vPosition;"                                +
        "in vec3 vNormal;"                                  +
        
        "uniform mat4 u_modelMatrix;"                       +
        "uniform mat4 u_viewMatrix;"                        +
        "uniform mat4 u_projectionMatrix;"                  +   
        "uniform vec3 u_lightAmbient;"                      +
        "uniform vec3 u_lightDiffuse;"                      +
        "uniform vec3 u_lightSpecular;"                     +
        "uniform vec4 u_lightPosition;"                     +
        "uniform vec3 u_materialAmbient;"                   +
        "uniform vec3 u_materialDiffuse;"                   +
        "uniform vec3 u_materialSpecular;"                  +   
        "uniform float u_materialShininess;"                +
        "uniform int u_LKeyPressed;"                        +

        "out vec3 phong_ads_light;"                         +

        "void main(void)"                                                                                                                               +
        "{"                                                                                                                                             +
        "   if(u_LKeyPressed == 1)"                                                                                                                       +
        "   {"                                                                                                                                          +
        "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                                                                            +
        "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix)));"                                                           +
        "       vec3 transformed_normal = normalize(normal_matrix * vNormal);"                                                                          +
        "       vec3 light_direction = normalize(vec3(u_lightPosition - eye_coords));"                                                                  +
        "       vec3 reflection_vector = reflect(-light_direction, transformed_normal);"                                                                +
        "       vec3 view_vector = normalize(-eye_coords.xyz);"                                                                                         +

        "       vec3 ambient = u_lightAmbient * u_materialAmbient;"                                                                                     +
        "       vec3 diffuse = u_lightDiffuse * u_materialDiffuse * max(dot(light_direction, transformed_normal), 0.0f);"                               +
        "       vec3 specular = u_lightSpecular * u_materialSpecular * pow(max(dot(reflection_vector, view_vector), 0.0f), u_materialShininess);"       +

        "       phong_ads_light = ambient + diffuse + specular;"                                                                                        +
        "   }"                                                                                                                                          +
        "   else"                                                                                                                                       +
        "   {"                                                                                                                                          +
        "       phong_ads_light = vec3(1.0f, 1.0f, 1.0f);"                                                                                              +
        "   }"                                                                                                                                          +

        "   gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * vPosition;"                                                               +
        "}";

    pv_vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(pv_vertexShaderObject, pv_vertexShaderSourceCode);
    gl.compileShader(pv_vertexShaderObject);
    if(gl.getShaderParameter(pv_vertexShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(pv_vertexShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //fragment shader 
    var pv_fragmentShaderSourceCode = 
        "#version 300 es"                                   +
        "\n"                                                +
        "precision highp float;"                            +

        "in vec3 phong_ads_light;"                          +
        "out vec4 FragColor;"                               +

        "void main(void)"                                   +
        "{"                                                 +
        "   FragColor = vec4(phong_ads_light, 1.0f);"       +
        "}";

    pv_fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(pv_fragmentShaderObject, pv_fragmentShaderSourceCode);
    gl.compileShader(pv_fragmentShaderObject);
    if(gl.getShaderParameter(pv_fragmentShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(pv_fragmentShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //shader program
    pv_shaderProgramObject = gl.createProgram();
    gl.attachShader(pv_shaderProgramObject, pv_vertexShaderObject);
    gl.attachShader(pv_shaderProgramObject, pv_fragmentShaderObject);

    //pre-linking binding of shader program object with vertex shader attributes
    gl.bindAttribLocation(pv_shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_VERTEX, "vPosition");
    gl.bindAttribLocation(pv_shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

    //linking 
    gl.linkProgram(pv_shaderProgramObject);
    if(!gl.getProgramParameter(pv_shaderProgramObject, gl.LINK_STATUS))
    {
        var error = gl.getProgramInfoLog(pv_shaderProgramObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //get MVP uniform location
    pv_modelMatrixUniform = gl.getUniformLocation(pv_shaderProgramObject, "u_modelMatrix");
    pv_viewMatrixUniform = gl.getUniformLocation(pv_shaderProgramObject, "u_viewMatrix");
    pv_perspectiveProjectionMatrixUniform = gl.getUniformLocation(pv_shaderProgramObject, "u_projectionMatrix");

    pv_LaUniform = gl.getUniformLocation(pv_shaderProgramObject, "u_lightAmbient");
    pv_LdUniform = gl.getUniformLocation(pv_shaderProgramObject, "u_lightDiffuse");
    pv_LsUniform = gl.getUniformLocation(pv_shaderProgramObject, "u_lightSpecular");
    pv_lightPositionUniform = gl.getUniformLocation(pv_shaderProgramObject, "u_lightPosition");

    pv_KaUniform = gl.getUniformLocation(pv_shaderProgramObject, "u_materialAmbient");
    pv_KdUniform = gl.getUniformLocation(pv_shaderProgramObject, "u_materialDiffuse");
    pv_KsUniform = gl.getUniformLocation(pv_shaderProgramObject, "u_materialSpecular");
    pv_materialShininessUniform = gl.getUniformLocation(pv_shaderProgramObject, "u_materialShininess");

    pv_LKeyPressedUniform = gl.getUniformLocation(pv_shaderProgramObject, "u_LKeyPressed");

    //per fragment lighting

    //vertex shader
    var pf_vertexShaderSourceCode = 
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

    pf_vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(pf_vertexShaderObject, pf_vertexShaderSourceCode);
    gl.compileShader(pf_vertexShaderObject);
    if(gl.getShaderParameter(pf_vertexShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(pf_vertexShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //fragment shader 
    var pf_fragmentShaderSourceCode = 
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

    pf_fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(pf_fragmentShaderObject, pf_fragmentShaderSourceCode);
    gl.compileShader(pf_fragmentShaderObject);
    if(gl.getShaderParameter(pf_fragmentShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(pf_fragmentShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //shader program
    pf_shaderProgramObject = gl.createProgram();
    gl.attachShader(pf_shaderProgramObject, pf_vertexShaderObject);
    gl.attachShader(pf_shaderProgramObject, pf_fragmentShaderObject);

    //pre-linking binding of shader program object with vertex shader attributes
    gl.bindAttribLocation(pf_shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_VERTEX, "vPosition");
    gl.bindAttribLocation(pf_shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

    //linking 
    gl.linkProgram(pf_shaderProgramObject);
    if(!gl.getProgramParameter(pf_shaderProgramObject, gl.LINK_STATUS))
    {
        var error = gl.getProgramInfoLog(pf_shaderProgramObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //get MVP uniform location
    pf_modelMatrixUniform = gl.getUniformLocation(pf_shaderProgramObject, "u_modelMatrix");
    pf_viewMatrixUniform = gl.getUniformLocation(pf_shaderProgramObject, "u_viewMatrix");
    pf_perspectiveProjectionMatrixUniform = gl.getUniformLocation(pf_shaderProgramObject, "u_projectionMatrix");

    pf_LaUniform = gl.getUniformLocation(pf_shaderProgramObject, "u_lightAmbient");
    pf_LdUniform = gl.getUniformLocation(pf_shaderProgramObject, "u_lightDiffuse");
    pf_LsUniform = gl.getUniformLocation(pf_shaderProgramObject, "u_lightSpecular");
    pf_lightPositionUniform = gl.getUniformLocation(pf_shaderProgramObject, "u_lightPosition");

    pf_KaUniform = gl.getUniformLocation(pf_shaderProgramObject, "u_materialAmbient");
    pf_KdUniform = gl.getUniformLocation(pf_shaderProgramObject, "u_materialDiffuse");
    pf_KsUniform = gl.getUniformLocation(pf_shaderProgramObject, "u_materialSpecular");
    pf_materialShininessUniform = gl.getUniformLocation(pf_shaderProgramObject, "u_materialShininess");

    pf_LKeyPressedUniform = gl.getUniformLocation(pf_shaderProgramObject, "u_LKeyPressed");

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

    if(pv_pf_toggle == 0)
    {
        gl.useProgram(pv_shaderProgramObject);
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -5.0]);

        gl.uniformMatrix4fv(pv_modelMatrixUniform, false, modelMatrix);
        gl.uniformMatrix4fv(pv_viewMatrixUniform, false, viewMaterix);
        gl.uniformMatrix4fv(pv_perspectiveProjectionMatrixUniform, false, perspectiveProjectionMatrix);

        gl.uniform3fv(pv_LaUniform, lightAmbient);
        gl.uniform3fv(pv_LdUniform, lightDiffuse);
        gl.uniform3fv(pv_LsUniform, lightSpecular);
        gl.uniform4fv(pv_lightPositionUniform, lightPosition);

        gl.uniform3fv(pv_KaUniform, materialAmbient);
        gl.uniform3fv(pv_KdUniform, materialDiffuse);
        gl.uniform3fv(pv_KsUniform, materialSpecular);
        gl.uniform1f(pv_materialShininessUniform, materialShininess);

        gl.uniform1i(pv_LKeyPressedUniform, LKeyPressed);
    }
    else
    {
        gl.useProgram(pf_shaderProgramObject);
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -5.0]);

        gl.uniformMatrix4fv(pf_modelMatrixUniform, false, modelMatrix);
        gl.uniformMatrix4fv(pf_viewMatrixUniform, false, viewMaterix);
        gl.uniformMatrix4fv(pf_perspectiveProjectionMatrixUniform, false, perspectiveProjectionMatrix);

        gl.uniform3fv(pf_LaUniform, lightAmbient);
        gl.uniform3fv(pf_LdUniform, lightDiffuse);
        gl.uniform3fv(pf_LsUniform, lightSpecular);
        gl.uniform4fv(pf_lightPositionUniform, lightPosition);

        gl.uniform3fv(pf_KaUniform, materialAmbient);
        gl.uniform3fv(pf_KdUniform, materialDiffuse);
        gl.uniform3fv(pf_KsUniform, materialSpecular);
        gl.uniform1f(pf_materialShininessUniform, materialShininess);

        gl.uniform1i(pf_LKeyPressedUniform, LKeyPressed);
    }

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
    
    if(pv_shaderProgramObject)
    {
        if(pv_fragmentShaderObject)
        {
            gl.detachShader(pv_shaderProgramObject, pv_fragmentShaderObject);
            gl.deleteShader(pv_fragmentShaderObject);
            pv_fragmentShaderObject = null;
        }

        if(pv_vertexShaderObject)
        {
            gl.detachShader(pv_shaderProgramObject, pv_vertexShaderObject);
            gl.deleteShader(pv_vertexShaderObject);
            pv_vertexShaderObject = null;
        }

        gl.deleteProgram(pv_shaderProgramObject);
        pv_shaderProgramObject = null;
    }

    if(pf_shaderProgramObject)
    {
        if(pf_fragmentShaderObject)
        {
            gl.detachShader(pf_shaderProgramObject, pf_fragmentShaderObject);
            gl.deleteShader(pf_fragmentShaderObject);
            pf_fragmentShaderObject = null;
        }

        if(pf_vertexShaderObject)
        {
            gl.detachShader(pf_shaderProgramObject, pf_vertexShaderObject);
            gl.deleteShader(pf_vertexShaderObject);
            pf_vertexShaderObject = null;
        }

        gl.deleteProgram(pf_shaderProgramObject);
        pf_shaderProgramObject = null;
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
        
        //'B' or 'b'
        case 65:
            toggleFullscreen();
            break;

        //'F' or 'f'
        case 70:
            pv_pf_toggle = 1;
            break;

        case 86:
            pv_pf_toggle = 0;
            break;

        //'L' or 'l'
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
