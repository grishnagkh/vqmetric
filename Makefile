.SUFFIXES:
.SUFFIXES: .cpp .o

CC = g++
CFLAGS2 = -Wall -g -I include
CFLAGS = -Wall -g -I include `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`
LDFLAGS = -lm
OBJDIR = build
EXEC_NAME=vqtool
RM = rm -rf
MKDIR_P=mkdir -p

vpath %.cpp src
vpath %.hpp include

OBJ = ${addprefix ${OBJDIR}/, Metric.o VQM.o PSNR.o SSIM.o VideoReader.o Y4MReader.o VideoCaptureReader.o main.o}

all: prepare build-all;

prepare:
	mkdir -p ${OBJDIR}

build-all: ${OBJ} 
	${CC} ${CFLAGS} ${OBJ} ${LIBS} ${LDFLAGS} -o ${EXEC_NAME}
${OBJDIR}/%.o: %.cpp
	${CC} ${CFLAGS2} -c -o $@ $<
clean:
	${RM} ${EXEC_NAME} 
	${RM} ${OBJDIR}
install:
	cp ${EXEC_NAME} /usr/bin
