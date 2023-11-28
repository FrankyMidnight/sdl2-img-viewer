CC = gcc
CFLAGS = -Wall -std=c2x 
LDFLAGS = -lSDL2 -lSDL2_image
SRC = sdl2-image-viewer.c
TARGET= sdl2-img-viewer

build :
	${CC} ${CFLAGS} ${SRC} ${LDFLAGS} -g -o ${TARGET}

clean :
	rm img-viewer

release :
	${CC} ${CFLAGS} ${SRC} ${LDFLAGS} -O2 -o ${TARGET}