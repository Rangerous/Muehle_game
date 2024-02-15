CFLAGS = -g -Wall -Werror 
CC = gcc 

# Default Ziel ( das erste )
all: sysprak-client

sysprak-client: sysprak-client.c outputManagement.c playerHandling.c server_connection.c performConnection.c memoryManagement.c config.c semaphoreManagement.c think.c  piece.c spielstandAusgabe.c
	$(CC) $(CFLAGS) -o $@ sysprak-client.c outputManagement.c playerHandling.c server_connection.c performConnection.c memoryManagement.c config.c semaphoreManagement.c think.c piece.c spielstandAusgabe.c

outputTest: outputManagement_test.c outputManagement.c 
	$(CC) $(CFLAGS) -o $@ outputManagement_test.c outputManagement.c
	
playerTest: playerHanding_test.c playerHandling.c
	$(CC) $(CFLAGS) -o $@ playerHanding_test.c playerHandling.c

clean:
	rm -f *.o ./sysprak-client ./outputTest ./playerTest
