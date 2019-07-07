# Motivation
Str2D is a library of 2D algorithms and data structures implemented in c++17 designed for manipulating large amounts of data. 

While reading two books, `Elements of Programming`(which now you can read for [free](http://componentsprogramming.com/elements-of-programming-authors-edition/)) and `From mathematics to generic programming`, I stumbled upon a coordinate structure called the segmented iterator and realised I could implement it together with data structures and algorithms needed for its use.  
The second motivation was [this](https://www.google.com/url?sa=t&source=web&rct=j&url=https://people.freebsd.org/~lstewart/articles/cpumemory.pdf&ved=2ahUKEwirjajuv57jAhVrxKYKHbfvDV4QFjAAegQIAhAB&usg=AOvVaw3VY2lnCBaI-B57Dric65cb) paper, which explained to me the inadequacy of data structures which utilize numerous single node allocations(e.g. `std::set` and the like).

At the heart of the library lies a data structure called `str2d::seg::vector`, the rest are build on top of it; hence I'll only focus on it. Once you've understood how the segmented vector is implemented you'll be able to easily deduce how to use it to implement `set`-like and `map`-like data structures.

There are currently only `str2d::seg::multiset` and `str2d::seg::multimap` data structures in this library apart from the `str2d::seg::vector`. 
The reason for exlusion of `str2d::seg::set` and `str2d::seg::map` is the lack of time; they will probably be included some time later.

# Implementation
Segmented vector is not a difficult structure to imagine. In it, an `std::vector` is used as an index which holds segment headers, structures holding pointers to segments of memory and possibly some meta data(Detailed explaination og segment headers will be given bellow). Those segments are where the data is actually held. The capacity of every segment is constant; the size on the other hand can vary.
Each segment holds at least half the capacity("limit") elements on it, except the first one; it can hold as many(less than capacity) or as little(more than 0) as it needs.

## Insertion
If an element is inserted into a segment which isn't at full capacity all actions are confined to that segment(which makes the structure vary cache friendly), otherwise an allocation of new segments s to occur and/or rebalancing to neighbouring segments.
In the case than new allocations happen, new segment headers have to be inserted into the index. Once the index becomes large enough, the operation of inserting into the index starts to affect performance. 

## Erasure
If an element is erased from a segment which holds more than "limit" elements, all operations are confided to that segment; otherwise
a deallocation of the segment has to occur nad/or rebalancing to neighbouring segments.


## Lookup


# Usage

# Conclusion
In a sense, the segmented vector extends the application area of the "flat" vector. As benchmarks show, that extension has limits which have to be taken into account. 

Google's btree is probably a safe bet as a drop in replacement for the std map and set data structures. If on the other hand iterations dominate over other operations, or you constantly need to erase and insert entire ranges, you could consider using the segmented data structures. 

Needless to say, these opinions mean little in comparison to actual benchmarks of your code.
