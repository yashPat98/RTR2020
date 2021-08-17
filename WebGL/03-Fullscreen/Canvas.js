//global variables 
var canvas = null;
var bFullscreen = false;

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

    //register keyboard's keydown event handler
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseDown, false);
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

function keyDown(event)
{
    //code
    switch(event.keyCode)
    {  
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
