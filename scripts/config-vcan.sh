sudo modprobe vcan

sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0

# canbusload -b -r vcan0@1000000 & 
# 	canplayer -I candump.log
# pkill canbusload