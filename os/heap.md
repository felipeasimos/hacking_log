# Heap

sources:
http://wiki.inf.ufpr.br/maziero/lib/exe/fetch.php?media=socm:socm-livro.pdf \
https://www.youtube.com/watch?v=HPDBOhiKaD8 \
https://www.blackhat.com/presentations/bh-usa-07/Ferguson/Whitepaper/bh-usa-07-ferguson-WP.pdf \
https://heap-exploitation.dhavalkapil.com/ \
https://stackoverflow.com/questions/12825148/what-is-the-meaning-of-the-term-arena-in-relation-to-memory \
https://www.youtube.com/watch?v=HWhzH--89UQ
https://www.youtube.com/watch?v=kVxmvaSliu8
https://www.youtube.com/watch?v=VLnhV1T5Ng4
https://www.youtube.com/watch?v=z33CYcMf2ug&t=68s
https://sourceware.org/glibc/wiki/MallocInternals%C2%A0
https://dangokyo.me/2017/12/05/introduction-on-ptmalloc-part1/
https://www.blackhat.com/presentations/bh-usa-07/Ferguson/Presentation/bh-usa-07-ferguson.pdf

dependencies:
[Processes](processes.md)

As we saw before, the heap is just a part of the
memory given to a process in which we can dynamically allocate
and deallocate parts of it. 

In some places you may also see the term __arena__ used with it, which means:

```
A structure that is shared among one or more threads which contains
references to one or more heaps, as well as linked lists of chunks within those heaps which are "free".
Threads assigned to each arena will allocate memory from that arena's free lists. 
```

The way the heap works is determined by system libraries, like `glibc`.

We often say _the_ heap, as if is only possible to be one by process,
when in fact you can actually have more than one. For simplicity
and learning purposes, we can think of it being only one heap
without much problem though.

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

There is also metadata for each chunk, so we can traverse
through the all the chunks in memory (without a pointer,
only knowing their sizes). This metadata end up working like a
doubly linked list.

