mkdir -p Ortho.app/Contents/MacOS
clang++ -Wno-deprecated-declarations -o Ortho.app/Contents/MacOS/Ortho Ortho.mm -framework Cocoa -framework QuartzCore -framework OpenGL
