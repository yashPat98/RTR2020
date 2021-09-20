mkdir -p OGLPP.app/Contents/MacOS
clang++ -c Sphere.mm
clang++ -Wno-deprecated-declarations -c OGLPP.mm
clang++ -o OGLPP.app/Contents/MacOS/OGLPP OGLPP.o Sphere.o -framework Cocoa -framework QuartzCore -framework OpenGL
