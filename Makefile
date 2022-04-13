all:
	gcc iv.c -o iv `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0`	
