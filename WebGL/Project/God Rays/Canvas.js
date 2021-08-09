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

var occluded_vertexShaderObject;
var occluded_fragmentShaderObject;
var occluded_shaderProgramObject;

var mvpMatrixUniform;
var colorUniform;

var godRays_vertexShaderObject;
var godRays_fragmentShaderObject;
var godRays_shaderProgramObject;

var sceneSamplerUniform;
var occludedSamplerUniform;
var lightPositionOnScreenUniform;
var godRays_mvpMatrixUniform;

var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var modelMatrixUniform;
var viewMatrixUniform;
var perspectiveProjectionMatrixUniform;
var LaUniform;
var LdUniform;
var LsUniform;
var lightPositionUniform;
var KaUniform;
var KdUniform;
var KsUniform;
var materialShininessUniform;

var fbo_occluded;
var occluded_depth_texture;
var occluded_color_texture;

var fbo_scene;
var scene_depth_texture;
var scene_color_texture;

var vao_quad;
var vbo_quad_vertices;
var vbo_quad_texcoords;

var vao_cube;
var vbo_cube_vertices;
var vbo_cube_normals;

var vao_pyramid;
var vbo_pyramid_vertices;
var vbo_pyramid_normals;

var perspectiveProjectionMatrix;
var orthographicProjectionMatrix;

var lightPosition;

var scr_width;
var scr_height;

