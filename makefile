CC = gcc
AS = nasmw

INCLUDES = /usr/include/i386-linux-gnu/sys/

FLAGS = -O3 -march=pentium -fstrength-reduce -fomit-frame-pointer -I $(INCLUDES)

OBJS =	unzip.o inflate.o crc32.o rifunct.o
	
all: romident makedat riexport

%.o: %.c
	$(CC) $(FLAGS) -o $@ -c $<

romident: $(OBJS) romident.o
	$(CC) romident.o $(OBJS) -o romident -s

makedat: $(OBJS) makedat.o
	$(CC) makedat.o $(OBJS) -o makedat -s

riexport: $(OBJS) riexport.o
	$(CC) riexport.o $(OBJS) -o riexport -s

unzip.o: unzip.c unzip.h types.h

