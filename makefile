CC = g++
CCFLAGS = -Wall
Assign7 : assign7.o
	$(CC) $(CCFLAGS) -o Assign7 assign7.o

assign7.o : assign7.cc
	 $(CC) $(CCFLGAGS) -c assign7.cc

clean :
	- rm *.o
	- rm Assign7

