@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "DeathlyHallowsAnimation.cpp"
rc.exe "RESOURCES.rc"
link.exe "DeathlyHallowsAnimation.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "DeathlyHallowsAnimation.exe"

del "DeathlyHallowsAnimation.obj"
del "RESOURCES.res"
del "DeathlyHallowsAnimation.exe"

EXIT (0)
