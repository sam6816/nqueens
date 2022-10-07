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

In search of more speed I tried a bit based version: `bittable.c`. It uses a lookup, and is not 
faster than the cumulated threats above.



