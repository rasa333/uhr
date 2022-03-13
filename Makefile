CC             = gcc
CFLAGS         = -O -Wunused
LDFLAGS        = -s
prg            = uhr

%.o : %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -c $<

objects        		     := $(patsubst %.c,%.o,$(wildcard *.c))

all     : $(prg)

$(prg)	: $(objects)
	$(CC) $(LDFLAGS) -o $(prg) $(objects) -ltermcap -lm

clean:
	rm -f *.o $(prg)
