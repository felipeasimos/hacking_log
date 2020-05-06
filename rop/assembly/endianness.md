# Endianness

sources:
https://betterexplained.com/articles/understanding-big-and-little-endian-byte-order/ \
https://en.wikipedia.org/wiki/Endianness

## The Problem

Every computer read a single byte in same manner. If you send a byte with
the value 0xA4 (164 in hexadecimal) to a friend's computer, they would be
able to read it in the same way you do.

However, the same does not apply when we are reading from a data structure
consisting of multiple bytes. Some machines look at data in a
Little-Endian way and others in a Big-Endian way.

## Bytes and Addresses

Little-Endian and Big-Endian are defined by where the stream of bytes begin
and where it ends in memory.

* Little-Endian - lowest address is where the stream begins and it ends at 
the highest.

* Big-Endian - highest address is where the stream begins and it ends at
the lowest.

For example, let's say we have the number 108. With two bytes we would
represent it as 0110 1100, where 1100 is the least significant byte and
0110 is the most significant byte.

In Big-Endian, we would store this number in memory like so:

Address | Value
--------|--------
0x1 | 1100
0x0 | 0110

Some say this is the most "intuitive" endianess, since the computer starts
reading from the most significant byte just like we do.

In Little-Endian, we would store it like so:

Address | Value
--------|-------
0x1 | 0110
0x0 | 1100

Here the least significant byte is at the lowest address.

## Communication

When sending data to another computer, in which endianness should we send
it? Most machines nowadays use Little-Endian but Big-Endian is used when
sending data to another computer and its what every computer expects to
receive.

So it doesn't really matter which endianness the receiver uses, it only
matters that the data is sent using Big-Endian. Once received the data
will be translated to Little-Endian if the receiver uses it.
