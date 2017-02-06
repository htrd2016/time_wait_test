C=gcc
CFLAGS=-g -Wall -O2
#LDFLAGS=-L/usr/lib/
#LIBS=-L/usr/lib/ -lmysqlclient -lpthread
#INCLUDES= -I./mysql-connector-c-6.0.2-linux-rhel5
TARGET_SERVER=Server
TARGET_CLIENT=Client

all:$(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_SERVER):
	$(CC) -o $(TARGET_SERVER) $(CFLAGS) server.c $(LIBS)

$(TARGET_CLIENT):
	$(CC) -o $(TARGET_CLIENT) $(CFLAGS) client.c $(LIBS)

clean:
	rm -f *.o $(TARGET_SERVER) $(TARGET_CLIENT)
