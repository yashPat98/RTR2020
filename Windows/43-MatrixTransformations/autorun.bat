@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "MatrixTransformations.cpp"
rc.exe "RESOURCES.rc"
link.exe "MatrixTransformations.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "MatrixTransformations.exe"

del "MatrixTransformations.obj"
del "RESOURCES.res"
del "MatrixTransformations.exe"

EXIT (0)
