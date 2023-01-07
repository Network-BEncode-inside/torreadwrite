PACKAGE = torreadwrite
TARGETS = torread torwrite

CC = gcc
CFLAGS = -Wall
LDFLAGS = -s
SRCS = src
RM = rm -f

.PHONY: all clean

all: $(TARGETS)

torread: $(SRCS)/torread.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

torwrite: $(SRCS)/torwrite.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

%.o : %c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(SRCS)/*.o $(TARGETS)
