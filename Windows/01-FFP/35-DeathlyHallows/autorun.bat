@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "DeathlyHallows.cpp"
rc.exe "RESOURCES.rc"
link.exe "DeathlyHallows.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "DeathlyHallows.exe"

del "DeathlyHallows.obj"
del "RESOURCES.res"
del "DeathlyHallows.exe"

EXIT (0)
