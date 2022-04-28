# Evaluator Make File
# James Stockton


# make file commands
#	all 	- compiles whole program
#	run		- run file
#	clean	- deletes compiled files


# compiler constants
CC = gcc
CFLAGS = 
TARGET = evaluator



all: $(TARGET).exe

run: all
	./$(TARGET).exe

clean:
	rm -f $(TARGET).exe


$(TARGET).exe: $(TARGET).c
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET).exe