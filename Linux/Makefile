## MAKEFILE FOR RPI DISTRO ##
## Peter Forbes - 2/10/16 ##

######## VARS ########
CC = g++
MAIN_DEPS = img_proc.h web_utils.h
MAIN_OBJS = img_proc.o web_utils.o
EXTRA_FLAGS = -Wpedantic -Wall -g -std=c++11
LIBS = -L/usr/lib


######## MAIN BUILD RULES ########


main : main.o $(MAIN_OBJS)
	$(CC) main.o $(MAIN_OBJS) -o main $(CRYPTO_FLAGS)

main.o : main.cpp
	$(CC) -c main.cpp -o main.o $(EXTRA_FLAGS)


######## COMPONENT BUILD RULES ########


img_proc.o : img_proc.cpp img_proc.h
	$(CC) $(EXTRA_FLAGS) -c img_proc.cpp $(LIBS) -o img_proc.o

web_utils.o : web_utils.cpp web_utils.h
	$(CC) $(EXTRA_FLAGS) -c web_utils.cpp $(LIBS) -o web_utils.o

######## ADDITIONAL RULES ########

clean:
	rm *.o main

