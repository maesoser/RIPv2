CC = gcc
INCL_PATH=include
CFLAGS=-I$(INCL_PATH) -Wall
BINPATH = bin/
SRC = src/
OBJPATH = obj

all: arp route ip udp aconf rip cap

raw:
	$(CC) -c $(SRC)rawnet.c $(SRC)timerms.c
	ar rs raw.a rawnet.o timerms.o

arp:
	$(CC) $(CFLAGS) -o $(BINPATH)arp_client $(SRC)rawnet.c $(SRC)timerms.c $(SRC)arp_client.c $(SRC)arp.c $(SRC)eth.c $(SRC)ipv4.c $(SRC)ipv4_route_table.c $(SRC)ipv4_config.c

route:
	$(CC) $(CFLAGS) -o $(BINPATH)route $(SRC)rawnet.c $(SRC)timerms.c $(SRC)eth.c $(SRC)arp.c $(SRC)ipv4.c $(SRC)ipv4_route_table.c $(SRC)ipv4_config.c $(SRC)route.c

ip:
	$(CC) $(CFLAGS) -o $(BINPATH)ipv4_server $(SRC)rawnet.c $(SRC)timerms.c $(SRC)eth.c $(SRC)arp.c $(SRC)ipv4.c $(SRC)ipv4_route_table.c $(SRC)ipv4_config.c $(SRC)ipv4_server.c
	$(CC) $(CFLAGS) -o $(BINPATH)ipv4_client $(SRC)rawnet.c $(SRC)timerms.c $(SRC)eth.c $(SRC)arp.c $(SRC)ipv4.c $(SRC)ipv4_route_table.c $(SRC)ipv4_config.c $(SRC)ipv4_client.c

udp:
	$(CC) $(CFLAGS) -o $(BINPATH)udp_client $(SRC)rawnet.c $(SRC)timerms.c $(SRC)eth.c $(SRC)arp.c $(SRC)ipv4.c $(SRC)ipv4_route_table.c $(SRC)ipv4_config.c $(SRC)udp.c $(SRC)udp_client.c
	$(CC) $(CFLAGS) -o $(BINPATH)udp_server $(SRC)rawnet.c $(SRC)timerms.c $(SRC)eth.c $(SRC)arp.c $(SRC)ipv4.c $(SRC)ipv4_route_table.c $(SRC)ipv4_config.c $(SRC)udp.c $(SRC)udp_server.c

rip:
	$(CC) $(CFLAGS) -o $(BINPATH)rip_client $(SRC)rawnet.c $(SRC)timerms.c $(SRC)eth.c $(SRC)arp.c $(SRC)ipv4.c $(SRC)ipv4_route_table.c $(SRC)ipv4_config.c $(SRC)udp.c $(SRC)rip_client.c
	$(CC) $(CFLAGS) -o $(BINPATH)rip_client_rellenarpaquete $(SRC)rawnet.c $(SRC)timerms.c $(SRC)eth.c $(SRC)arp.c $(SRC)ipv4.c $(SRC)ipv4_route_table.c $(SRC)ipv4_config.c $(SRC)udp.c $(SRC)rip_client_rellenarpaquete.c
	$(CC) $(CFLAGS) -o $(BINPATH)rip_server $(SRC)rawnet.c $(SRC)timerms.c $(SRC)eth.c $(SRC)arp.c $(SRC)ipv4.c $(SRC)ipv4_route_table.c $(SRC)ipv4_config.c $(SRC)udp.c $(SRC)rip_route_table.c $(SRC)rip_server.c

aconf:
	$(CC) $(CFLAGS) -o $(BINPATH)aconf $(SRC)aconf.c

.PHONY: clean

clean:
	rm -f $(BINPATH)*

cap:
	sudo setcap cap_net_admin,cap_net_raw=eip $(BINPATH)aconf
	sudo setcap cap_net_admin,cap_net_raw=eip $(BINPATH)ipv4_server
	sudo setcap cap_net_admin,cap_net_raw=eip $(BINPATH)ipv4_client
	sudo setcap cap_net_admin,cap_net_raw=eip $(BINPATH)arp_client
	sudo setcap cap_net_admin,cap_net_raw=eip $(BINPATH)route
	sudo setcap cap_net_admin,cap_net_raw=eip $(BINPATH)udp_server
	sudo setcap cap_net_admin,cap_net_raw=eip $(BINPATH)udp_client
	sudo setcap cap_net_admin,cap_net_raw=eip $(BINPATH)rip_client
	sudo setcap cap_net_admin,cap_net_raw=eip $(BINPATH)rip_server