var sphere;

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

    //get screen resolution
    scr_width = parseInt(window.screen.width * window.devicePixelRatio);
    scr_height = parseInt(window.screen.height * window.devicePixelRatio);

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

    //occluded shader program
    //vertex shader
    var occluded_vertexShaderSourceCode = 
    "#version 300 es"                               +
    "\n"                                            +
    "in vec4 vPosition;"                            +
    "uniform mat4 u_mvpMatrix;"                     +
    "void main(void)"                               +
    "{"                                             +
    "   gl_Position = u_mvpMatrix * vPosition;"     +
    "}";

    occluded_vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(occluded_vertexShaderObject, occluded_vertexShaderSourceCode);
    gl.compileShader(occluded_vertexShaderObject);
    if(gl.getShaderParameter(occluded_vertexShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(occluded_vertexShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //fragment shader 
    var occluded_fragmentShaderSourceCode = 
    "#version 300 es"                           +
    "\n"                                        +
    "precision highp float;"                    +
    "out vec4 FragColor;"                       +
    "uniform vec3 u_color;"                     +
    "void main(void)"                           +
    "{"                                         +
    "   FragColor = vec4(u_color, 1.0);"        +
    "}";

    occluded_fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(occluded_fragmentShaderObject, occluded_fragmentShaderSourceCode);
    gl.compileShader(occluded_fragmentShaderObject);
    if(gl.getShaderParameter(occluded_fragmentShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(occluded_fragmentShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //shader program
    occluded_shaderProgramObject = gl.createProgram();
    gl.attachShader(occluded_shaderProgramObject, occluded_vertexShaderObject);
    gl.attachShader(occluded_shaderProgramObject, occluded_fragmentShaderObject);

    //pre-linking binding of shader program object with vertex shader attributes
    gl.bindAttribLocation(occluded_shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_VERTEX, "vPosition");

    //linking 
    gl.linkProgram(occluded_shaderProgramObject);
    if(!gl.getProgramParameter(occluded_shaderProgramObject, gl.LINK_STATUS))
    {
        var error = gl.getProgramInfoLog(occluded_shaderProgramObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //get MVP uniform location
    mvpMatrixUniform = gl.getUniformLocation(occluded_shaderProgramObject, "u_mvpMatrix");
    colorUniform = gl.getUniformLocation(occluded_shaderProgramObject, "u_color");

    //god rays shader program
    //vertex shader
    var godRays_vertexShaderSourceCode = 
        "#version 300 es"           +
        "\n"                        +
        
        "in vec2 vPosition;"        +
        "in vec2 vTexCoord;"        +
        
        "uniform mat4 mvpMatrix;"   +
        
        "out vec2 out_texcoord;"    +

        "void main(void)"           +
        "{"                         +
        "   out_texcoord = vTexCoord;"  +
        "   gl_Position = mvpMatrix * vec4(vPosition.x, vPosition.y, 0.0f, 1.0f);"  +
        "}";

    godRays_vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(godRays_vertexShaderObject, godRays_vertexShaderSourceCode);
    gl.compileShader(godRays_vertexShaderObject);
    if(gl.getShaderParameter(godRays_vertexShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(godRays_vertexShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //fragment shader
    var godRays_fragmentShaderSourceCode = 
        "#version 300 es"                           +
        "\n"                                        +
        "precision highp float;"                    +

        "in vec2 out_texcoord;"                     +

        "uniform sampler2D sceneSampler;"           +
        "uniform sampler2D occludedSampler;"        +
        "uniform vec2 lightPositionOnScreen;"       +

        "out vec4 FragColor;"                       +
        
        "const float decay = 0.96815f;"             +
        "const float exposure = 0.3f;"              +
        "const float density = 0.926f;"             +
        "const float weight = 0.58767f;"            +
        
        "void main(void)"                                                                       +
        "{"                                                                                     +
        "   int NUM_SAMPLES = 120;"                                                             +

        "   vec2 tc = out_texcoord.xy;"                                                         +
        "   vec2 deltaTexCoord = (tc - lightPositionOnScreen);"                                 +
        "   deltaTexCoord *= 1.0f / float(NUM_SAMPLES) * density;"                              +
        "   float illuminationDecay = 1.0f;"                                                    +

        "   vec4 base_color = texture(sceneSampler, tc);"                                       +
        "   vec4 occluded_color = texture(occludedSampler, tc) * 0.4f;"                         +

        "   for(int i = 0; i < NUM_SAMPLES; i++)"                                               +
        "   {"                                                                                  +   
        "       tc -= deltaTexCoord;"                                                           +
        "       vec4 tex_sample = texture(occludedSampler, tc) * 0.4f;"                         +
        "       tex_sample *= illuminationDecay * weight;"                                      +
        "       occluded_color += tex_sample;"                                                  +
        "       illuminationDecay *= decay;"                                                    +
        "   }"                                                                                  +

        "   FragColor = vec4(vec3(occluded_color) * exposure, 1.0f) + (base_color * 1.1f);"     +
        "   }"; 
    
    godRays_fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(godRays_fragmentShaderObject, godRays_fragmentShaderSourceCode);
    gl.compileShader(godRays_fragmentShaderObject);
    if(gl.getShaderParameter(godRays_fragmentShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(godRays_fragmentShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //shader program
    godRays_shaderProgramObject = gl.createProgram();
    gl.attachShader(godRays_shaderProgramObject, godRays_vertexShaderObject);
    gl.attachShader(godRays_shaderProgramObject, godRays_fragmentShaderObject);

    //pre-linking binding of shader program object with vertex shader attributes
    gl.bindAttribLocation(godRays_shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_VERTEX, "vPosition");
    gl.bindAttribLocation(godRays_shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_TEXCOORD, "vTexCoord");

    //linking 
    gl.linkProgram(godRays_shaderProgramObject);
    if(!gl.getProgramParameter(godRays_shaderProgramObject, gl.LINK_STATUS))
    {
        var error = gl.getProgramInfoLog(godRays_shaderProgramObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }

    //get MVP uniform location
    sceneSamplerUniform = gl.getUniformLocation(godRays_shaderProgramObject, "sceneSampler");
    occludedSamplerUniform = gl.getUniformLocation(godRays_shaderProgramObject, "occludedSampler");
    lightPositionOnScreenUniform = gl.getUniformLocation(godRays_shaderProgramObject, "lightPositionOnScreen");
    godRays_mvpMatrixUniform = gl.getUniformLocation(godRays_shaderProgramObject, "mvpMatrix");

    //per-fragment lighting
    //vertex shader
    var vertexShaderSourceCode = 
        "#version 300 es"                                                                               +
        "\n"                                                                                            +
        
        "in vec4 vPosition;"                                                                            +
        "in vec3 vNormal;"                                                                              +

        "uniform mat4 u_modelMatrix;"                                                                   +
        "uniform mat4 u_viewMatrix;"                                                                    +
        "uniform mat4 u_perspectiveProjectionMatrix;"                                                   +
        "uniform vec4 u_lightPosition;"                                                                 +

        "out vec3 transformed_normal;"                                                                  +
        "out vec3 light_direction;"                                                                     +
        "out vec3 view_vector;"                                                                         +

        "void main(void)"                                                                               +
        "{"                                                                                             +
        "   vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                                +
        "   mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix)));"               +
        "   transformed_normal = normal_matrix * vNormal;"                                              +
        "   light_direction = vec3(u_lightPosition - eye_coords);"                                      +
        "   view_vector = -eye_coords.xyz;"                                                             +
        "   gl_Position = u_perspectiveProjectionMatrix * u_viewMatrix * u_modelMatrix * vPosition;"    +
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
        "in vec3 transformed_normal;"               +
        "in vec3 light_direction;"                  +
        "in vec3 view_vector;"                      +

        "uniform vec3 u_La;"                        +
        "uniform vec3 u_Ld;"                        +
        "uniform vec3 u_Ls;"                        +
        "uniform vec3 u_Ka;"                        +
        "uniform vec3 u_Kd;"                        +
        "uniform vec3 u_Ks;"                        +
        "uniform float u_materialShininess;"        +

        "out vec4 fragColor;"                       +

        "void main(void)"                           +
        "{"                                         +
        "   vec3 phong_ads_light;"                  +

        "   vec3 normalized_transformed_normal = normalize(transformed_normal);"    +
        "   vec3 normalized_view_vector = normalize(view_vector);"                  +
        "   vec3 normalized_light_direction = normalize(light_direction);"          +
        "   vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normal);"  +
        
        "   vec3 ambient = u_La * u_Ka;"                                                                    +
        "   vec3 diffuse = u_Ld * u_Kd * max(dot(normalized_light_direction, normalized_transformed_normal), 0.0f);"    +
        "   vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_view_vector), 0.0f), u_materialShininess);"     +
        "   phong_ads_light = ambient + diffuse + specular;"                                                                        +

        "   fragColor = vec4(phong_ads_light, 1.0f);"                                                                               +
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
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

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
    modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_modelMatrix");
    viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_viewMatrix");
    perspectiveProjectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_perspectiveProjectionMatrix");

    LaUniform = gl.getUniformLocation(shaderProgramObject, "u_La");
    LdUniform = gl.getUniformLocation(shaderProgramObject, "u_Ld");
    LsUniform = gl.getUniformLocation(shaderProgramObject, "u_Ls");
    lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_lightPosition");

    KaUniform = gl.getUniformLocation(shaderProgramObject, "u_Ka");
    KdUniform = gl.getUniformLocation(shaderProgramObject, "u_Kd");
    KsUniform = gl.getUniformLocation(shaderProgramObject, "u_Ks");
    materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_materialShininess");

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
                                        -1.0, 1.0, 1.0, 
                                        -1.0, 1.0, -1.0, 
                                        -1.0, -1.0, -1.0,
                                        -1.0, -1.0, 1.0, 

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

    var cubeNormals = new Float32Array([
                                        //near 
                                        0.0, 0.0, 1.0,
                                        0.0, 0.0, 1.0,
                                        0.0, 0.0, 1.0,
                                        0.0, 0.0, 1.0,

                                        //right
                                        1.0, 0.0, 0.0,
                                        1.0, 0.0, 0.0,
                                        1.0, 0.0, 0.0,
                                        1.0, 0.0, 0.0,

                                        //far
                                        0.0, 0.0, -1.0,
                                        0.0, 0.0, -1.0,
                                        0.0, 0.0, -1.0,
                                        0.0, 0.0, -1.0,

                                        //left
                                        -1.0, 0.0, 0.0,
                                        -1.0, 0.0, 0.0,
                                        -1.0, 0.0, 0.0,
                                        -1.0, 0.0, 0.0,

                                        //top
                                        0.0, 1.0, 0.0,
                                        0.0, 1.0, 0.0,
                                        0.0, 1.0, 0.0,
                                        0.0, 1.0, 0.0,

                                        //bottom
                                        0.0, -1.0, 0.0,
                                        0.0, -1.0, 0.0,
                                        0.0, -1.0, 0.0,
                                        0.0, -1.0, 0.0,
                                    ]);

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

    var pyramidNormals = new Float32Array([
                                            //near 
                                            0.0, 0.447214, 0.894427,
                                            0.0, 0.447214, 0.894427,
                                            0.0, 0.447214, 0.894427,
                
                                            //right
                                            0.894427, 0.447214, 0.0,
                                            0.894427, 0.447214, 0.0,
                                            0.894427, 0.447214, 0.0,
        
                                            //far
                                            0.0, 0.447214, -0.894427,
                                            0.0, 0.447214, -0.894427,
                                            0.0, 0.447214, -0.894427,
        
                                            //left
                                            -0.894427, 0.447214, 0.0,
                                            -0.894427, 0.447214, 0.0,
                                            -0.894427, 0.447214, 0.0
                                        ]);

    var quadVertices = new Float32Array([
                                            parseFloat(scr_width), parseFloat(scr_height),
                                            0.0, parseFloat(scr_height), 
                                            0.0, 0.0,
                                            parseFloat(scr_width), 0.0,
                                        ]);

    var quadTexCoords = new Float32Array([
                                            1.0, 1.0, 
                                            0.0, 1.0,
                                            0.0, 0.0, 
                                            1.0, 0.0,
                                        ]);

    //initialize sphere data
    sphere = new Mesh();
    makeSphere(sphere, 0.5, 20, 20);

    //setup vao and vbo
    //screen quad
    vao_quad= gl.createVertexArray();
    gl.bindVertexArray(vao_quad);
        vbo_quad_vertices = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_quad_vertices);
            gl.bufferData(gl.ARRAY_BUFFER, quadVertices, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 2, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        vbo_quad_texcoords = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_quad_texcoords);
            gl.bufferData(gl.ARRAY_BUFFER, quadTexCoords, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD, 2, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD);
        gl.bindBuffer(gl.ARRAY_BUFFER, null); 
    gl.bindVertexArray(null);

    //cube
    vao_cube= gl.createVertexArray();
    gl.bindVertexArray(vao_cube);
        vbo_cube_vertices = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_cube_vertices);
            gl.bufferData(gl.ARRAY_BUFFER, cubeVertices, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        vbo_cube_normals = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_cube_normals);
            gl.bufferData(gl.ARRAY_BUFFER, cubeNormals, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_NORMAL);
        gl.bindBuffer(gl.ARRAY_BUFFER, null); 
    gl.bindVertexArray(null);

    //pyramid
    vao_pyramid= gl.createVertexArray();
    gl.bindVertexArray(vao_pyramid);
        vbo_pyramid_vertices = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid_vertices);
            gl.bufferData(gl.ARRAY_BUFFER, pyramidVertices, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        vbo_pyramid_normals = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid_normals);
            gl.bufferData(gl.ARRAY_BUFFER, pyramidNormals, gl.STATIC_DRAW);
            gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_NORMAL);
        gl.bindBuffer(gl.ARRAY_BUFFER, null); 
    gl.bindVertexArray(null);

    //framebuffer configurations for occluded scene
    fbo_occluded = gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo_occluded);
        //create a color attachment texture
        occluded_color_texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, occluded_color_texture);
        
        gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texStorage2D(gl.TEXTURE_2D, 1, gl.RGBA8, scr_width, scr_height);

        //create depth attachment texture
        occluded_depth_texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, occluded_depth_texture);
        gl.texStorage2D(gl.TEXTURE_2D, 1, gl.DEPTH_COMPONENT32F, scr_width, scr_height);

        //attach
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, occluded_color_texture, 0);                                
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, occluded_depth_texture, 0);
        
        gl.drawBuffers([gl.COLOR_ATTACHMENT0]);

        gl.bindTexture(gl.TEXTURE_2D, null);
        gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    //framebuffer configurations for base scene
    fbo_scene = gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo_scene);
        //create a color attachment texture
        scene_color_texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, scene_color_texture);
        
        gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texStorage2D(gl.TEXTURE_2D, 1, gl.RGBA8, scr_width, scr_height);

        //create depth attachment texture
        scene_depth_texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, scene_depth_texture);
        gl.texStorage2D(gl.TEXTURE_2D, 1, gl.DEPTH_COMPONENT32F, scr_width, scr_height);

        //attach
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, scene_color_texture, 0);                                
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, scene_depth_texture, 0);
        
        gl.drawBuffers([gl.COLOR_ATTACHMENT0]);
        
        gl.bindTexture(gl.TEXTURE_2D, null);
        gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjectionMatrix = mat4.create();
    orthographicProjectionMatrix = mat4.create();

    lightPosition = new Float32Array([3.0, 2.0, -8.0, 1.0]);
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
    mat4.ortho(orthographicProjectionMatrix, 0.0, parseFloat(canvas.width), 0.0, parseFloat(canvas.height), 1.0, -1.0);   
}

