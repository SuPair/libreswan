# one packet, which gets eaten by XFRM, so east does not initiate
ping -n -c 1 -I 192.1.3.209 192.1.2.23
# wait on OE retransmits and rekeying
sleep 5
# should show established tunnel and no bare shunts
ipsec whack --trafficstatus
ipsec whack --shuntstatus
ipsec look
killall ip > /dev/null 2> /dev/null
cp /tmp/xfrm-monitor.out OUTPUT/road.xfrm-monitor.txt
# ping should succeed through tunnel
ping -n -c 2 -I 192.1.3.209 192.1.2.23
echo done
