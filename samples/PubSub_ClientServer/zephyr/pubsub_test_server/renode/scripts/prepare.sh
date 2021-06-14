renode --disable-xwt --console -e 'emulation CreateTap "tap0" "tap" true; quit'
ip addr add 192.0.2.2/24 dev tap0
ip link set tap0 up
ip route add 232.0.0.0/8 dev tap0