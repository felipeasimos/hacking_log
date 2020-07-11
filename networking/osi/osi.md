# OSI Model

sources:
https://www.travessa.com.br/ccna-6-0-guia-completo-de-estudo-2-ed-2018/artigo/803f6cd0-e70a-4e0e-8d51-05559bc1a61b

OSI stands for Open Systems Interconnection. The model is used as a base for the connections
models used in computer nowadays, like the TCP/IP model.

![OSI Model structure](http://2.bp.blogspot.com/-oL2B7rB_Ddc/UXus4JZPgXI/AAAAAAAAAKc/e0pDS9JpRD8/s1600/osi.gif)

## Overview

This is the model structure:

Application  |
-------------|
Presentation |
Session      |
Transport    |
Networking   |
Data Link    |
Physical     |

* __Application__ - SMTP, some internet browser

	* The application trying to exchange information with another one.

* __Presentation__ - JPEG, ASCII, GIF

	* Translation of data between __Application__/__Session__ layers

	* Compression/Decompression

	* Cryptograhpy

* __Session__

	* Keeps different fluxes of data apart (connect to two sites at the same time, without one interfering with the other).

	* Session management

* __Transport__ - TCP, UDP

	* Segmentation/Reconstruction of fluxes of data

	* Provide logical connection between applications (ex: TCP port fields) 

	* Responsible for multiplexing fluxes of data

	* Establishment/Finalization of virtual (logical) cirtcuitry

	* Mask all info from upper layers, providing transparency in the transmition

	* Integrity of data (ex: TCP ACKs)

* __Networking__ - IP, ICMP

	* Logical routing of the packets (ex: IP)

	* 2 types of packets: data packets and router update packets.

* __Data Link__ - ARP, Ethernet

	* Bridge between upper layer (__Networking__) and lower (__Physical__), allowing compatibility for many physical devices

	* Format message in frames, add own header (ex: ethernet)

* __Physical__

	* Digital to Virtual translation
