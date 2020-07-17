# Heap

sources:
http://wiki.inf.ufpr.br/maziero/lib/exe/fetch.php?media=socm:socm-livro.pdf \
https://www.youtube.com/watch?v=HPDBOhiKaD8 \
https://www.blackhat.com/presentations/bh-usa-07/Ferguson/Whitepaper/bh-usa-07-ferguson-WP.pdf \
https://heap-exploitation.dhavalkapil.com/ \
https://stackoverflow.com/questions/12825148/what-is-the-meaning-of-the-term-arena-in-relation-to-memory \
https://www.youtube.com/watch?v=HWhzH--89UQ

dependencies:
[Processes](processes.md)

As we saw before, the heap is just a part of the
memory given to a process in which we can dynamically allocate
and deallocate parts of it. 

In some places you may also see the term __arena__ used to refer to it, which means:

```
"large, contiguous piece of memory that you allocate once and
then use to manage memory manually by handing out parts of that memory"
```

The way the heap works is determined by system libraries, like `glibc`.

The management of the heap in GNU/Linux is done by the _ptmalloc2_
allocator, and it is the one where are going to focus in this section,
although some general concepts about the heap will also be presented.

## Layout, Names and Lists

Here we have the basic scheme of a heap:

```
|--------------------------------|
| 100 Mb | 100 Mb(free) | 200 Mb |
|--------------------------------|
```

As you can see the first 100 Mb and last 200 Mb are allocated,
while the 100 Mb at the center are free. We call each of these
spaces __chunks__. To manage the heap we need to be aware of every
chunk: where it is, its size, if it is freed, etc.

To do so efficiently, we store every __free__ chunk in a __bin__,
which are singly or doubly-linked circular lists with
representations of chunks. Mind that when i say we store
__free__ chunks in a bin, it means every chunk that has not
been allocated yet __or__ have been deallocated (anything
that is not an alocated chunk).

Also, keep in mind that there are many different __bins__ of
different types.

There is also metadata in the heap for each chunk, so we can
traverse through the all the chunks in memory (without a pointer,
only knowing their sizes). This metadata end up working like a
doubly linked list.

## Fragmentation

To understand why the heap works the way it works, first we
need to understand the problems he tries to solve.

Let's say we have 1 Gb of space:

```
|-----------------------------------------------------------------------|
|                                1 GB(free)                            |
|-----------------------------------------------------------------------|
```

We will later see how the algorithm chooses where to get memory
to allocate, but for now just imagine we ask it to allocate
200 Mb, and it allocates the memory from the beginning of the heap:

```
|-----------------------------------------------------------------------|
| 200 Mb |                       800 Mb(free)                          |
|-----------------------------------------------------------------------|
```

Then we ask for 300 Mb, then 200 Mb, then 100 Mb, and then 100 Mb again:

```
|-----------------------------------------------------------------------|
| 200 Mb | 300 Mb | 200 Mb | 100 Mb | 100 Mb |      100 Mb(free)        |
|-----------------------------------------------------------------------|
```

And now we call `free` for the first 100 Mb we asked for and 300 Mb:

```
|-----------------------------------------------------------------------|
| 200 Mb | 300 Mb(free) | 200 Mb | 100 Mb(free) | 100 Mb | 100 Mb(free) |
|-----------------------------------------------------------------------|
```

Currently, we have 500 Mb of memory available for allocation, but we
can only allocate at most 300 Mb for one allocation request! You can
imagine how this can get even worse as we continue. This is the
problem of __Fragmentation__, or, more specifically
__Exterior Fragmentation__.

This problem affects dynamic memory in general, not just the heap.

As we keep going, the __bins__ lists get larger and larger, making
heap operations more complex.

How we tackle this problem? There are two ways:

