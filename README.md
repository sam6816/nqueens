## A collection of N-Queens counting programs in C

After I had experimented with backtracking I wrote a first N-Queens program.
That basic version is `backtrack_goto.c`. It uses -1 to indicate a fresh empty
row. 

`threats_cumul.c` is more than twice as fast; it uses a 2D array to keep the
current threats up to date. As a side effect, these overlapping threats can be
printed:

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

Starting at the top, the `#` are the queens, sending dots as threats. Small and
big `O`s indicate overlapping threats. The line shows on which row a queen is
sought. This is with N = 14 and 120000 skipped solutions, otherwise the top
queens are all on the left side.  It looks quite interesting when running - the
timing has to be fine-tuned, because the last rows change very fast, while the
first (top) rows hardly do. 

In search of more speed I tried a bit based version: `bittable.c`. It uses a
lookup, but is not faster than the cumulated threats above.

I finally found a very nice "simple" version (as he calls it) be Kenji Kise, as
part of some benchmark. The trick is to split the current threats into three
groups: straight, left and right diagonal. The diagonals are propagated by
bit-shifting. Plus you have to pick the queen from the candidate bits: `x AND
-x` should ideally generate a `BLSI` (Extract Lowest Set Isolated Bit)
instruction. 

This makes it 5x faster. There are some subtleties, and I made a `kise_up.c`
with explicit backtracking, and a more minimalistic `kise_noup.c` like his
original.   

A *recursive* nqueens from _computerpuzzle.net_ turned out to be the fastest
way to make use of the bit operations.  My version of it is `recbits_puzzle.c`. 

Here are the timings for N14. Besides `-O3`, `-march=skylake` is also
important, to get above mentioned `blsi` or the `andn` combination of NOT and
AND. 

   
||ms
|----------|-------------
|backtrack | 310
|kise      | 280 / 260   
|recursive | 230 / 215  
|bfs       | 150   



#### Breadth First, segmented 

After a lot of testing with gcc and clang I then came to the conclusion that it
is the backtracking itself that limits the speed. There is just too much
jumping up and down the rows. So I tried a BFS (breadth first search) approach. 

A simple way to deal with the large numbers is to split the solutions after
three rows and restart and count each one separately. This makes N14 efficient
and N16 possible.  But I wanted a smoother and automatic segmentation, so I
made the generating loop return on demand, when the solutions grow too big.

I ended up with `bits_bfs_segm_omp.c`, which is ~30% faster than versions that
do not need any memory (that do not use the cache).  

To even use multithreading, one row is generated manually. It should be at
least two, because for N16, only 8 (not 16) first solutions are needed, which
in my case is also the number of threads. The edge queens have less solutions
than middle ones...some cores finish and have to wait idle. 

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

OK, this finally does only half of the work, so careful when comparing
directly. I should not have done that symmetry halving trick, just to prove
that one single pre-generation does not give fine enough thread loads...

The `perf stat` characteristics of `bits_bfs_segm_omp.c` (vs. recursive) are:

- many page faults (but also cache hits) vs. practically none 
- 2.0 instructions/cycle                 vs. 1.2   
- 9% branch misses                       vs. 23%   


#### bits_bfs_segm_omp.c details

There are some info lines to uncomment to show the progress; these are the
first lines with N16 and 4 threads:

```
[3] WBLOC added at sp 0 nr  9 split:  53743/63 m 6.1 0x7fc16ac00010
[1] WBLOC added at sp 0 nr  9 split:  56596/65 m 6.1 0x7fc16ad15010
[0] WBLOC added at sp 0 nr  9 split:  60164/64 m 5.9 0x7fc16aaf9010
[2] WBLOC added at sp 0 nr  9 split:  59413/66 m 6.0 0x7fc16a9f7010
[3] WBLOC added at sp 1 nr  6 split:  43874/10 m 2.7 0x7fc16a83c010
[0] WBLOC added at sp 1 nr  6 split:  46227/11 m 2.8 0x7fc16a659010
[2] WBLOC added at sp 1 nr  6 split:  43162/ 9 m 2.6 0x7fc16a576010
[1] WBLOC added at sp 1 nr  6 split:  48559/10 m 2.7 0x7fc16a738010
[0] WBLOC added at sp 1 nr  6 split:  47258/11 m 2.8 0x560f52d379b0
[2] WBLOC added at sp 1 nr  6 split:  44677/ 9 m 2.6 0x7fc15c0620a0
...
```

`sp` is the stack pointer i.e. level, `nr` is the number of rows (on the
chessboard) left.  All threads surpass the defined `LIM` of 32000 with a
multiplier `m` around 6.0. So they all restart their batch with 1000-element
arrays with 9 rows ahead.

After 3 more rows again the limit is hit, but now the "chunk divisor" is set to
around 10, not 60, resulting in 4000-element arrays. The multipliers are lower
and soon will reach below 1.0. In fact I made a multiplier > 2.0 an additional
condition for a *split*. 

