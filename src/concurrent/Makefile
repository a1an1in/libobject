TARGET = build_in.o
#COMOBJS = ../common/proto.o   #公共模块
CC = gcc
LD = ld
CCFLAGS += -c 

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))  #子目录obj

$(TARGET) : $(OBJS) $(COMOBJS)
	$(LD) -r $^ -o $@ $(LIBS) $(LDFLAGS) 
%.o : %.c
	$(CC) $< -fPIC -o  $@ $(CCFLAGS) $(INCLUDES) $(MACROS)
.PHONY:clean
clean:
	rm -rf *.o
	rm -rf $(TARGET) 
	rm -rf Makefile