function render()
{
    //variable declarations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();
    var lightPositionOnScreen = new Float32Array(2);
    var projected = new Float32Array(4);

    //code
    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo_occluded);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
        gl.useProgram(occluded_shaderProgramObject);

        //white sphere
        gl.uniform3f(colorUniform, 0.984, 0.5859, 0.004);

        mat4.translate(modelViewProjectionMatrix, perspectiveProjectionMatrix, [lightPosition[0], lightPosition[1], lightPosition[2]]);
        gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);

        sphere.draw();
        
        //black occluding scene
        gl.uniform3f(colorUniform, 0.09, 0.058, 0.0);

        //cube
        mat4.translate(modelViewProjectionMatrix, perspectiveProjectionMatrix, [0.0, 0.0, -10.0]);
        gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);

        gl.bindVertexArray(vao_cube);
        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);

        //pyramid
        mat4.translate(modelViewProjectionMatrix, perspectiveProjectionMatrix, [3.0, 0.0, -10.0]);
        gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);

        gl.bindVertexArray(vao_pyramid);
        gl.drawArrays(gl.TRIANGLES, 0, 12);

        gl.bindVertexArray(null);
        gl.useProgram(null);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    
    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo_scene);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

        //per-fragment lighting shader
        gl.useProgram(shaderProgramObject);

        gl.uniform3f(LaUniform, 0.0, 0.0, 0.0);
        gl.uniform3f(LdUniform, 1.0, 1.0, 1.0);
        gl.uniform3f(LsUniform, 1.0, 1.0, 1.0);
        gl.uniform4f(lightPositionUniform, lightPosition[0], lightPosition[1], lightPosition[2], lightPosition[3]);

        gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
        gl.uniformMatrix4fv(perspectiveProjectionMatrixUniform, false, perspectiveProjectionMatrix);

        //cube
        modelMatrix = mat4.create();
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -10.0]);

        gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);

        gl.uniform3f(KaUniform, 0.24725, 0.1995, 0.0745);
        gl.uniform3f(KdUniform, 0.75164, 0.6064, 0.22648);
        gl.uniform3f(KsUniform, 0.628281, 0.555802, 0.366065);
        gl.uniform1f(materialShininessUniform, 0.4 * 128.0);

        gl.bindVertexArray(vao_cube);
        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);

        //pyramid
        modelMatrix = mat4.create();
        mat4.translate(modelMatrix, modelMatrix, [3.0, 0.0, -10.0]);

        gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);

        gl.uniform3f(KaUniform, 0.0, 0.0, 0.0);
        gl.uniform3f(KdUniform, 0.5, 0.0, 0.0);
        gl.uniform3f(KsUniform, 0.7, 0.6, 0.6);
        gl.uniform1f(materialShininessUniform, 0.25 * 128.0);

        gl.bindVertexArray(vao_pyramid);
        gl.drawArrays(gl.TRIANGLES, 0, 12);

        gl.bindVertexArray(null);
        gl.useProgram(null);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(godRays_shaderProgramObject);

    mat4.multiply(projected, perspectiveProjectionMatrix, lightPosition);
    projected[0] /= projected[3];
    projected[1] /= projected[3];

    lightPositionOnScreen[0] = (projected[0] + 1.0) / 2.0;
    lightPositionOnScreen[1] = (projected[1] + 1.0) / 2.0;

    gl.uniformMatrix4fv(godRays_mvpMatrixUniform, false, orthographicProjectionMatrix);
    gl.uniform2f(lightPositionOnScreenUniform, lightPositionOnScreen[0], lightPositionOnScreen[1]);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, scene_color_texture);
    gl.uniform1i(sceneSamplerUniform, 0);

    gl.activeTexture(gl.TEXTURE1);
    gl.bindTexture(gl.TEXTURE_2D, occluded_color_texture);
    gl.uniform1i(occludedSamplerUniform, 1);

    gl.bindVertexArray(vao_quad);
    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);

    gl.bindVertexArray(null);
    gl.useProgram(null);

    //animation loop
    requestAnimationFrame(render, canvas);
}

