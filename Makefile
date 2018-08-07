CC = g++
CFLAGS = 
OBJS = DS2482.o onewire.o
PROG = onewire
CPPCHECK = cppcheck --enable=all

all: $(PROG)

$(PROG): $(OBJS)
		$(CC) $(CFLAGS) $(OBJS) -o $@

onewire.o: onewire.cpp
DS2482.o: DS2482.h

clean:
		rm -f *~ *.o $(PROG) core a.out
check:
		$(CPPCHECK) *.cpp

#onewire:	onewire.c
#		gcc onewire.c -o onewire