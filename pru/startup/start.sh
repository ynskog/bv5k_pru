#!/bin/bash
source /lib/firmware/gpio_setup.sh
echo start | sudo tee /sys/devices/platform/ocp/4a32600*.pruss-soc-bus/4a300000.pruss/4a334000.*/remoteproc/remoteproc*/state
sleep 1;echo start | sudo tee /sys/devices/platform/ocp/4a32600*.pruss-soc-bus/4a300000.pruss/4a338000.*/remoteproc/remoteproc*/state

