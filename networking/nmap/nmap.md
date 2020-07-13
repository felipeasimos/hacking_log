# Nmap

dependencies:
[OSI Model](../osi/osi.md)

sources:
https://nmap.org/book/toc.html \
https://www.youtube.com/watch?v=4t4kBkMsDbQ \
https://academiaclavis.com.br/courses/exin-ethical-hacking-e-comptia-pentest-mais \
https://www.geeksforgeeks.org/tcp-flags/ \
https://geek-university.com/nmap/port-states/ \
https://www.lifewire.com/internet-protocol-tutorial-subnets-818378 \
https://nmap.org/book/host-discovery-specify-targets.html \
https://www.digitalocean.com/community/tutorials/understanding-ip-addresses-subnets-and-cidr-notation-for-networking \
https://nmap.org/book/host-discovery-techniques.html \
https://linuxhint.com/nping_nmap_arp_scan/ \
https://www.lifewire.com/address-resolution-protocol-817941 \

`nmap` stands for Network Mapper. As you can guess by the name, it is a
really popular network reconnaissance tool that let us scan for specific
ports, hosts and networks while having a lot of control on how the scan
itself is done.

The focus of this paper is in the scan techniques, but keep in mind that
`nmap` has also a lot of options to specify ports, hosts, scan speed, DNS
resolution and much more.

Keep in mind that `nmap` is really limited when not run with an user with
raw packets permissions. Running as an unprivileged user will not only limit
the options at your disposal, it will also restrict `nmap` from gathering
some information about the hosts (like MAC addresses in some instances).

## Installation

In most linux distributions, `nmap` can be installed directly through the
package manager. For example, in Debian based distributions this will do
the job:

```
sudo apt install nmap
```

## Common Options

Option | Function
-------|--------
-sV    | try to determine service version running
-Pn    | jump Host Discovery phase, scan all hosts given
-sn    | don't scan, just do Host Discovery
-A     | traceroute, OS detection, version detection and script scanning
-D <ip list> | create decoys. Send duplicate packets with decoy IPs as source
-O     | detect target's operating system
-T     | control scanning speed

## Scan Cheatsheet

