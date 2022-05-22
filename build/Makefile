CC = gcc

CFLAGS = -O2 -Wall -Wno-missing-braces -std=c99
CFLAGS += -lraylib -lglfw -lGL -lopenal -lm -pthread -ldl -lm -lX11 -fopenmp

cosmology: main.o funcs.o physics.o quadtree.o
	$(CC) main.o funcs.o physics.o quadtree.o -o cosmology $(CFLAGS)

main.o: ../src/main.c
	$(CC) -c ../src/main.c
funcs.o: ../src/funcs.c
	$(CC) -c ../src/funcs.c
physics.o: ../src/physics.c
	$(CC) -c ../src/physics.c
quadtree.o: ../src/quadtree.c
	$(CC) -c ../src/quadtree.c

clean:
	rm *.o cosmology