Also, be aware that adjacent chunks don't necessarily belong to
the same __bin__ (usually they don't actually).

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
	 | size of chunk, in bytes          	| A | M | P |
mem ---> |--------------------------------------------------|
	 |      User data start here ...                    |
	 `--------------------------------------------------´
```

`mem` is the address returned to us by `malloc`, but the allocated
chunk actually starts at `chunk`. This metadata is there to
allow to find the allocated chunks.

The size of the previous chunk, and the size of the current chunk
are both `size_t`, which is usually 4 bytes for each.

Since all chunks are multiple of 8 bytes, the last three bits of
the size field can be reserved, and carry special meaning:

* __A (NON_MAIN_AREA)__ - 0 for chunks in the main area. Each thread
spawned receives its own arena, and for those chunks this bit is set.

* __M (IS_MMAPED)__ - the chunk was obtained through `mmap`. If set,
the other two bits are ignored, since mmaped chunks are neither in
the arena, not adjacent to a free chunk.

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

If the chunk is a large chunk and it is free, we also have
two pointers, for the next and previous large chunks.

## Data Structures

`glibc` has some structs used in the heap management. They are saved in the heap itself
(like the metadata we saw before!) and appear in it in the same order as they are
presented here:

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
Fast bins | single | LIFO | Each bin has chunks of same size | 0, 12, 20, ... 80 (or 60) | 10 | Fastest | No two contiguous free chunks here coalesce together, because they do not hold size of previous chunk (trading speed for fragmentation, fastbin is the only one that does this). Next chunk's `P` is on (always considered in use).|
Unsorted bin | double | FIFO | No order | any | 1 | | kind like a "cache" bin. Freed chunks are sent here to then be send to large or small bin | 
Small bins | double | FIFO | Each bin has chunks of same size | lower than 500 bytes | 62 | Faster than Large bins, slower than Fast bins | |
Large bins | double | FIFO | Decreasing | lower than 128Kb | 63 | Slowest | Chunks bigger than 128Kb are served by mmap | 

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

During initialization, small and large bins are empty.

### Creation

The heap creation is implicitly triggered when calling an allocation
function under certain conditions:

1. No heap exists

2. Called immediatly followed by subsystem initialization

### Allocation

When calling `malloc`, this is the path we take:

```
malloc(size_t bytes) -> __libc_malloc(size_t bytes)
			|           |        
			|           |        
			v           v        
	   arena_get(ar_ptr, size)  _int_malloc(mstate av, size_t bytes)
	                            |                     |
				    |                     |
				    v                     v
			alloc_perturb(char* p, size_t n)  malloc_consolidate(mstate av)--.
							  |                              |
							  |                              |
							  v                              v
							  malloc_init_state(mstate av)   unlink(AV, P, BK, FD)
```

#### `__libc_malloc(size_t bytes)`

1. Calls `arena_get` to get `mstate` pointer.

2. Calls `_int_malloc` with the pointer and the size.

3. Unlocks the arena.

4. Before returning pointer to chunk, one of these must be true:

	* Returned pointer is NULL.

	* Chunk is MMAPPED.

	* Arena for chunk is the same for the one found at step 1.

#### `arena_get(ar_ptr, size)`

1. Acquires an arena and locks the corresponding mutex.

2. `ar_ptr` is set to the corresponding arena.

3. `size` is just a hint as to how much memory will be required
immediately.

#### `_int_malloc(mstate av, size_t bytes)`

1. Update `bytes` to handle alignments.

2. Checks if `av` is NULL of not.

3. If `av` is NULL (absance of usable arena), calls `sysmalloc`
to obtain chunk using mmap. If sucessful, calls `alloc_perturb`.
Returns the pointer.

4. Otherwise:

* If size falls in fastbin range:

	1. Get index into the fastbin array to access an
	appropriate bin according to request size.

	2. Removes first chunk from that list (LIFO) and
	assign `victim` pointer to it.

	3. If `victim` is NULL, move on to next case (smallbin)

	4. If `victim` is not NULL, do a security check to ensure
	the size belongs to this bin ("malloc(): memory corruption (fast)").

	5. Calls `alloc_perturb`.

	6. Return `victim`.

* If size falls in the smallbin range:

	1. Get index into the samllbin array to access an
	appropriate bin according to the requested size.

	2. If there are no chunks in this bin (check `bin` and `bin->bk`),
	move on to the next case (largebin).

	3. `victim = bin->bk` (last chunk in the bin).

	4. If `victim` is NULL (happens during initialization), call
	`malloc_consolidate` and skip this complete step of checking
	into different bins.

	5. If `victim` is not NULL, check `victim->bk->fd == victim`
	("malloc(): smallbin double linked list corrupted").

	6. Sets the `P` bit (PREV\_INUSE) bit for the next chunk (in
	memory, not list).

	7. Remove `victim` from list.

	8. Set appropriate arena bit for this chunk depending on `av`.

	9. Calls `alloc_perturb` and then return the pointer.

* If size falls in the largebin range:

	1. Get index into the largebin array to access an appropriate
	bin according to the requested size.

	2. If `av` has fastchunks (check FASTCHUNKS\_BIT in `av->flags`)
	call `malloc_consolidate` on `av`.

5. If no pointer has yet been returned, this mean one or more of these
statements is true:

	* Size falls into fastbin range but no fastchunk is available

	* Size falls into smallbin range but no smallchunk is available
	(calls `malloc_consolidate` during initialization)

	* Size falls into largebin range

6. Unsorted chunks are checked and traversed chunks are place into
bins (only step where chunks are placed into bins). Traversal happens
from the tail, until end of unsorted bin is reached or we went
through MAX\_ITERS chunks (10000). Each iteration looks like so:

	1. `victim` points to the current chunk being considered (tail)

	2. Check if `victim`'s chunk size is within minimum (2\*SIZE\_SZ) and
	maximum (`av->system_mem`) range ("malloc(): memory corruption").

	3. If requested size belongs to smallbin, `victim` is the
	`last_remainder`, `victim` is the only chunk in unsorted bin and
	`victim` size is equal or greater to the one requested, break
	the chunk into 2:

		* The first chunk matches the requested size and is returned
		(using `alloc_perturb`).

		* Left over chunk becomes the new `last_remainder` and is
		inserted back into unsorted bin.

	4. Else, remove `victim` from unsorted bin.
	
	5. If the size of `victim` match the one requested exactly,
	returns chunk after calling `alloc_perturb`.

	6. If `victim` size belongs to smallbin, add chunk to
	appropriate smallbin at the head.

	7. Else insert into appropriate largebin while maintaing
	sorted order.

7. Check if requested size does not fall in the smallbin range,
if so then check largebins.

	1. Get index into largebin array to access an appropriate
	bin according to the requested size.

	2. Point `victim` to the smallest chunk with enough size
	for the request.

	3. Remove `victim` from the bin (`unlink`).

	4. Split `victim` (to have a chunk that matches request)
	and put left over chunk in the unsorted bin.

	5. Call `alloc_perturb` and return `victim`.

8. Until now, everytime we check in the fastbins and smallbins we
do so by going directly to the bin with the __exact__ chunk size
as the one requested. Let's try finding chunks bigger than the
one requested! Repeat these steps until all bins are exhausted:

	1. The index is incremented to check the next bin.

	2. Use `av->binmap` to skip over empty bins.

	3. Point `victim` to tail of current bin.

	4. `av->binmap` ensure that __definitely empty__ bins
	are skipped, but does ensure that __all empty__ bins
	are skipped. Because of that, we need to check if
	`victim` is empty (skip to next iteration if it is).

	5. When at a non-empty bin where the chunk size is
	greater than the one requested (continue to next
	iteration if it isn't):

		* Split the chunk into 2: request size chunk
		and left over.

		* Send left over to unsorted bin (to its tail,
		to ensure we pass the check
		`unsorted_chunks(av)->fd->bk == unsorted_chunks(av)`,
		which gives us "malloc(): corrupted unsorted chunks 2")

		* Call `alloc_perturb` and return `victim`.

9. If still no chunk is found, 'top' chunk will be used to service the
request:

	1. `victim` points to `av->top`

	2. If top chunk is greater than the requested size + MINSIZE,
	split it into two chunks: remainder will be the new top
	chunk and the other is returned after calling `alloc_perturb`.

	3. See if `av` has fastchunks or not (check FASTBINS\_BIT in
	`av->flags`), if so, call `malloc_consolidate` on `av` and 
	return to step 6.

	4. If `av` doesn't have fastchunks, call `sysmalloc` (get mmap
	to give us a chunk) and return obtained pointer after calling
	`alloc_perturb`.

#### `alloc_perturb(char* p, size_t n)`

If `perturb_byte` (tunable parameter for `malloc` using M\_PERTURB)
is non-zero (by default it is 0), sets the `n` bytes pointed to by
`p` to be equal to `perturb_byte` ^ 0xff.

#### `malloc_consolidate(mstate av)`

It is like an specialized free.

1. Check if `global_max_fast` is 0 (`av` not initialized) or not.

2. If it is 0, call `malloc_init_state` with `av` as parameter and return

3. If it is not 0, clear FASTCHUNKS\_BIT for `av`

4. Iterate through fastbin array from first to last indices:

	1. Get lock on the current fastbin chunk and 
	stop if it is NULL.

	2. If previous chunk (by memory) is not in use, call
	`unlink` on the previous chunk.

	3. If next chunk (by memory) is not top chunk:

		* If next chunk is not in use, call `unlink`
		on it.

		* Merge the chunk with the previous if free and next
		if free (by memory). Add consolidated chunk to the
		head of unsorted bin.

	4. If next chunk (by memory) was top chunk, merge the chunks
	appropriatly into a single top chunk.

Obs.: The `P` bit (PREV\_IN\_USE) is used to check. Hence, fastbin chunks
won't identify their previous chunk as free if it is a fastbin chunk (but
they will at some point be identified since we are going through all fastbins).

