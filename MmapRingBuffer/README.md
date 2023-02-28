Windows 'Magic' RingBuffer implementation.
	
A ringbuffer will typically need to do some small mgmt to wrap around the indexes.
This isn't hard work but it makes it so the buffer will work poorly with api 
that taking an array and r/w a bunch of data at once (e.g. i/o streams)

The 'Magic' RingBuffer works by reserving 2 contiguous regions of virtual memory 
of equal size and remapping them so that they point to the same physical memory.
This way, overflow on the end of the first buffer will auto-magically 'wrap around' to 
the beginning.

IOW, for the following vmem layout : [AAAAAAAA|BBBBBBBB]  where a and b represent 2 different
regions contiguous to each other, A and B would each actually map to the same physical memory region.
As such, a write of the values VXYZ starting on the second to last block of region A would end up
being written in physical memory as such : [YZ....VX]

Usually, this is implemented on posix systems by using mmap. There are many examples and a lot of documentation on this subject.

Because I'm masochistic, this is an implementation using the winapi. 

VMemMirrorBuffer is a simple utility class to manage the bookkeeping of reserving the region, segmenting it, allocating 
the backing memory, and mapping the segments to the backing memory and, of course, freeing all of that when done.

Because of the way things are, the size of your buffer must be a whole multiple of system's page size.

I think maybe the ringbuffer should round up the number of elements you want to store so that it always work.

But it doesn't. Such is life.

Current impl only support trivial data types. I think it kind of make sense with the way the memory is managed.
Also there's a memcpy somewhere, so there's that.

The whole thing might be more usable if it were in a single header. Tough luck.
If someone would actually use it if that were the case just ping me and I'll do it. 

Until then, I wont.