The faster the solutions are growing, the more they get chopped up, by `int
chdiv =  sz * multip * multip / LIM;`. Using the square of `multip` seems
perfect, but it does not matter too much whether you restart with 200 elements
or 5000. But the simple way of looping through a big block and restarting all
as a 1-element-arrays is suboptimal.  


##### Partial runs

There is a second parameter, `nruns`, which defaults to `N - 2` (one row is
pre-generated, the last one is not needed). Here is the output for the 1. to 4.
row; single threaded for correct ordering: 

```
[0]  0.  fffe     2     0 -->       14    <- side queen only takes away 2 fields, 14 of 16 are free
[0]  1.  fffd     4     1 -->       13
[0]  2.  fffb     8     2 -->       13
[0]  3.  fff7    10     4 -->       13
[0]  4.  ffef    20     8 -->       13
[0]  5.  ffdf    40    10 -->       13
[0]  6.  ffbf    80    20 -->       13
[0]  7.  ff7f   100    40 -->       13
N16 --> 210
[0]  0.  fffe     2     0 -->      157
[0]  1.  fffd     4     1 -->      146
[0]  2.  fffb     8     2 -->      135    
[0]  3.  fff7    10     4 -->      136
[0]  4.  ffef    20     8 -->      136
[0]  5.  ffdf    40    10 -->      136
[0]  6.  ffbf    80    20 -->      136
[0]  7.  ff7f   100    40 -->      136
N16 --> 2236
[0]  0.  fffe     2     0 -->     1406
[0]  1.  fffd     4     1 -->     1336
[0]  2.  fffb     8     2 -->     1249
[0]  3.  fff7    10     4 -->     1162
[0]  4.  ffef    20     8 -->     1168
[0]  5.  ffdf    40    10 -->     1175
[0]  6.  ffbf    80    20 -->     1174
[0]  7.  ff7f   100    40 -->     1174
N16 --> 19688
[0]  0.  fffe     2     0 -->    10246
[0]  1.  fffd     4     1 -->     9895
[0]  2.  fffb     8     2 -->     9331
[0]  3.  fff7    10     4 -->     8757
[0]  4.  ffef    20     8 -->     8123   <- min.
[0]  5.  ffdf    40    10 -->     8151
[0]  6.  ffbf    80    20 -->     8171
[0]  7.  ff7f   100    40 -->     8232
N16 --> 141812
```

This shows that the side queen(s) actually first take the lead: it pays off to
let *one* of the diagonal threats run off the board immediately.

Here after 9, 10 and 11 rows:

```
[0]  0.  fffe     2     0 -->  6010779
[0]  1.  fffd     4     1 -->  6403798
[0]  2.  fffb     8     2 -->  6588563     <<< max.
[0]  3.  fff7    10     4 -->  6452221
[0]  4.  ffef    20     8 -->  6266436
[0]  5.  ffdf    40    10 -->  6061189
[0]  6.  ffbf    80    20 -->  5916534
[0]  7.  ff7f   100    40 -->  5900596
N16 --> 99200232
[0]  0.  fffe     2     0 -->  8578176
[0]  1.  fffd     4     1 -->  9402105
[0]  2.  fffb     8     2 --> 10069220
[0]  3.  fff7    10     4 --> 10152666
[0]  4.  ffef    20     8 --> 10189809
[0]  5.  ffdf    40    10 --> 10259136
[0]  6.  ffbf    80    20 --> 10583987
[0]  7.  ff7f   100    40 --> 10621348    <<< max. 
N16 --> 159712894
[0]  0.  fffe     2     0 -->  8404688
[0]  1.  fffd     4     1 -->  9624396
[0]  2.  fffb     8     2 --> 10762647
[0]  3.  fff7    10     4 --> 11263761
[0]  4.  ffef    20     8 --> 12026200
[0]  5.  ffdf    40    10 --> 12596135
[0]  6.  ffbf    80    20 --> 13044679
[0]  7.  ff7f   100    40 --> 13188557    <<< abs. max. 
N16 --> 181822126
```

Suddenly there is a clear distribution favoring the center queens: now it pays
off to have *both* diagonals run off the board.

From here the 181.8 Mio solutions get to be reduced by ~12 over the remaining 5
rows. See final result for N16 above: 14.7 Mio.

### Conclusion

The segmented breadth-first approach is an extra layer for a backtracking
algorithm. It keeps the generations in sync as long as possible, or desired.
Instead of backtracking proper i.e. fetching a previous single state it works
more by *extinction*: a dead branch simply leaves no trace in the next
generation. 

With backtracking, also by recursion, every single new solution must ask: "Or
am I on the last row?". I believe the row-hopping also leads to all these
branch-misses (24% vs. 9%), because the probability for a queen depends very
much on the row. __builtin_expect() takes only constants. Branch Predictor is
efficient, but it works by statistics, not logic reasoning.  The recursive
version is only super fast with gcc doing very long **constprop**-inlining.  
