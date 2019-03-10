#!/bin/bash
echo stop | sudo tee /sys/devices/platform/ocp/4a32600*.pruss-soc-bus/4a300000.pruss/4a338000.*/remoteproc/remoteproc*/state
echo stop | sudo tee /sys/devices/platform/ocp/4a32600*.pruss-soc-bus/4a300000.pruss/4a334000.*/remoteproc/remoteproc*/state

