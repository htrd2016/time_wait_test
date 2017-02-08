C=gcc
CFLAGS=-g -Wall -O2
#LDFLAGS=-L/usr/lib/
LIBS=-L/usr/lib/ -lpthread
#INCLUDES= -I./mysql-connector-c-6.0.2-linux-rhel5
TARGET_SERVER=Server
TARGET_CLIENT=Client
TARGET_SEND_DATA=SendData

all:$(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_SEND_DATA)

$(TARGET_SERVER):
	$(CC) -o $(TARGET_SERVER) $(CFLAGS) server.c $(LIBS)

$(TARGET_CLIENT):
	$(CC) -o $(TARGET_CLIENT) $(CFLAGS) client.c $(LIBS)

$(TARGET_SEND_DATA):
	$(CC) -o $(TARGET_SEND_DATA) $(CFLAGS) send_data.c $(LIBS)

clean:
	rm -f *.o $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_SEND_DATA)
