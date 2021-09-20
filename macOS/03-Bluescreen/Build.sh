mkdir -p Bluescreen.app/Contents/MacOS
clang -Wno-deprecated-declarations -o Bluescreen.app/Contents/MacOS/Bluescreen Bluescreen.m -framework Cocoa -framework QuartzCore -framework OpenGL
