//global variables 
var canvas = null;

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

function keyDown(event)
{
    //code
    alert("key is pressed.");
}

function mouseDown()
{
    //code
    alert("mouse key is pressed.");
}
