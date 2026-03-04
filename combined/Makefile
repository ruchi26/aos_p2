MPICC = mpicc.mpich

OMPFLAGS = -fopenmp
OMPLIBS = -lgomp

CFLAGS = -g -std=gnu99 -I. -Wall $(OMPFLAGS)
LDLIBS = $(OMPLIBS) -lm

MP_SRC1 = gtmpi1.c
MP_SRC2 = gtmp1.c

all: combined 

SRCS := $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

%.o: %.c
	$(MPICC) -c $(CFLAGS) $< -o $@

combined: $(OBJS) 
	$(MPICC) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS)

clean:
	rm -rf *.o *.dSYM combined

