# Caching

sources:
http://www.ic.uff.br/~simone/arqcomp/contaulas/aula3.pdf \
http://www.ic.uff.br/~simone/arqcomp/contaulas/aula5.pdf
https://en.wikipedia.org/wiki/Locality_of_reference \
https://www.eecg.utoronto.ca/~moshovos/ECE243-07/l26-caches.html \
https://www.d.umn.edu/~gshute/arch/cache-addressing.xhtml \
https://course.ccs.neu.edu/com3200/parent/NOTES/cache-basics.html \
http://thebeardsage.com/cache-optimization-critical-word-first-and-early-restart/ \
https://courses.cs.washington.edu/courses/cse378/09wi/lectures/lec15.pdf \
https://en.wikipedia.org/wiki/Cache_replacement_policies#Least_recently_used_(LRU)

Most computing systems have a cache to improve the access to memory

## Key Concepts

Caches work so well because of the __Principle of locality__. That actually
has two different types:

	* __Temporal__ - There is a high chance that a resource or data
	that has just been accessed will be accessed again.

	* __Spatial__ - There is a high chance that data or resources close
	to one that has just accessed will be accessed again.

Keep that in mind when thinking why caches works.

## Memory Hierarchy

Between the disk and the processor there are many memory layers.

![Memory hierarchy](https://external-content.duckduckgo.com/iu/?u=https%3A%2F%2Ftse1.mm.bing.net%2Fth%3Fid%3DOIP.bIhu_2Gi1FiYJ4TC_zGWlAHaEi%26pid%3DApi&f=1)

The storage space of each layer is smaller than the layers below it.

Each layer store some addresses with the data related to it.

## Basic Functionality

Due to __Spatial locality__, we don't just return the address to the
upper layer, we send a block with the addresses close with the requested
address, since they will likely also be used.

When the processor wants to access an address, the closest memory layer:

1. Check if it has a block (also called lines or slots) that has the address.

2. If it doesn't (__cache miss__), ask for the layer below it (recursively):

	* If the layer below it gives the block, write the block to
	the current layer and send to the upper layer.

	* If the layer below doesn't give the block, notify the upper
	layer the error (end of recursion, just send the notification to
	the upper layers).

3. If it does (__cache hit__), give it to the upper layer.

Obs.: Writing the requested address when the block is full and the address
is not in it will force us to evict another address from the block, we
will talk about this later.
 
### Indexing

#### Sets and Ways

Because of this optimization, caches are organized in 2 dimensional array
of blocks: the rows are called __sets__ and the columns
are called __ways__.

A cache with only one __set__ is a __fully-associative__ cache.

A cache with only one __column__ is a __direct-mapped__ cache.

Any other configuration is called a __N-way set-associative__ cache,
where __N__ is the number of __ways__. The number of sets is not specified
in the name, but can be deduced given the total cache capacity.

Given an address, we find where we will store it and its data in the
cache by dividing the address and getting the __set__, __way__ and
__offset__ from it:

```
<--- address bits --->
.--------------------.
| TAG | SET | OFFSET |
`-----'-----'--------´
```

* __TAG__ - Identify a block inside a __SET__. Each __TAG__ is
unique to the __SET__ it is in. __TAGS__ inside a __SET__ are
not ordered.

* __SET__ - Used to find the __set__ the address belongs to. Just
do a modulo operation with these bits (or simply get the least
significant bits in this field, same thing) to know what __SET__
an address belongs to.

* __OFFSET__ - Offset of an address within a cache block. Addresses which
the only difference is the __OFFSET__ will all be mapped to the same
block (close addresses get mapped to the same block). A consequence of
this is that every block in the lower layer maps to a single block in the
upper layer.

The size of each field depends on the number of __ways__ and __sets__.

And please, be aware that we don't index a specific address, but a block
that can contain more than one address:

![Cache Block](https://image.slidesharecdn.com/1ejajwdaraaebsb0dxko-signature-c9cec41e9de1df0e70d844d44f050ec324e45468089694cb239d95836fa2ca72-poli-160427174743/95/memory-caching-22-638.jpg?cb=1461779382)

### Flags

Each block has `valid` and `modify` flags:

* __Valid__ - Tells if that block is actually being used (or is just
garbage values and the space is free to use).

* __Modify__ - Tells if the block values are different from the
same blocks in lower layers.

## Optimizations

### Early Start

When fetching a block, data is fecthed in normal order
(sequentially). As soon as the requested data is arrives, it is
sent to the processor.

The processor can start execution using the data while the rest
of the block is being loaded.

### Critical Word First

The data the processor needs is fetched directly from memory.

Once the processor has the data, the cache starts fetching the
block.

## Policies

## Write Policy

When the processor write a new value to somewhere in a block to a
cache, there are two strategies that are used:

* __Write-through__ - Immediatly update lower layers with the
new value.

* __Write-back__ - Update lower layers when the updated block
is evicted.

Both strategies use the write buffer.

## Replacement Policy

When a __cache miss__ occur and a new block is brought to the
cache, the __set__ it belongs to may be full.

In this case, we need to choose a __tag__ inside to evict, in
order to make space for the new block.

Below are some policies that can be used to make that choice,
mind that they are usually implemented at the hardware level
(comes with the memory itself).

* __Least Recently Usage (LRU)__ - Choose the least recently
used blocks first. Keeping track of each block usage is
required as an additional mechanism.

* __First In First Out(FIFO)__ - The first block that entered
the __set__ is discarded. Keeping track of what are the older
blocks compared to others is required.

* __Random Replacement(RR)__ - Just discard a random block.
