all:
	gcc -Iinclude/ -Llibs/ src/app.c src/camera.c src/main.c src/scene.c src/texture.c src/utils.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lobj -lopengl32 -lm -o surface.exe -Wall -Wextra -Wpedantic

linux:
	gcc -Iinclude/ -Llibs/ src/app.c src/camera.c src/main.c src/scene.c src/texture.c src/utils.c -lobj -lSDL2 -lSDL2_image -lGL -lm -o surface -Wall -Wextra -Wpedantic