* minimize the occurance of fragmentation with [allocation strategies](#allocation-strategies)

* periodically do an [internal fragmentation](#internal-fragmentation)

Obs.: Actually there is a third way called _Defragmentation_,
but it only works in physical memories, not virtual ones like
the heap.

### Internal Fragmentation

The idea is to "round up" some allocated chunks so they take more
memory, in order to completely eliminate a free chunk.

As an example, check out this memory:

```
|-------------------------------|
| 100 Mb | 1 Mb (free) | 200 Mb |
|-------------------------------|
```

If we do some internal fragmentation, we can get rid of the
free 1 Mb simply by giving more memory to the allocated
chunk that comes before it:

```
|-------------------------------|
|     101 Mb     |    200 Mb    |
|-------------------------------|
```

However, memories like the heap uses _pages_. That
means that addresses returned by allocations are multiples
of the page size (usually 4096, which is 0x1000). Due to
this, the allocations will have some significant distance
between them.

You could argue that we could use smaller page sizes, but
that result in greater page tables to keep track of them,
making their management less efficient.

### Allocation Strategies

The idea is to minimize the fragmentation by finding a free area of
memory that best fits an allocation request. This analysis can be done
through the following methods:

* __First-Fit__ - choose the first free area with enough space for the
request.

* __Best-Fit__ - choose the smallest free chunk possible in which the
request fits.

* __Worst-Fit__ - choose the biggest free chunk possible, in the hopes
that the "leftovers" will be enough for future allocations.

* __Next-Fit__ - iterate the list of chunks from the last
allocated/deallocated chunk, and from there implement __First-Fit__.

Research has found that the best policy is __First-Fit__, the second
one being __Best-Fit__ (with a significant distance between the two).

## Metadata In The Heap Itself

### Allocated Chunk

When we ask `malloc` for some memory, it also includes some metadata
just before the returned address:

```
chunk -> .--------------------------------------------------.
	 | size of previous chunk, if allocated             |
	 |--------------------------------------------------|
	 | size of chunk, in bytes          | A | M (0) | P |
mem ---> |--------------------------------------------------|
	 |      User data start here ...                    |
	 `--------------------------------------------------´
```

`mem` is the address returned to us by `malloc`, but the allocated
chunk actually starts at `chunk`. This metadata is there to
allow to find the allocated chunks.

The size of the previous chunk, and the size of the current chunk
are both `size_t`, which is usually 4 bytes for each.

As you can see, the three least significant bits in the size
fields have special meaning:

* __A (NON_MAIN_AREA)__ - 0 for chunks in the main area. Each thread
spawned receives its own arena, and for those chunks this bit is set.

* __M (IS_MMAPED)__ - the chunk was obtained through `mmap`. If set,
the other two bits are ignored, since mmaped chunks are neither in
the arena, not adjacent to a free chunk. This bit will always be
zero for the current chunk since it is clearly in the heap.

* __P (PREV_INUSE)__ - 0 when previous chunk (not the previous one
in the linked list, but the one directly before it in memory) is
free (and hence the size of previous chunk is stored in the first
field). The very first chunk allocated has this bit set. If it is
1, then we cannot determine the size of the previous chunk.

### Free Chunk

When we call `free` on an address `mem` that was previously allocated
this is how it looks afterwards:

```
chunk -> .--------------------------------------------------.
	 | size of previous chunk, if allocated             |
	 |--------------------------------------------------|
	 | size of chunk, in bytes              | A | M | P |
mem ---> |--------------------------------------------------|
	 | forward pointer to next chunk in list            |
	 |--------------------------------------------------|
	 |back pointer to previous chunk in list            |
	 |--------------------------------------------------|
	 |      unused space (may be 0 bytes long)          |
	 `--------------------------------------------------´
```

We have two new fields. The pointers here point to somewhere
in the __bin__ that this free chunk belongs to. Mind that
when we free a chunk, it is coalesced with any free chunks it
has borders with.

## Data Structures

`glibc` has some structs used in the heap management:

### `_heap_info`

Only one per heap.

```
typedef struct _heap_info {

	mstate ar_ptr; // arena for this heap
	struct _heap_info* prev; //previous heap
	size_t size; // current size in bytes
	char pad[-5 * SIZE_SZ & MALLOC_ALIGN_MASK]; //for alignment
} heap_info;
```

Next, we can see `mstate`, that is the type of `ar_ptr`;

### `malloc_state` or `mstate`

```
struct malloc_state {

	mutex_t mutex; // ensure synchronized access
	int flags; // flags about current arena, is there fastbins? is memory non-contiguous? etc

#if THREAD_STATS
	// statistics for locking
	long stat_lock_direct, stat_lock_loop, stat_lock_wait;
#endif

	// array of fastbins, their operations are quicker
	// in large part due to less operations being performed on
	// them
	mfastbinptr fastbins[NFASTBINS];

	// special chunk of memory that border the end of available
	// memory. It is not in any bin
	mchunkptr top;

	// used when a small request for a chunk does not fit
	// exactly into any given chunk of memory
	mchunkptr last_remainder;

	// like fastbins, but for larger 'normal' chunks
	mchunkptr bins[NBINS * 2];

	// used as an one-level index to figure out faster if
	// a given bin is definitely empty. This speeds up
	// traversals by alowing the allocator to skip over
	// confirmed empty bins
	unsigned int binmap[BINMAPSIZE];

	// points to next area. Circular linked list
	struct malloc_state* next;

	// used to track amount of memory currently allocated by the
	// system. The INTERNAL_SIZE_T is size_t in most plataforms
	INTERNAL_SIZE_T system_mem;
	INTERNAL_SIZE_T max_system_mem;
};
```

### `malloc_chunk`

This one is a pretty big deal. Each chunk has a corresponding
`malloc_chunk` to represent it in a bin.

Mind that, to reflect that the heap metadata is different for
free and allocated chunks, some fields are only used when the
chunk is free:

```
struct malloc_chunk {

	INTERNAL_SIZE_T mchunk_prev_size; // size of previous chunk (if free)
	INTERNAL_SIZE_T mchunk_size; // size in bytes , including overhead

	struct malloc_chunk* fd; // double links -- used only if free
	struct malloc_chunk* bk;

	// Only used for large blocks: pointer to next larger size
	struct malloc_chunk* fd_nextsize; // double links -- used only if free
	struct malloc_chunk* fd_prevsize;
};
```

## Bins

As we states before, each chunk is stored in a __bin__. There are 4
types of __bins__:

Name | Linking | Management | Order | Chunk Sizes (not including metadata) | Number of Bins | Speed | Caviats |
-----|---------|------------|-------|--------------------------------------|------------------|-------|---------|
Fast bins | single | LIFO | Each bin has chunks of same size | 0, 12, 20, ... 80 | 10 | Fastest | No two contiguous free chunks here coalesce together (trading speed for fragmentation).  |
Unsorted bin | double | FIFO | No order | any | 1 | | kind like a "cache" bin. Freed chunks are sent here to then be send to the right bin | 
Small bins | double | FIFO | Each bin has chunks of same size | lower than 500 bytes | 62 | Faster than Large bins, slower than Fast bins | |
Large bins | double | FIFO | Decreasing | between 500 bytes and 128Kb | 63 | Slowest | Chunks bigger than 128Kb are served by mmap | 

obs.: Small and Large bins are the ones that may be coalesced together when freed.

### Special Bins

These chunks don't belong to any bin:

* __Top Chunk__ - It is the chunk which borders the top of an arena. While
servicing `malloc` it is used as a last resort. It can grow with the
heap itself by using the `sbrk` system call. The `P` (PREV_INUSE) flag is
always set for it.

* __Last Remainder Chunk__ - It is the chunk obtained from the last
split. Sometimes, when exact size chunks are not available , bigger chunks
are split into two. One part is returned to the user and the other becomes
the last remainder chunk.

## Heap Operations

### Initialization

Heap initialization occurs the first time an allocation is requested. This
is when the heap is created, almost always prior to any call to `malloc`
from the developer, due to process/application initialization step, where
`libc` is called some times to allocate memory.
