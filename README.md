# RIPv2 Protocol Implementation

## Intro

RIPv2 protocol implementation written by Irene López and I for the subject of Advanced Networks and Communication Services at the Carlos III University (uc3m).
For such application layer protocol to work we also developed a basic UDP/IP stack.


In order to have an easier access to the linux raw sockets, our teacher [Manuel Urueña](https://github.com/muruenya) developed for us the aid code [librawnetcc](https://github.com/muruenya/librawnet). On this repository we integrated his code to avoid installing the librawnet package and present the complete top-down solution.

## Layers

### Network Access (Raw and Ethernet)

![Ethernet](https://steveinit.files.wordpress.com/2017/12/ethernet-frame-page-1.png?w=940)

### ARP frame

![ARP](https://3.bp.blogspot.com/-FarNbNAGm54/WCLV0N88oJI/AAAAAAAADj0/SFktyQ5K7isi_eA2B5RZHIuy_sqXsmDUwCLcB/s640/ARP%2BHeader.png)

### Internet (IP)

![IPv4](https://nmap.org/book/images/hdr/MJB-IP-Header-800x576.png)

### Transport (UDP)

![UDP](https://nmap.org/book/images/hdr/MJB-UDP-Header-800x264.png)

### Application (RIPv2)

## RIPv2 server example

```
make all
cp bin/rip_server rip
cp bin/aconf aconf
./aconf -i wlp2s0
cp riptable.txt /tmp/riptable.txt
cp tables/riptable.txt riptable.txt
./rip riptable.txt
```

## RIPv2 client example

```
sudo ip addr del 192.100.100.126/24 dev eth1
sudo ip addr del 192.168.0.2/24 dev eth1
sudo ip addr add 10.0.9.2/24 dev eth1
make rip
make aconf
cp bin/aconf aconf
cp bin/rip_client rip_client
cp configs/routetable.txt routetable.txt
./aconf -i eth1
./rip_client 10.0.9.1
```

## Cisco iOS RIPv2 configuration example

```
sudo ip addr add 192.168.0.2/24 dev eth1
telnet 192.168.0.1

  config terminal

    interface eth0.1
      no ip add 192.168.1.1/24
      ip add 10.0.9.169/30
      no shutdown
    exit

    interface eth0.2
      no ip add 192.168.2.1/24
      ip add 10.0.9.161/30
      no shutdown
    exit

    interface eth0.3
      no ip add 192.168.3.1/24
      ip add 10.0.9.1/26
      no shutdown
    exit

    //Static route
    //Backup route

    ip route 10.0.9.64/26 10.0.9.170
    ip route 10.0.9.164/30 10.0.9.170
    ip route 10.0.9.172/30 10.0.9.170
    ip route 10.0.9.128/27 10.0.9.162

    // Configuración RIP

    router rip
      version 2
      network eth0.1
      network eth0.2
      network eth0.3
      network eth0.4
      redistribute static
      redistribute connected
    exit
  exit

```

## References

[Linux Raw Sockets](http://man7.org/linux/man-pages/man7/raw.7.html)

[Using Linux Raw Sockets](http://squidarth.com/networking/systems/rc/2018/05/28/using-raw-sockets.html)

[librawnetcc](https://github.com/muruenya/librawnet)


[Ethernet RFC](https://tools.ietf.org/html/rfc1042)

[ARP RFC](https://tools.ietf.org/html/rfc826)

[IP RFC](https://tools.ietf.org/html/rfc791)

[UDP RFC](https://tools.ietf.org/html/rfc768)

[RIPv2 RFC](https://tools.ietf.org/html/rfc2453)
