src = $(wildcard *.c)
obj = $(src:.c=.o)

LDFLAGS = -lz -lm

part1: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) part1
