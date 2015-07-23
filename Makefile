.SUFFIXES:
.SUFFIXES: .cpp .o

CC = g++
CFLAGS2 = -Wall -g -I include
CFLAGS = -Wall -g -I include `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`
LDFLAGS = -lm
OBJDIR = build

vpath %.cpp src
vpath %.hpp include

OBJ = ${addprefix ${OBJDIR}/, Metric.o VQM.o PSNR.o SSIM.o VideoReader.o Y4MReader.o VideoCaptureReader.o main.o}

all: ${OBJ} 
	${CC} ${CFLAGS} ${OBJ} ${LIBS} ${LDFLAGS} -o vqtool
${OBJDIR}/%.o: %.cpp
	${CC} ${CFLAGS2} -c -o $@ $<
clean:
	rm -r ${OBJDIR}/*
