# Motivation
Str2D is a library of 2D algorithms and data structures implemented in c++17 designed for manipulating large amounts of data. 

While reading two books, `Elements of Programming`(which now you can read for [free](http://componentsprogramming.com/elements-of-programming-authors-edition/)) and `From mathematics to generic programming`, I stumbled upon a coordinate structure called the segmented iterator and realised I could implement it together with data structures and algorithms needed for its use.  
The second motivation was [this](https://www.google.com/url?sa=t&source=web&rct=j&url=https://people.freebsd.org/~lstewart/articles/cpumemory.pdf&ved=2ahUKEwirjajuv57jAhVrxKYKHbfvDV4QFjAAegQIAhAB&usg=AOvVaw3VY2lnCBaI-B57Dric65cb) paper, which explained to me the inadequacy of data structures which utilize numerous single node allocations(e.g. `std::set` and the like).

At the heart of the library lies a data structure called `str2d::seg::vector`, the rest are build on top of it; hence I'll only focus on it. Once you've understood how the segmented vector is implemented you'll easily deduce how to use it to implement `set`-like and `map`-like data structures.

There are currently only `str2d::seg::multiset` and `str2d::seg::multimap` data structures in this library apart from the `str2d::seg::vector`. 
The reason for exlusion of `str2d::seg::set` and `str2d::seg::map` is the lack of time; they will probably be included some time later.

# Implementation and Usage
Segmented vector is not a difficult structure to imagine. 

In it, an `std::vector` is used as an index which holds segment headers, structures holding pointers to segments of memory and possibly some meta data(detailed explaination of segment headers will be given bellow). Those segments are where the data is actually held. The capacity of every segment is constant; the size on the other hand can vary.
Each segment holds at least half the capacity("limit") elements on it, except the first one; it can hold as many(less than capacity) or as little(more than 0) as it needs.

### Coordinate Structures/Iterators
Segmented vector utilizes 3 kinds of coordinate structures : 
1) Segment iterator - random access iterator that iterates over a range of segments. It can't be dereferenced like ordinary
random access iterators; data inside it is accessed like it's accessed in a sequence container(e.g. `std::vector`), i.e. by
using `begin` and `end` methods. Those methods of the segment iterator return a flat iterator.
   
2) Flat iterator - regular random access iterator; when dereferenced, returns the value type stored in the segmented vector.

3) Segmented coordinate - regular bidirectinal iterator; when dereferenced, returns the value type stored in the segmented vector.
This type is returned when `begin` and `end` functions of the segmented vector are called. Inside it holds a segment iterator and a flat iterator pointing inside that segment. It bassically works like the iterator of `std::deque`, except it's not random access. Its segment iterator is accessed through `segment` method, while its flat iterator is accessed via `flat` method of the coordinate.

The algorithms in the library are aware of these coordinate structures, and use them in nesteed loops to decrease the number of checks needed in each iterations. If only segmented coordinate(regular bidirectional iterator) were used, each iteration of an algorithm would have to check whether it's reached the end of the segment and the end of the entire range. By using nested loops, only check for the end of the entire range is needed in each iteration.

```cpp
using seg_vec_t = str2d::seg::vector<int>; 
using seg_set_t = str2d::seg::set<int>;

using segmented_coordinate = str2d::SegmentedCoordinate<seg_vec_t>;

using segment_iterator = str2d::SegmentIterator<seg_vec_t>; 
// or segment_iterator = str2d::SegmentIterator<segmented_coordinate>

using flat_iterator = str2d::FlatIterator<seg_vec_t>; 
// or flat_iterator = str2d::FlatIterator<segmented_coordinate>

seg_vec_t sv; 
segmented_coordinate first = sv.begin(); 
segmented_coordinate last = sv.end(); 

segment_iterator first_seg = first.segment(); // or first_seg = str2d::segment(it)
flat_iterator first_flat = first.flat();   // or first_flat = str2d::flat(it)

segment_iterator last_seg = last.segment(); // or first_seg = str2d::segment(it)
flat_iterator first_flat = last.flat();   // or first_flat = str2d::flat(it)
```
Now in order to write any algorithm you would have to write a nested loop using segment and flat iterators. Considering
that would be very cumbersome, the library already provides some basic generic algorithms which work on these coordinate structures.
If you need an algorithm which is not in the library, just write it yourself in put it there; that, in the end, is the way standard
template library was intended to be used, by always being extended.

Note 1) These objects and typedefs will be used throughout the examples bellow. 

Note 2) I deliberately avoid using the keyword ```auto``` in order
        to show exactly what type an object is. Very likely it would be used in real code.

### Insertion
If an element is inserted into a segment which isn't at full capacity all actions are confined to that segment(which makes the structure very cache friendly), otherwise an allocation of new segments and/or rebalancing to neighbouring segments have to occur.
In the case than new allocations happen, new segment headers have to be inserted into the index. Once the index becomes large enough, the operation of inserting into the index starts to affect performance. 

### Erasure
If an element is erased from a segment which holds more than "limit" elements, all operations are confided to that segment; otherwise
a deallocation of the segment and/or rebalancing to neighbouring segments have to occur. It has the same good cache locality and same problems with the index size getting to big as the number of elements in the container increases.

### Lookup
If the data isn't sorted, linear lookup is the best you can get. If it is, as it is for `str2d::seg::multiset` and `str2d::seg::multimap` binary search(`lower_bound`, `upper_bound`, `equal_range`) can be used. Considering the segmented coordinate is a bidirectional iterator, regular binary search wouldn't be a massive improvement over the linear search. Binary search algorithms inside the library are aware of the coordinate structures presented above and can use them to an advantage. Firstly, a binary search over a range of segments is used to locate the segment on which our element resides. After that segment had been located, another binary search
(regular one) is used to locate the flat iterator of that segment which points to the element we were looking for.


# Memory 

# Exception Safety

# Map

# Conclusion
In a sense, the segmented vector extends the application area of "flat" vector so it can be used as a set container for a large number of elements. As benchmarks show, that extension has limits which have to be taken into account. 

Google's btree is probably a safe bet as a drop in replacement for the std map and set data structures. If on the other hand iterations dominate other operations, or you're constantly erasing and inserting entire ranges, you could consider using the segmented vector.

Needless to say, these opinions mean little in comparison to actual benchmarks of your code.
