# Motivation
Str2D is a library of 2D algorithms and data structures implemented in c++17 designed for manipulating large amounts of data. 

While reading two books, `Elements of Programming`(which now you can read for [free](http://componentsprogramming.com/elements-of-programming-authors-edition/)) and `From mathematics to generic programming`, I stumbled upon a coordinate structure called the segmented iterator and realised I could implement it together with data structures and algorithms needed for its use.  
The second motivation was [this](https://www.google.com/url?sa=t&source=web&rct=j&url=https://people.freebsd.org/~lstewart/articles/cpumemory.pdf&ved=2ahUKEwirjajuv57jAhVrxKYKHbfvDV4QFjAAegQIAhAB&usg=AOvVaw3VY2lnCBaI-B57Dric65cb) paper, which explained to me the inadequacy of data structures which utilize numerous single node allocations(e.g. `std::set` and the like).

At the heart of the library lies a data structure called `str2d::seg::vector`, the rest are build on top of it; hence I'll only focus on it. Once you've understood how the segmented vector is implemented you'll easily deduce how to use it to implement `set`-like and `map`-like data structures.

Note : There are currently only `str2d::seg::multiset` and `str2d::seg::multimap` data structures in this library apart from the `str2d::seg::vector`. The reason for exlusion of `str2d::seg::set` and `str2d::seg::map` is the lack of time; they will probably be included some time later.

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

Typedefs used in the examples bellow :
```cpp
using seg_vec_t = str2d::seg::vector<int>; 
using seg_set_t = str2d::seg::multiset<int>;

using segmented_coordinate = str2d::SegmentedCoordinate<seg_vec_t>;

using segment_iterator = str2d::SegmentIterator<seg_vec_t>; 
// or segment_iterator = str2d::SegmentIterator<segmented_coordinate>

using flat_iterator = str2d::FlatIterator<seg_vec_t>; 
// or flat_iterator = str2d::FlatIterator<segmented_coordinate>
``` 

Functions  used in the examples bellow :
```cpp
int rand_int(); // returns a random integer

template<typename C>
Iterator<C> rand_iterator(const C& c); // return a random iterator

seg_vec_t init_vector(); // initializes vector so that the objects inside it have random values

seg_set_t init_set(); // initializes set so that the objects inside it have have nondecreasing values
```

```cpp

void test() {
   seg_vec_t svec = init_vector();
   
   segmented_coordinate first = svec.begin(); 
   segmented_coordinate last = svec.end();

   segmented_coordinate middle = str2d::seg::successor(first, svec.size() >> 1); 
   // extracts segment and flat iterators from the coordinate
   // and advances much faster than a regular bidirectioanl iterator would
   
   flat_iterator middle_flat = middle.flat();      
   // or middle_flat = str2d::flat(middle)
   // extracting the flat iterator from the coordinate
  
   while(middle_flat != middle.end()) {
      *middle_flat += 1; 
      ++middle_flat;
   }
   // increments the value pointed to by every flat iterator in the range [middle_flat, middle.end())
   
   segment_iterator middle_seg = middle.seg();      
   // or middle_seg = str2d::seg(middle)
   // extracting the segment iterator from the coordinate
   
   while(middle_seg != last.seg()) {
      *(middle.begin() + (middle.size() >> 1)) += 1;
      ++middle_seg;
   }
   // increments the value in the middle of every segment in the segment range [middle_seg, last.seg()) 
}

using seg_set_t = str2d::seg::multiset<int>;
seg_vec_t sset;
```
Now in order to write any algorithm you would have to write a nested loop using segment and flat iterators. Considering
that would be very cumbersome to write every time, the library already provides some basic generic algorithms which work on these coordinate structures.
If you need an algorithm which is not in the library, just write it yourself in put in there; that, in the end, that is the way standard
template library was intended to be used; by using the already established algorithms and extending adding new usefull ones.

Note : I deliberately avoided using the keyword `auto` in these examples in order to show what are exact types of these
        coordinate structures. Later on `auto` will be used.
        
        
### Lookup
If the data isn't sorted, linear lookup is the best you can get. If it is, as it is for `str2d::seg::multiset` and `str2d::seg::multimap` binary search(`lower_bound`, `upper_bound`, `equal_range`) can be used. Considering the segmented coordinate is a bidirectional iterator, regular binary search wouldn't be a massive improvement over the linear search. Binary search algorithms inside the library are aware of the coordinate structures presented above and can use them to an advantage. Firstly, a binary search over a range of segments is used to locate the segment on which our element resides. After that segment had been located, another binary search
(regular one) is used to locate the flat iterator of that segment which points to the element we were looking for.
```cpp

```

### Insertion
If an element is inserted into a segment which isn't at full capacity all actions are confined to that segment(which makes the structure very cache friendly), otherwise an allocation of new segments and/or rebalancing to neighbouring segments have to occur.
In the case than new allocations happen, new segment headers have to be inserted into the index. Once the index becomes large enough, the operation of inserting into the index starts to affect performance.
```cpp
svec.insert(rand_iterator(svec), rand_int());
sset.insert(rand_int());
```
`insert` method of the set first has to look for the place where the object has to be inserted. If we happen to know
where that place is, we can insert directly there without breaking the set invariants(the element before the place we're inserting must be less or equal to, and the element at the place we're inserting must be greater or equal to the element we want to insert). 
```cpp
sset.insert_unguarded(x);
```
If we're inserting a sorted range of elements.   

### Erasure
If an element is erased from a segment which holds more than "limit" elements, all operations are confided to that segment; otherwise
a deallocation of the segment and/or rebalancing to neighbouring segments have to occur. It has the same good cache locality and same problems with the index size getting to big as the number of elements in the container increases.
```cpp

```
# Memory 

# Exception Safety

# Map

# Conclusion
In a sense, the segmented vector extends the application area of "flat" vector so it can be used as a set container for a large number of elements. As benchmarks show, that extension has limits which have to be taken into account. 

Google's btree is probably a safe bet as a drop in replacement for the std map and set data structures. If on the other hand iterations dominate other operations, or you're constantly erasing and inserting entire ranges, you could consider using the segmented vector.

Needless to say, these opinions mean little in comparison to actual benchmarks of your code.
