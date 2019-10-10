PACKAGE = torreadwrite
TARGETS = torread torwrite

CC = gcc
LD = $(CC)
CFLAGS = 
LDFLAGS = -s
SRCS = src
RM = rm -fv

.PHONY: all clean

all: $(TARGETS)

torread: $(SRCS)/torread.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

torwrite: $(SRCS)/torwrite.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(TARGETS)