function uninitialize()
{
    //release textures
    if(occluded_color_texture)
    {
        gl.deleteTexture(occluded_texture);
        occluded_texture = 0;
    }

    if(occluded_depth_texture)
    {
        gl.deleteTexture(occluded_depth_texture);
        occluded_depth_texture = 0;
    }

    if(scene_color_texture)
    {
        gl.deleteTexture(occluded_texture);
        occluded_texture = 0;
    }

    if(scene_depth_texture)
    {
        gl.deleteTexture(scene_depth_texture);
        scene_depth_texture = 0;
    }

    //release fbo 
    if(fbo_occluded)
    {
        gl.deleteFramebuffer(fbo_occluded);
        fbo_occluded = 0;
    }

    if(fbo_scene)
    {
        gl.deleteFramebuffer(fbo_scene);
        fbo_scene = 0;
    }

    //quad
    if(vao_quad)
    {
        gl.deleteVertexArray(vao_quad);
        vao_quad = null;
    }

    if(vbo_quad_vertices)
    {
        gl.deleteBuffer(vbo_quad_vertices);
        vbo_quad_vertices = null;
    }

    if(vbo_quad_texcoords)
    {
        gl.deleteBuffer(vbo_quad_texcoords);
        vbo_quad_texcoords = null;
    }

    //sphere
    if(vao_sphere)
    {
        gl.deleteVertexArray(vao_sphere);
        vao_sphere = null;
    }

    if(vbo_sphere_vertices)
    {
        gl.deleteBuffer(vbo_sphere_vertices);
        vbo_sphere_vertices = null;
    }

    if(vbo_sphere_indices)
    {
        gl.deleteBuffer(vbo_sphere_indices);
        vbo_sphere_indices = null;
    }

    //cube
    if(vao_cube)
    {
        gl.deleteVertexArray(vao_cube);
        vao_cube = null;
    }

    if(vbo_cube_vertices)
    {
        gl.deleteBuffer(vbo_cube_vertices);
        vbo_cube_vertices = null;
    }

    if(vbo_cube_normals)
    {
        gl.deleteBuffer(vbo_cube_normals);
        vbo_cube_normals = null;
    }

    //pyramid
    if(vao_pyramid)
    {
        gl.deleteVertexArray(vao_pyramid);
        vao_pyramid = null;
    }

    if(vbo_pyramid_vertices)
    {
        gl.deleteBuffer(vbo_pyramid_vertices);
        vbo_pyramid_vertices = null;
    }

    if(vbo_pyramid_normals)
    {
        gl.deleteBuffer(vbo_pyramid_normals);
        vbo_pyramid_normals = null;
    }

    //release godRays shader
    if(godRays_shaderProgramObject)
    {
        if(godRays_fragmentShaderObject)
        {
            gl.detachShader(godRays_shaderProgramObject, godRays_fragmentShaderObject);
            gl.deleteShader(godRays_fragmentShaderObject);
            godRays_fragmentShaderObject = null;
        }

        if(godRays_vertexShaderObject)
        {
            gl.detachShader(godRays_shaderProgramObject, godRays_vertexShaderObject);
            gl.deleteShader(godRays_vertexShaderObject);
            godRays_vertexShaderObject = null;
        }

        gl.deleteProgram(godRays_shaderProgramObject);
        godRays_shaderProgramObject = null;
    }

    //release occluded shader
    if(occluded_shaderProgramObject)
    {
        if(occluded_fragmentShaderObject)
        {
            gl.detachShader(occluded_shaderProgramObject, occluded_fragmentShaderObject);
            gl.deleteShader(occluded_fragmentShaderObject);
            occluded_fragmentShaderObject = null;
        }

        if(occluded_vertexShaderObject)
        {
            gl.detachShader(occluded_shaderProgramObject, occluded_vertexShaderObject);
            gl.deleteShader(occluded_vertexShaderObject);
            occluded_vertexShaderObject = null;
        }

        gl.deleteProgram(occluded_shaderProgramObject);
        occluded_shaderProgramObject = null;
    }

    //release per-fragment lighting shader
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

        //left 
        case 37:
            lightPosition[0] -= 0.1;
            break;

        //up
        case 38:
            lightPosition[1] += 0.1;
            break;
        
        //right
        case 39:
            lightPosition[0] += 0.1;
            break;

        //down
        case 40:
            lightPosition[1] -= 0.1;
            break;

        //add
        case 107:
            lightPosition[2] += 0.1;
            break;
        
        //subtract
        case 109:
            lightPosition[2] -= 0.1;
            break;
    }
}

function mouseDown()
{
    //code
    
}
