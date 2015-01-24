CFLAGS = -O3 -Wall -pedantic -std=c99    
TARGET =  main
SOURCES = parser.c path.c draw.c $(TARGET).c

 
LIBS = -lm -framework SDL2
CC = gcc 

all: 
	$(CC) $(SOURCES) -o ./logo $(CFLAGS) $(LIBS)

	

clean:
	rm -f $(TARGET)

run: all
	$(TARGET) 

	$(TARGET) 