#### `malloc_init_state(mstate av)`

Only called from `malloc_consolidate`.

1. For non fast bins, create empty circular linked lists for each bin

2. Set FASTCHUNKS_BIT flag for `av`.

3. Initialize `av->top` to the first unsorted chunk.

### Deallocation (Free)

When calling `free` this is the path we take:

```
free -> __libc_free(void* mem)
	   |                |
	   |                |
	   v                v
	munmap_chunk     _int_free(mstate av, mchunkptr p, int have_lock)
			   |
		           |
			   v
			free_perturb(char* p, size_t n)
```

#### `__libc_free(void* mem)`

1. Return if `mem` is NULL

2. If chunk is mmapped and the dynamic brk/mmap threshold needs adjusting,
call `munmap_chunk`.

3. Get arena pointer for the chunk.

4. Call `_int_free`

#### `_int_free(mstate av, mchunkptr p, int have_lock)`

1. Check wheter `p` is before `p + chunksize(p)` in the memory
(to avoid wrapping) ("free(): invalid pointer").

2. Check wheter the chunk is at least of size MINSIZE or a multiple of
MALLOC\_ALIGNMENT ("free(): invalid pointer").

3. If the chunk size falls in fastbin list:

	1. Check if next chunk size is between minimum and
	maximum size (`av->system_mem`) ("free(): invalid next size (fast)")

	2. Calls `free_perturb` on the chunk.

	3. set FASTCHUNKS\_BIT for `av`.

	4. Get index into fastbin array according to chunk size.

	5. Check if the top of the bin is not the chunk we are going to add ("double free or corruption (fasttop)")

	6. Check if the size of the fastbin chunk at the top is the same as the chunk we are adding ("invalid fastbin entry (free)").

	7. Insert the chunk at the top of the fastbin list and return.

4. If the chunk is not mmapped:

	1. Check if the chunk is the to chunk ("double free or corruption (top)").

	2. Check wheter next chunk (by memory) is within the arena boundaries ("double free or corruption (out)").

	3. Check wheter next chunk's (by memory) `P` bit is marked or not. ("double free or corruption (!prev)).

	4. Check wheter the size of the next chunk is between the minimun and the maximum size (`av->system_mem`)
	("free(): invalid next size (normal)").

	5. Call `free_perturb` on the chunk.

	6. If previous chunk (by memory) is not in use, call unlink on the previous chunk.

	7. If next chunk (by memory) is not top chunk:

		* If next chunk is not in use, call unlink on next chunk

		* Merge the chunk with the previous if free and next
		if free (by memory). Add consolidated chunk to the
		head of unsorted bin. Before insertion check
		`unsorted_chunks(av)->fd->bk == unsorted_chunks(av)`
		("free(): corrupted unsorted chunks")

		* If next chunk (by memory) was a top chunk, merge the chunks
		appropriately into a single top chunk.

5. If the chunk was mmapped, call `munmap_chunk`.

#### `free_perturb(char* p, size_t n)`

If `perturb_byte` (tunable parameter for malloc using M\_PERTURB) is non-zero (by default is 0), sets
the `n` bytes pointed to by `p` to be equal to `perturb_byte`.
