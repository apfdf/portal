all:
	g++ main.cpp glad.c -o portal -I include -L lib -lSDL3 -lopengl32