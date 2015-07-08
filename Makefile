.SUFFIXES:
.SUFFIXES: .cpp .o

CC = g++
CFLAGS = -Wall -g -I include `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`
LDFLAGS = -lm
OBJDIR = build

vpath %.cpp src
vpath %.hpp include

OBJ = ${addprefix ${OBJDIR}/, VideoCaptureReader.o VideoReader.o main.o}

all: ${OBJ} 
	${CC} ${CFLAGS} ${OBJ} ${LIBS} ${LDFLAGS} -o main
${OBJDIR}/%.o: %.cpp
	${CC} ${CFLAGS} -c ${LIBS} -o $@ $<
clean:
	rm -r ${OBJDIR}/*