`nmap` has a lot of [Scan Types](#scan-types), and so it is good idea to have
a cheatsheet for them:

Option | Scan Type | Stealth   | Traffic | Results | Caviats
-------|-----------|-----------|---------|---------|---------
-sT    | [connect](#connect-scan--st)   | full-open | (syn) -> \| <- (syn,ack) \| (ack) -> \| (rst) -> | [`open`][`closed`][`filtered`] |
-sS    | [SYN](#syn-scan--ss)       | half-open | (syn) -> \| <- (syn,ack) \| (rst) -> | [`open`][`closed`][`filtered`] |
-sU    | [UDP](#udp-scan--su)       | packet reaches application | (UDP packet) -> \| <- (UDP response) | [`open`][`open\|filtered`][`closed`][`filtered`] |
-sA    | [ACK](#ack-scan--sa)       | send a TCP ACK | (ack) -> \| <- (rst) | [`unfiltered`][`filtered`] | used to determine firewall behaviour
-sW    | [Window](#window-scan--sw)    | send a TCP ACK | (ack) -> \| <- (rst) | [`open`][`closed`][`filtered`] | Only works in systems which the TCP window size field is 0 for `closed` ports. If not, all ports `open`
-sF    | [FIN](#null-fin-and-xmas-scans--sn--sf--sx)       | send a TCP FIN | (fin) -> \| <- (rst) | [`open\|filtered`][`closed`][`filtered`] | Only works in systems following the RFC segment: return RST for `closed` ports when receiving a packet without SYN,ACK or RST
-sN    | [NULL](#null-fin-and-xmas-scans--sn--sf--sx)      | send a TCP with no flags set | () -> \| <- (rst) | [`open\|filtered`][`closed`][`filtered`] | Only works in systems following the RFC segment: return RST for `closed` ports when receiving a packet without SYN,ACK or RST
-sX    | [Xmas](#null-fin-and-xmas-scans--sn--sf--sx)      | send a TCP PUSH,URG,FIN | (push,urg,fin) -> \| <- (rst) | [`open\|filtered`][`closed`][`filtered`] | Only works in systems following the RFC segment: return RST for `closed` ports when receiving a packet without SYN,ACK or RST
-sM    | [Maimon](#maimon-scan--sm)    | send a TCP FIN,ACK | (fin,ack) -> \| <- (rst) | [`open\|filtered`][`closed`][`filtered`] | Based on FIN,NULL and Xmas Scans. Works in many BSD-derived systems that drop packet if port is `open`, return RST if `closed`
--scanflags | [Flexible](#flexible-scan---scanflags) | send a TCP with given flags set | (given flags) -> \| <- (rst) | [`open\|filtered`][`closed`][`filtered`] | `nmap` determine port state like in the [Port State cheatsheet](#port-states)
-sI    | [Idle](#idlescan--si-zombie-ip)      | you don't even send packets to target | no interaction between attacker and target | [`open`][`closed`][`filtered`] | Depends on access to a zombie (idle host that set IP ID incrementally and in a global way)
-sO    | [IP Protocol](#ip-protocol-scan--so) | depend on the protocol | (protocol packet) -> \| <- (response) | [`open`][`closed`][`filtered`][`open\|filtered`] | The results refer to the protocols used, not ports. `-p` is used to give the protocol number.

## Host Discovery Cheatsheet

`nmap` has a bunch of techniques to do [Host Discovery](#host-discovery).
This is useful since many hosts don't respond to the default ICMP
echo requests:

Option | Ping Type | Stealth | Traffic | Caviats |
-------|-----------|---------|---------|---------|
-PS<port list> | [SYN](#tcp-syn-ping--psport-list) | full-open | (syn) -> \| <- ((syn,ack) || (rst)) \| (rst)-> | unprivileged users just `connect` (send ACK back)
-PA<port list> | [ACK](#tcp-ack-ping--paport-list) | no connection | (ack) -> \| <- (rst) | unprivileged users just `connect`
-PU<port list> | [UDP](#udp-ping--puport-list) | no TCP used | (UDP on top of ICMP) -> \| <- (ICMP if port is closed) | open ports don't respond
-PE | [ICMP echo request](#icmp-ping--pe--pp--pm) | standard `ping` | (echo request) -> \| <- (echo reply) | ignored by most hosts
-PP | [ICMP timestamp query](#icmp-ping--pe--pp--pm) | ICMP request | (timestamp query) -> \| <- (query response) | useful when only some ICMP packets are blocked
-PM | [ICMP address mask query](#icmp-ping--pe--pp--pm) | ICMP request | (address mask query) -> \| <- (query response) | useful when only some ICMP packets are blocked
-PO<protocol list> | [IP Protocol](#ip-ping--poprotocol-list) | depend on the protocol | (protocol packet) -> \| <- (response) | flexible
-PR | [ARP](#arp-scan--pr) | default ARP | normal ARP exchange | only difference from normal ARP is that `nmap` handles it to make the process faster for many hosts. Default scan for LAN

## Port States

`nmap`'s output show the state of the ports scanned. Here is a quick cheatsheet to understand what each state means:

State  | Represent
-------|----------
open   | application is listening for connections on the port
closed | probes were received, but there is no application on the port
filtered | probes were not received and the state could not be determined
unfiltered | probes were received but states could not be determined
open/filtered | port is either open of filtered
closed/filtered | port is either closed or filtered

## TCP

To stablish a TCP connection, the hosts have to go through a TCP
3-way handshake. It is important to know this, since its is a part
of the protocol that `nmap` scans manipulate a lot to get information.

There is also an exchange of packets that happens to close the connection
properly. The entire connection happens like so:

```
Client                            Server
  |           (SYN) ->               |
  |                                  |
  |           <- (SYN, ACK)          |
  |                                  |
  |           (ACK) ->               |
  |                                  |
  | ... connection stablished ...    |
  |                                  |
  |           (FIN) ->               |
  |                                  |
  |           <- (FIN,ACK)           |
  |                                  |
  |           (ACK) ->               |
  |                                  |
  |  ... connection finished ...     |
```

Mind that the  server could also send the initial `FIN` packet to end the
connection if it wanted.

### TCP Flags

What the `SYN`, `ACK` and `FIN` represent above is the flags in a TCP
packet. Each flag has a role to play:

Flag | Represent        | Function
-----|------------------|----------
SYN  | synchronization  | Used to stablish connection
ACK  | acknowlegement   | Confirm to other host that certain packets have arrived
FIN  | finished         | Used to perform a graceful shutdown
RST  | reset            | Alert host that the current connection is closed, need to reset connection. Used when there is an error.
PUSH | push             | Alert both hosts transport layer to forward the packet immediatly
URG  | urgent           | Alert receiver's transport layer to forward packet to application layer immediatly, packet is delivered out of sequence

In [Scan Types](#scan-types) you can see how `nmap` doesn't play by rules of the
protocol and based on how the remote host responds, it can gather information about it.

## Basic Usage

A really basic `nmap` command to scan your own computer's `80` port:

```
nmap 127.0.0.1 -p 80
```

And that is basically how the semantic goes: `nmap <specify host> -p <specify ports>`

## Specify Hosts

Everything in the `nmap` command line that isn't an option or option argument is treated
as target host specification.

One simple way to scan many hosts is just to list the many hosts you want to scan:

```
nmap 127.0.0.1 192.168.0.6
```

Another way, is to specify the number of bits to change:

```
nmap 192.168.0.0/24
```

This is called CIDR (Classless Inter-Domain Routing) notation. And it basically says that
when you write `x.x.x.x/numbits` you are refererring to every IP address that has a different
combination of its bits, except for the first `numbits`.

For the above example, it will go through every IP `192.168.x.x`, where `x` can be any combination
of 8 bits.

You can basically use it to scan your entire network!

Of course, you can also use the domain name for the scans, `nmap` itself has this `scanme.nmap.org`
domain for you to scan while praticing with the tool:

```
nmap scanme.nmap.org
```

For more info check the [Nmap book](https://nmap.org/book/host-discovery-specify-targets.html)

## Specify Ports

To specify ports to scan, we need to use the `-p` option flag.

you can specify the ports directly, using commas to separate between different ports:

```
nmap 127.0.0.1 -p 80
nmap 127.0.0.1 -p 80,443
```

You can also use a dash (`-`) to specify a range of ports:

```
nmap 127.0.0.1 -p 1-1100
```

Or use the name of a service, so `nmap` translates it to the port in which the service is
commonly used:

```
nmap 127.0.0.1 -p ssh
```

You can feel free to go crazy with this:

```
nmap 127.0.0.1 -p 83,ssh,1-5
```

## Host Discovery

### Disable Scan

Before scanning hosts, we need to know if they are up and running.

Or maybe, thats actually the only thing you want to do. In which
case you can just use the `-sn` option. With `-sn` you can still
use `--script` and `--traceroute` though:

```
nmap 192.168.0.0/24 -sn
```

### Disable Host Discovery

The above command can be really useful to check the hosts in LAN.

Normally, `nmap` check which hosts are alive before scanning
them (not stealth at all), with `-Pn` it jumps immediatly to
the scanning phase. The downfall is that since `nmap` doesn't know
which hosts are up, it will scan every IP given to it.

### Host Discovery Techniques

By default, `nmap` does _Host Discovery_ by pinging the hosts, just
like the `ping` command does, sending ICMP echo requests and waiting 
for responses.

However, as a security measure, many hosts (even servers) don't respond to these
ICMP requests, even though it is stated in the RFC requirement.

To combat this, `nmap` has some techniques that can be used in _Host Discovery_.

Keep in mind that the we still use ICMP in all of these, the difference is what
we add to the payload in the upper layer.

#### TCP SYN Ping (`-PS<port list>`)

This option sends an empty TCP packet with the SYN flag set. The default port it sends
to is `80`. In linux, unprivileged users will just do a default `connect` system call instead.

For more info check the [online `nmap` book](https://nmap.org/book/host-discovery-techniques.html#host-discovery-PS)

##### Trafiic

1. Client -> (SYN)

2. Server -> (RST) if port is closed, (SYN, ACK) if it is open

3. Client -> (RST) if Server sent (SYN,ACK)

obs.: Mind that for unprivileged users, a normal handshake would happen

`nmap` forcefully shutdowns the connection if an (SYN,ACK) was sent by the Server, and
doesn't care wheter the port is opened or closed, since at this stage it
only cares if the host is up.

#### TCP ACK Ping (`-PA<port list>`)

Similar to the above ping, but instead of SYN, we send an ACK. Since no connection is set,
the remote host should just return an RST.

`connect` is used instead for unprivileged users, and so you would just send an SYN and connect
normally.

It can work on some firewalls rules that ignore SYN pings.

For more info check the [online `nmap` book](https://nmap.org/book/host-discovery-techniques.html#host-discovery-PA)

##### Traffic

1. Client -> (ACK)

2. Server -> (RST)

#### UDP Ping (`-PU<port list>`)

This scan sends an UDP packet to the given ports. For most ports, the UDP packet will be empty,
except for common ports like 53 and 161, where `nmap` add protocol-specific payload so it is
more likely to receive an response.

If an open port is reached, most services will just ignore the empty packet and return no response.

If the port is closed, the target host should send back an ICMP `port unreachable` packet.

For more info check the [online `nmap` book](https://nmap.org/book/host-discovery-techniques.html#host-discovery-PU)

##### Traffic

1. Client sends UDP packet

2. If Server is alive and port is closed, it should send back an ICMP `port unreachable` packet.

Keep in mind that if no response if given, you can't know if the host is down or the port is closed.

#### ICMP Ping (`-PE`, `-PP`, `-PM`)

In addition to previous _Host Discovery_ techniques using unusual TCP and UDP packets, `nmap` can send
standard packets like the ones sent by the `ping` command or other ICMP-only packets.

Each of the options do the following:

* `-PE` - Echo request. Send echo request and expect echo reply. Exactly like `ping`.

* `-PM` - Address mask query. Query for an address mask.

* `-PP` - Timestamp query. Query for a timestamp. 

`-PM` and `-PP` can be really useful when firewalls don't block all kinds of ICMP packets.

For more info check the [online `nmap` book](https://nmap.org/book/host-discovery-techniques.html#host-discovery-icmpping)

#### IP Ping (`-PO<protocol list>`)

The idea of this ping is to be flexible. The protocol list given as argument correspond to the
protocol number in the IP header (default is 1, for ICMP).

The packets are sent with the protocol header on top of it, with no additional data
for the upper layers, except for ICMP, IGMP, TCP and UDP, where the packets are sent with
the proper protocol headers.

You can also use the `--data-lenght` option to add random data to the packet.

Any response given back means that the host is alive.

For more info check the [online `nmap` book](https://nmap.org/book/host-discovery-techniques.html#host-discovery-PO)

#### ARP Scan (`-PR`)

A common use for `nmap` is to LAN networks. For most LANs, the majority of the IP addresses
are not used at any given time. Because of this, when an IP is from a LAN (you can know this
just by looking at the IP address), `nmap` by default performs the ARP Scan for _Host Discovery_.

An ARP Scan is just the normal ARP protocol exchange to know to what MAC an private IP address
belongs. Inside a LAN you need to know the MAC of a host to send something to it.

As a consequence, our computer end up sending ARP requests before the `nmap` scan. However, this
can be bad new when scanning many different hosts inside LAN, that's where we use `nmap`'s ARP scan:
it basically gives `nmap` the control over these ARP requests so it can optimize the process for its
purposes.

In summary, ARP Scans are not stealthy at all (`nmap` is just performing normal ARP requests), however
they are the fastest scan in LAN (since `nmap` has to send ARP requests to know the hosts MAC anyway)
and thus this is the default scan for LAN networks.

For more info check the [online `nmap` book](https://nmap.org/book/host-discovery-techniques.html#arp-scan)

## Scan Types

### Connect Scan (`-sT`)

Default scan when SYN scan is not an option (when the user don't have raw packets privileges or using
IPv6). It just use the `connect` system call like most programs do.

This and TCP FTP bounce Scan, are the only scans unprivileged users can do. Pratically any IDS would
log this connection, since it is really just a legitimate connection.

For more info check the [online `nmap` book](https://nmap.org/book/scan-methods-connect-scan.html)

#### Traffic

1. Client -> (SYN)

2. Server -> (SYN, ACK) if port is __open__, (RST) if it is __closed__

3. Client -> (ACK) if Server sent (SYN, ACK)

4. Client -> (RST)

#### Response Table

Probe Response | Assigned Table |
---------------|----------------|
TCP SYN,ACK    | `open` |
TCP RST        | `closed` |
No response (even after retransmissions) | `filtered` |
ICMP unreachable error | `filtered` |

### SYN Scan (`-sS`)

Default scan. It is quick and relatively stealthy, since it nevers fully stablishes
a TCP connection.

For more info check the [online `nmap` book](https://nmap.org/book/synscan.html)

#### Traffic

1. Client -> (SYN)

2. Server -> (SYN,ACK) if port is open, (RST) if closed

3. Client -> (RST)

The port is considered `filtered` if `nmap` receives no response or an ICMP unreachable
error packet.

#### Response Table

Probe Response | Assigned Table |
---------------|----------------|
TCP SYN,ACK    | `open` |
TCP RST        | `closed` |
No response (even after retransmissions) | `filtered` |
ICMP unreachable error | `filtered` |

### UDP Scan (`-sU`)

Sends an empty UDP packet, except for some common ports, where some payload is added
to increase the chances of getting a response.

For more info check the [online `nmap` book](https://nmap.org/book/scan-methods-udp-scan.html)

#### Traffic 

1. Client send UDP packet

2. Server responds (or don't)

Since UDP doesn't stablish a connection, we don't need to send anything back.

#### Response Table

Probe Response | Assigned Table |
---------------|----------------|
Any UDP packet | `open` |
No response received (even after retransmissions) | `open|filtered`
ICMP unreachable error (type 3, code 3) | `closed`
Other ICMP unreachable errors (type 3, code 1, 2, 9, 10, 11 or 13) | `filtered`

The `open|filtered` state is due to UDP not having to stablish a connection: since we
are just sending empty packets, the packet will be analysed by the application directly,
which will most likely just ignore it, since it is an invalid packet.

So we can't know for sure that the port is `filtered` if we didn't receive a response.

### ACK Scan (`-sA`)

This scan is not really used to determine which ports are opened, but is useful to
figure out firewall rulesets, and wheter they are statefull or not.

It sends a packet with the ACK flag set, and thus a RST is received for both opened
and closed ports, which are marked as `unfiltered` (they are reachable, but the state
can't be determined).

Ports that don't respond are labeled as `filtered`.

Let's say that from an FIN Scan, you get that a port is either `open` or 
`filtered` and ACK Scan tells you that the port is `unfiltered` (`open` or
`filtered`). You now know that the port is actually `open`!

For more info check the [online `nmap` book](https://nmap.org/book/scan-methods-ack-scan.html)

#### Traffic

1. Client -> (ACK)

2. Server -> (RST) || nothing

#### Response Table

Probe Response | Assigned Table |
---------------|----------------|
TCP RST response | `unfiltered` |
No response received (even after retransmissions) | `filtered` |
ICMP unreachable error (type 3, code 1, 2, 3, 9, 10 or 13) | `filtered` | 

### Window Scan (`-sW`)

In some systems, the window size field in the TCP header response
(even when is just RST) is return 0 for closed ports and a positive
number for open ports. This is what this scan evaluates.

Only a small number os systems behave like this, for the others, this
scan will perceive all their ports as `closed`.

This scan, just like the ACK Scan, sends a packet with the ACK flag set.

For more info check the [online `nmap` book](https://nmap.org/book/scan-methods-window-scan.html)

#### Traffic

1. Client -> (ACK)

2. Server -> (RST) || (no response)

#### Response Table

Probe Response | Assigned Table |
---------------|----------------|
TCP RST response with non-zero window field | `open` |
TCP RST response with zero window field | `closed` |
No response received (even after retransmissions) | `filtered`
ICMP unreachable error (type 3, code 1, 2, 3, 9, 10, 11 or 13) | `filtered`

### NULL, FIN and Xmas Scans (`-sN`, `-sF`, `-sX`)

The TCP RFC says "if the [destination] port state is CLOSED .... an incoming segment not
containing a RST causes a RST to be sent in responsea". The following pages discuss about
ignoring packets sent to open ports without the SYN, RST or ACK flags sets, saying: 
"you are unlikely to get here, but if you do, drop the segment, and return".

Therefore, systems compliant with the RFC text, when receiving a packet without the
SYN, RST or ACK flags, should return RST if the port is closed and don't respond if the
port is open.

This is the idea behind these three scans:

* __NULL Scan (-sN)__ - doesn't set any flags.

* __FIN Scan (-sF)__ - only set the FIN flag.

* __Xmas Scan (-sX)__ - set the FIN, PSH and URG flags (lighting the flags field like a christimas tree).

These scans can sneak past some non-statefull firewalls and packet filtering routers. The downside is that
not all systems are complaint with the RFC, which may cause RST packets to be sent back even when the port
is opened (__Keep that in mind!__).

These scans don't usually work with Microsoft Windows and many Cisco devices, but does work against most
Unix-based systems.

For more info check the [online `nmap` book](https://nmap.org/book/scan-methods-null-fin-xmas-scan.html)

#### Traffic

1. Client sends packet.

2. Server should respond RST if port is closed and don't respond if it is opened.

#### Response Table

Probe Response | Assigned Table |
---------------|----------------|
No response received (even after retransmissions) | `open|filtered`
TCP RST packet | `closed`
ICMP unreachable error (type 3, code 1, 2, 3, 9, 10, 11 or 13) | `filtered`

If `nmap` receives an ICMP destination prohibited error, it knows the port is
filtered, but most filters simply drop banned probes without any response, making
the ports appear opened. Since `nmap` can't be sure which is which, it marks them
as `open|filtered`.

### Maimon Scan (`-sM`)

Named after the one who discovered it, Uriel Maimon. The idea is the same
behind the [NULL, FIN and Xmas Scans](#null-fin-and-xmas-scans--sn--sf--sx),
except that the probe is FIN/ACK.

According to the RFC, a RST packet should be generated in response to such a 
probe wheter the port is `open` or `closed`, however, Uriel noticed that many
BSD-derived systems simply drop the packet if the port is `open`.

For more info check the [online `nmap` book](https://nmap.org/book/scan-methods-maimon-scan.html)

#### Traffic

1. Client -> (FIN,ACK)

2. Server -> No response if `open` or `filtered`, (RST) if closed

#### Response Table

Probe Response | Assigned Table |
---------------|----------------|
No response received (even after retransmissions) | `open|filtered` |
TCP RST packet | `closed` | 
ICMP unreachable error (type 3, code 1, 2, 3, 9, 10, 11 or 13) | `filtered` |

### Flexible Scan (`--scanflags`)

Until now we saw many scans that simply set some TCP flags and wait for a response.

The `--scanflags` option adds more flexibility to your scans, allowing you to choose
what flags to set!

For example, to set the SYN and FIN flags at the same time (the order of the flags
doesn't matter):

```
nmap --scanflags SYNFIN localhost -p 65000
```

Or even do the Xmas scan from before:

```
nmap --scanflags FINPSHURG lcoalhost -p 65000
```

If you compare this result with the result of an Xmas scan using `-sX`,
you can see that they are different, since `nmap` follows the usual logic
from the [Port States](#port-states) cheatsheet when using `--scanflags`.

For more info check the [online `nmap` book](https://nmap.org/book/scan-methods-custom-scanflags.html)

### Idlescan (`-sI <zombie ip>`)

Not only stealthy, this scan can also be used to discover IP-based trust systems between hosts.
Although more complicated to understand, this scan can lead to huge payoff!

It is based in these three concepts:

* To determine if a TCP port is opened, send it a SYN packet. The target machine will respond
with a SYN/ACK if the port is open, and RST if it is closed. Yes, i am just describing a SYN Scan.

* A machine that receives an unsolicited SYN/ACK packet will respond with a RST. An unsolicited
RST will be ignored.

* Every IP packet has a fragment identification number (IP ID). Since many systems simply
increment this number for each packet sent, probing for the IPID can tell an attacker how
many packets have been sent since the last probe.

Using these, we can scan a target while forging our identity as a "zombie" machine.

For more info check the [online `nmap` book](https://nmap.org/book/idlescan.html)

#### Idlescan Step By Step

1. Probe the zombie's IP ID and record it.

2. Forge a SYN packet from the zombie and send it to target. Depending on the port state, the
target's reaction may or may not cause the zombie's IP ID to be incremented.

3. Probe the zombie's IP ID again. The target port state is then determined by comparing this new
IP ID with the one recorded at step 1.

What the IP ID let us know at the end is the number of packets sent by the zombie to the target
machine, we can infer some information based on the number of packets the zombie has sent:

* __0__ - zombie received a RST from the target and ignored it (port `closed`). Or the zombie didn't
receive anything (port `filtered`).

* __1__ - usually is because the zombie receives a SYN/ACK from the target (port `open`) and responds
with a RST.

* __more than 1__ - zombie is a "bad" zombie (uses non-incremental IP ID values) or it has been
communicating with other hosts while the scan was happening.

As you can see, we can't differ `closed` from `filtered`.

#### Finding A Working Zombie Host

To make the scan work, we need a proper zombie. A host that assign IP ID values incrementally
on a global basis. It also should be idle, to avoid communication between other hosts and the 
zombie to get in our way. The optimal zombie would also have a low latency for both target and
attacker.

`nmap` tests the proposed zombie and reports any errors with it. If one doesn't work, try another.

A good approach is to test a zombie from a network close from you and the target, to provide low
latency. IO devices like printers are a good start, since they are not constanly used.

Try using OS detection (`-O`) to find a good zombie. With it, `nmap` can determine the IP ID
sequence generation method, and if the type shown is `Incremental` or
`Broken little-endian incremental`, it may be a good zombie (although it may just increase IP ID
per-host).

### IP Protocol Scan (`-sO`)

You can set the IP protocol to be used in the scan. If this option is used, the `-p` argument
will be used to give the desired protocols, instead of the desired ports.

The IP protocol payload is empty, except for some popular protocols For these, a header is
added (TCP, UDP, ICMP, etc).

#### Traffic

1. Client sends packet with desired protocol

2. Server sends response (or none)

#### Response Table

Probe Response | Assigned State |
---------------|----------------|
Any response   | `open` (for protocol used as response) |
ICMP protocol unreachable error (type 3, code 2) | `closed` |
Other ICMP unreachable errors (type 3, code 1, 3, 9, 10 or 13) | `filtered` (though they prove ICMP is `open` if sent from the target machine)
No response received (even after retransmissions) | `open\|filtered`
