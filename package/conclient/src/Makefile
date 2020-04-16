OBJS=client.o config.o util.o
client:$(OBJS)
	$(CC) $^ -o $@ -luci $(CFLAGS)
OBJS:%.0:%.c
	$(CC) -c $< -o $@ -luci $(CFLAGS)

clean:
	rm *.o client
