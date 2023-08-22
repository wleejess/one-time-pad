CC = gcc
CFLAGS = -std=gnu99 -g

all: enc_server enc_client dec_server dec_client keygen

enc_server: enc_server.c
	$(CC) $(CFLAGS) -o $@ $<

enc_client: enc_client.c
	$(CC) $(CFLAGS) -o $@ $<

dec_server: dec_server.c
	$(CC) $(CFLAGS) -o $@ $<

dec_client: dec_client.c
	$(CC) $(CFLAGS) -o $@ $<

keygen: keygen.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f enc_server enc_client dec_server dec_client keygen

.PHONY: all clean

