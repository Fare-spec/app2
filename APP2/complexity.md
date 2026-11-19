# Complexity

The interpreter runs in **O(L + E)** time, where `L` is the program length and `E` the number of executed instructions. Most instructions are O(1), except `R`, which is O(k) for k rotated values.

Memory usage is **O(L + S + F)**, where `S` is the maximum stack size and `F` the maximum routine depth.

All performance tests show **linear time complexity**. Memory usage is also linear, except for `memfree`, which uses O(log n) memory.

The implementation uses contiguous allocations and an explicit frame stack to reduce memory overhead and avoid recursive calls.
