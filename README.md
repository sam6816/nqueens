### A collection of N-Queens counting programs in C

After I had experimented with backtracking I wrote a first N-Queens program. That basic 
version is `backtrack_goto.c`. It uses -1 to indicate a fresh empty row.

`threats_cumul.c` is more than twice as fast; it uses a 2D array to keep the current 
threats up to date. As a side effect, these overlapping threats can be printed:

```

      #        
    #...       
   .o.. . #    
  ... o  o.. # 
 ..#. ... o o..
 ...o ... .o o#
 . ...o# .o .oo
 # ...O...o .o.
 ...o..o...o#.o
 .#Oo .o. O.oo.
 ooOo#o. oo..oo
-oo.OOo...o o.o
 o.ooooO. ...o.
 .oo..OO. . o.o

120009       

```

Starting at the top, the hashes are the queens, sending dots as threats. Small and big Os 
indicate overlapping threats. The line shows on which row a queen is sought. This is with
N = 14 and 120000 skipped solutions, otherwise the top queens are all on the left side.
It looks quite interesting when running - the timing has to be fine-tuned, because the last 
rows change very fast, while the first (top) rows hardly move,

In search of more speed I tried a bit based version: `bittable.c`. It uses a lookup, but is not 
faster than the cumulated threats above.

I finally found a very nice "simple" version (as he calls it) be Kenji Kise. The trick is to 
split the current threats into three groups: straight, left and right diagonal. The diagonals are propagated
by bit-shifting. Plus you have to pick the queen from the candidate bits: `x AND -x` should ideally 
generate a `BLSI - Extract Lowest Set Isolated Bit` instruction.

This makes it 5x faster. There are some subtleties, and I made a `kise_up.c` with explicit backtracking, 
and a more minimalistic `kise_noup.c`.   

A recursive nqueens from `computerpuzzle.net` turned out to be the fastest way to make use of the bit operations. 
My version of it is `recbits_puzzle.c`. 

## Breadth First

After a lot of testing with gcc and clang I then came to the conclusion that it is the backtracking itself that 
is limiting the speed. There is just too much jumping up and down the rows. So I tried a BFS (breadth first search)
approach. 

A simple way to deal with the large numbers is to split the solutions after three rows and restart and count each one separately.
But I wanted a smoother and automatic segmentation, so I made the generating loop return when the solutions grow too big.

So I ended up with `bits_bfs_segm_omp.c`, which is ~30% faster than versions that do not need any memory (that do not use the cache).  

To even use multithreading, one row is generated manually. It should be at least two, because for N16, only 8 first solutions
are needed, which in my case is also the number of threads. The edge queens have less solutions than middle ones...some cores finish and 
have to wait idle. 

This is the output for N=16 with 4 threads:


```

[0]  0.  fffe     2     0 -->   436228
[3]  1.  fffd     4     1 -->   569531
[1]  2.  fffb     8     2 -->   736363
[2]  3.  fff7    10     4 -->   892999
[0]  4.  ffef    20     8 -->  1050762
[3]  5.  ffdf    40    10 -->  1160280
[1]  6.  ffbf    80    20 -->  1249262
[2]  7.  ff7f   100    40 -->  1290831
N16 --> 14772512

real    0m0.724s
user    0m2.723s
sys     0m0.070s

```

OK, this finally does only half of the work, so careful when comparing directly. I should not have 
done that symmetry halving trick, just to prove that one single pre-generation does not give fine 
enough thread loads...

The `perf` characteristics of my `bits_bfs_segm_omp.c` (vs. recursive) are:

- many page faults (but also cache hits) vs. 0 
- up to 2.0 instructions/cycle           vs. 1.2   
- 9% branch misses                       vs. 23%   





 
































  










 












 

 








