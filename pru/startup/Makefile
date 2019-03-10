install:
	sudo mkdir /opt/bv5k/pru -p
	sudo cp *.sh /opt/bv5k/pru/
	sudo cp systemd/*.service /etc/systemd/system
	sudo systemctl enable bv5k_udp.service
	sudo systemctl enable bv5k_pru.service

