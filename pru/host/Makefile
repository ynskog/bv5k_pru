all: host

.PHONY: host

host: host.c
	gcc host.c -o bv5k_udp

clean:
	rm bv5k_udp

install: host
	mkdir /opt/bv5k/udp -p
	cp bv5k_udp /opt/bv5k/udp/
