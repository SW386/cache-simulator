# Cache Simulator
Implementation of a Cache for CS 250

This is an implementation of a simple write back and write allocate cache with
a least recently used replacement algorithm.

virt2phys contains the translation algorithm from virtual address to physical address. It requires two inputs, the path to a file containing the page table, and the virtual address to translate in hex

cachesim contains the cache lookup simulation. It requires 4 inputs, the trace file containing loads and stores, the cache size in kilobytes, associativity, and the block size of the cache.
