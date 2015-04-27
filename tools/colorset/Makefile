TARGET = colorset
INCLUDES       = -I ./include 
CFLAGS         = -Wall -D_GNU_SOURCE -DHAVE_USLEEP $(INCLUDES)

SRCS = colorset.c \
	   lib/procutils.c \
	   lib/at.c

OBJS = $(SRCS:.c=.o)

.PHONY: all

all: clean $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $? $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	$(RM) -f *.o $(TARGET) *~
