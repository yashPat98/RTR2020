@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "UniformScalingObject.cpp"
rc.exe "RESOURCES.rc"
link.exe "UniformScalingObject.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "UniformScalingObject.exe"

del "UniformScalingObject.obj"
del "RESOURCES.res"
del "UniformScalingObject.exe"

EXIT (0)
