@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "VariadicMsgBox.cpp"
link.exe "VariadicMsgBox.obj" "user32.lib" "gdi32.lib"

call "VariadicMsgBox.exe"

del "VariadicMsgBox.obj"
del "VariadicMsgBox.exe"

EXIT (0)
