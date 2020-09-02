@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Discipline_02.cpp"
rc.exe "RESOURCES.rc"
link.exe "Discipline_02.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "Discipline_02.exe"

del "Discipline_02.obj"
del "RESOURCES.res"
del "Discipline_02.exe"

EXIT (0)
