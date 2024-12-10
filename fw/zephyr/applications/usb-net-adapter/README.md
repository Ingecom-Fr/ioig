# IoIg firmware tests with zephyr

## Configure, build and flash project

Copy or link this folder in ~/zephyrproject/applications folder

Build:

~~~sh
west build -b rpi_pico
~~~

Flash:

~~~sh
cp build/zephyr/zephyr.uf2 /media/$USER/RPI-RP2
~~~


## Configure device on host side:

Configure device's the network interface with:

~~~
IP address : 192.0.2.2
Netmask : 255.255.255.0
Gw: 0.0.0.0
~~~

Or 

Manually :

~~~sh
   ip addr show enx00005e005301
   sudo ip link set enx00005e005301 down
   sudo ip addr add 192.0.2.2/24 dev enx00005e005301 
   sudo ip link set enx00005e005301 up
~~~

Ping device:

~~~sh
ping 192.0.2.1
~~~


### Send UDP packets:
~~~sh
echo "Hello UDP" | nc -u 192.0.2.1 5001
~~~

### Listen UDP Packets:
~~~sh
nc -u -l 192.0.2.2 5001
~~~


### Send TCP packet:

~~~sh
echo "Hello TCP" | nc 192.0.2.1 5002
~~~

### Listen TCP Packets:

~~~sh
nc -l 192.0.2.2 5002
~~~sh