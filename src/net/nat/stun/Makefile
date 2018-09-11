TARGET = build_in.o
CCFLAGS += -c -g -O0 -Wno-return-type

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))  #子目录obj

$(TARGET) : $(OBJS) $(COMOBJS)
	$(LD) -r $^ -o $@ $(LDFLAGS) 
%.o : %.c
	$(CC) $< -fPIC -o  $@ $(CCFLAGS) $(INCLUDES) $(MACROS)
.PHONY:clean
clean:
	rm -rf *.o
	rm -rf $(TARGET) 
	rm -rf Makefile

