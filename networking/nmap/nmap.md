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
-sV    | try to determine service running
-Pn    | jump Host Discovery phase, scan all hosts given
-sn    | don't scan, just do Host Discovery

## Scan Cheatsheet

`nmap` has a lot of [Scan Types](#scan-types), and so it is good idea to have
a cheatsheet for them:

Option | Scan Type | Stealth   | Traffic
-------|-----------|-----------|---------
-sT    | connect   | full-open | (syn) -> \| <- (syn,ack) \| (ack) -> | (rst) ->

## Host Discovery Cheatsheet

`nmap` has a bunch of techniques to do [Host Discovery](#host-discovery).
This is useful since many hosts don't respond to the default ICMP
echo requests:

Option | Ping Type | Stealth | Traffic | Caviats
-------|-----------|---------|---------|
-PS<port list> | SYN | full-open | (syn) -> \| <- ((syn,ack) || (rst)) \| (rst)-> | 
-PA<port list> | ACK | no connection | (ack) -> \| <- (rst) |
-PU<port list> | UDP | no TCP used | (UDP on top of ICMP) -> \| <- (ICMP if port is closed) | open ports don't respond
-PE | ICMP echo request | standard `ping` | (echo request) -> \| <- (echo reply) | blocked by most firewalls
-PP | ICMP timestamp query | ICMP request | (timestamp query) -> \| <- (query response) | useful when only some ICMP packets are blocked
-PM | ICMP address mask query | ICMP request | (address mask query) -> \| <- (query response) | useful when only some ICMP packets are blocked
-PO<protocol list> | IP Protocol | depend on the protocol | (protocol packet) -> \| <- (response) | flexible
-PR | ARP | default ARP | normal ARP exchange | only difference from normal ARP is that `nmap` handles it to make the process faster for many hosts

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

#### TCP SYN Ping (-PS<port list>)

This option sends an empty TCP packet with the SYN flag set. The default port it sends
to is `80`. In linux, unprivileged users will just do a default `connect` system call instead.

For more info check the [online `nmap` book](https://nmap.org/book/host-discovery-techniques.html)

##### Trafiic

1. Client -> (SYN)

2. Server -> (RST) if port is closed, (SYN, ACK) if it is open

3. Client -> (RST) if Server sent (SYN,ACK)

obs.: Mind that for unprivileged users, a normal handshake would happen

`nmap` forcefully shutdowns the connection if an (SYN,ACK) was sent by the Server, and
doesn't care wheter the port is opened or closed, since at this stage it
only cares if the host is up.

#### TCP ACK Ping (-PA<port list>)

Similar to the above ping, but instead of SYN, we send an ACK. Since no connection is set,
the remote host should just return an RST.

`connect` is used instead for unprivileged users, and so you would just send an SYN and connect
normally.

It can work on some firewalls rules that ignore SYN pings.

For more info check the [online `nmap` book](https://nmap.org/book/host-discovery-techniques.html)

##### Traffic

1. Client -> (ACK)

2. Server -> (RST)

#### UDP Ping (-PU<port list>)

This scan sends an UDP packet to the given ports. For most ports, the UDP packet will be empty,
except for common ports like 53 and 161, where `nmap` add protocol-specific payload so it is
more likely to receive an response.

If an open port is reached, most services will just ignore the empty packet and return no response.

If the port is closed, the target host should send back an ICMP `port unreachable` packet.

For more info check the [online `nmap` book](https://nmap.org/book/host-discovery-techniques.html)

##### Traffic

1. Client sends UDP packet

2. If Server is alive and port is closed, it should send back an ICMP `port unreachable` packet.

Keep in mind that if no response if given, you can't know if the host is down or the port is closed.

#### ICMP Ping (-PE, -PP, -PM)

In addition to previous _Host Discovery_ techniques using unusual TCP and UDP packets, `nmap` can send
standard packets like the ones sent by the `ping` command or other ICMP-only packets.

Each of the options do the following:

* `-PE` - Echo request. Send echo request and expect echo reply. Exactly like `ping`.

* `-PM` - Address mask query. Query for an address mask.

* `-PP` - Timestamp query. Query for a timestamp. 

`-PM` and `-PP` can be really useful when firewalls don't block all kinds of ICMP packets.

#### IP Ping (-PO<protocol list>)

The idea of this ping is to be flexible. The protocol list given as argument correspond to the
protocol number in the IP header (default is 1, for ICMP).

The packets are sent with the protocol header on top of it, with no additional data
for the upper layers, except for ICMP, IGMP, TCP and UDP, where the packets are sent with
the proper protocol headers.

You can also use the `--data-lenght` option to add random data to the packet.

Any response given back means that the host is alive.

#### ARP Scan (-PR)

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

## Scan Types

### Connect Scan (-sT)

Default scan when SYN scan is not an option (when the user don't have raw packets privileges or using
IPv6). It just use the `connect` system call like most programs do.

This and TCP FTP bounce Scan, are the only scans unprivileged users can do. Pratically any IDS would
log this connection, since it is really just a legitimate connection.

#### Traffic

1. Client -> (SYN)

2. Server -> (SYN, ACK) if port is open, (RST) if it is closed

3. Client -> (ACK) if Server sent (SYN, ACK)

4. Client -> (RST)

### SYN Scan (-sS)

### FIN Scan (-sF)

### NULL Scan (-sN)

### Xmas Scan (-sX)

### ACK Scan (-sA)

### Window Scan (-sW)

### Maimon Scan (-sM)

### UDP Scan (-sU)

### Idlescan (-sI)

### Flexible Scan (--scanflags)

## Get More Info

### Scan Service (-sV)

### SO Scan (-O)
