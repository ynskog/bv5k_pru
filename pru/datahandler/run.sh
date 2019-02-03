sudo make
sudo chmod 666 /dev/rpmsg_pru31
sudo cat /dev/rpmsg_pru31 > /dev/null &
sudo echo "lol" > /dev/rpmsg_pru31 # Need to write data from ARM once to initialize the communication channel

