# Motivation
Str2D is a library of 2D algorithms and data structures implemented in c++17 designed for manipulating large amounts of data. 

While reading two books, `Elements of Programming`(which now you can read for [free](http://componentsprogramming.com/elements-of-programming-authors-edition/)) and `From mathematics to generic programming`, I stumbled upon a coordinate structure called the SegmentIterator and realised I could implement it together with data structures and algorithms needed for its use.  
The second motivation was [this](https://www.google.com/url?sa=t&source=web&rct=j&url=https://people.freebsd.org/~lstewart/articles/cpumemory.pdf&ved=2ahUKEwirjajuv57jAhVrxKYKHbfvDV4QFjAAegQIAhAB&usg=AOvVaw3VY2lnCBaI-B57Dric65cb) paper, which explained to me the inadequacy of data structures which utilize numerous single node allocations(e.g. `std::set` and the like).

At the heart of the library lies a data structure called `str2d::seg::vector`, the rest are build on top of it; hence this guide will mainly focus on it and somewhat on `str2d::seg::set`(`str2d::seg::map` is exluded beaceuse it's functionally almost indentical to `str2d::seg::set`). Once you've understood how the segmented vector is implemented you'll easily deduce how to use it to implement `set`-like and `map`-like data structures.

Note : There are currently only `str2d::seg::multiset` and `str2d::seg::multimap` data structures in this library apart from the `str2d::seg::vector`. The reason for exlusion of `str2d::seg::set` and `str2d::seg::map` is the lack of time; they will probably be included some time later.

# Implementation and Usage
Segmented vector is not a difficult structure to imagine. 

In it, an `std::vector` is used as an index which holds segment headers, structures holding pointers to segments of memory and possibly some meta data(detailed explaination of segment headers will be given bellow). Those segments are where the data is actually held. The capacity of every segment is constant; the size on the other hand can vary.
Each segment holds at least half the capacity(`limit`) elements on it, except the first one; it can hold as many(less than capacity) or as little(more than 0) as it needs.

Objects stored in `str2d::seg::multiset` and `str2d::seg::multimap` are all mutable. For `str2d::seg::multiset`, I could have made the objects constant while the user is manipulating them and mutable when they're used internally; I couldn't do the same for `str2d::seg::multimap`, so I decided to leave them mutable for both data structures. The user will have to take care not to mess up the invariants. On the other hand, this will prove useful when we want to bypass some unnecessary checks.

## Coordinate Structures/Iterators
Segmented vector utilizes 3 kinds of coordinate structures : 
1) Segment iterator - random access iterator that iterates over a range of segments. It can't be dereferenced like ordinary
random access iterators; data inside it is accessed like it's accessed in a sequence container(e.g. `std::vector`), i.e. by
using `begin` and `end` methods of the segment iterator. Return type of those methods is a flat iterator.
   
2) Flat iterator - regular random access iterator; when dereferenced, returns the value type stored in the segmented vector.

3) Segmented coordinate - regular bidirectinal iterator; when dereferenced, returns the value type stored in the segmented vector.
This type is returned when `begin` and `end` functions of the segmented vector are called. Inside, it holds a segment iterator and a flat iterator pointing somewhere inside that segment. It bassically works like the iterator of `std::deque`, except it's not random access. Its segment iterator is accessed through `segment` method, while its flat iterator is accessed via `flat` method of the coordinate.

The algorithms in the library are aware of these coordinate structures, and use them in nesteed loops to decrease the number of checks needed in each iterations. If only segmented coordinate(regular bidirectional iterator) were used, each iteration of an algorithm would have to check whether it's reached the end of the segment and the end of the entire range. By using nested loops, only check for the end of the entire range is needed in each iteration. There is alse the check to see whether we have reached the last segment; it happens once for each segment in the range.

Typedefs used in the examples bellow :
```cpp
#include <str2d.h>

using seg_vec_t = str2d::seg::vector<int>; 
using seg_set_t = str2d::seg::multiset<int>;

using segmented_coordinate = str2d::SegmentedCoordinate<seg_vec_t>;

using segment_iterator = str2d::SegmentIterator<seg_vec_t>; 
// or segment_iterator = str2d::SegmentIterator<segmented_coordinate>

using flat_iterator = str2d::FlatIterator<seg_vec_t>; 
// or flat_iterator = str2d::FlatIterator<segmented_coordinate>
``` 

Functions used in the examples bellow :
```cpp
int rand_int(); // returns a random integer

template<typename C>
str2d::Iterator<C> rand_iterator(const C& c); // return a random iterator

seg_vec_t init_vector(); 
// initializes a vector so that it's not empty and the objects inside it have random values

seg_set_t init_set(); 
// initializes a set so that it's not empty and the objects inside it have have nondecreasing values

struct increment
{
   bool operator()(int& x) { ++x; }
}
```

```cpp

void coordinates_example() {
   seg_vec_t svec = init_vector();
   
   segmented_coordinate first = svec.begin(); 
   segmented_coordinate last = svec.end();

   segmented_coordinate middle = str2d::seg::successor(first, svec.size() >> 1); 
   // "str2d::seg::successor" extracts segment and flat iterators from the coordinate
   // and advances much faster than a regular bidirectioanl iterator would
   
   flat_iterator middle_flat = middle.flat();      
   // or middle_flat = str2d::flat(middle)
   // extracting the flat iterator from the coordinate
  
   while(middle_flat != middle.end()) {
      *middle_flat += 1; 
      ++middle_flat;
   }
   // increments the value pointed to by every flat iterator 
   // in the range [middle_flat, middle.end())
   
   segment_iterator middle_seg = middle.seg();      
   // or middle_seg = str2d::seg(middle)
   // extracting the segment iterator from the coordinate
   
   while(middle_seg != last.seg()) {
      *(middle.begin() + (middle.size() >> 1)) += 1;
      ++middle_seg;
   }
   // increments the value of the object in the middle of every segment 
   // in the segment range [middle_seg, last.seg()) 
}
```
Now in order to write any algorithm you would have to write a nested loop using segment and flat iterators. Considering
that would be very cumbersome to write every time, the library already provides some basic generic algorithms which work on these coordinate structures.
If you need an algorithm which is not in the library, just write it yourself in put in there; that, in the end, is the way the standard
template library was intended to be used; by using already established algorithms and adding new useful ones.

Note : Use of the keyword `auto` was deliberately avoided in these examples in order to show the exact types of these
       coordinate structures. Later on `auto` will be used.
       
       
## Iteration
For iteration we can always write a neested loop which would do the job.
```cpp
void iterate_by_hand_example() {
   seg_vec_t svec = init_vec();
   auto [first_seg, first_flat] = str2d::seg::extract(svec.begin());
   auto [last_seg, last_flat] = str2d::seg::extract(str2d::seg::successor(svec.begin(), svec.size() >> 1));
   while(first_seg != last_seg) {
      std::for_each(first_flat, first_seg.end(), increment());
      ++first_seg;
      first_flat = first_seg.begin();
   }
   std::for_each(first_flat, last_flat, increment());
}
```
As said, it's cumbersome writing neested loops, so we just use already existing algorithms.
```cpp
void iterator_example() {
   seg_vec_t svec = init_vec();
   str2d::seg::for_each(svec.begin(), svec.end(), increment());
}
```
        
## Lookup
If the data isn't sorted, linear lookup is the best we can get. If it is, as it is for `str2d::seg::multiset` and `str2d::seg::multimap`, binary search(`lower_bound`, `upper_bound`, `equal_range`) can be used. Considering the segmented coordinate is a bidirectional iterator, regular binary search wouldn't be a massive improvement over the linear search. Binary search algorithms inside the library are aware of the coordinate structures presented above and can use them to an advantage. Firstly, a binary search over a range of segments is used to locate the segment on which our element resides. After that segment had been located, another binary search(regular one) is used to locate the flat iterator of that segment which points to the element we were looking for.
```cpp
void linear_lookup_example() {
   seg_vec_t svec = init_vector();
   
   auto it = str2d::seg::find(svec.begin(), svec.end(), rand_int());
   if(it != svec.end()) {
      ++(*it);
      // element has been found
      // increment in by 1
   }
}

void binary_lookup_example() {
   seg_set_t sset = init_set();
   
   int r = rand_int();
   auto it = str2d::seg::lower_bound(sset.begin(), sset.end(), r);
   // or just use lower_bound method of the set
   // it = sset.lower_bound(r);
   
   if(it != svec.end() && *it == r) {
      ++(*it);
      // element has been found
      // increment in by 1
   }
   // or just use lower_bound method of the set
   it = sset.lower_bound(r);
}
```

## Insertion
If an element is inserted into a segment which isn't at full capacity all actions are confined to that segment(which makes the structure very cache friendly), otherwise an allocation of new segments and/or rebalancing to neighbouring segments have to occur.
In the case than new allocations happen, new segment headers have to be inserted into the index. 
Once the index becomes large enough, the operation of inserting into the index starts to affect performance.
```cpp
void insert_example() {
   seg_vec_t svec = init_vector();
   seg_vec_t sset = init_set();

   svec.insert(rand_iterator(svec), rand_int());
   sset.insert(rand_int());
}
```
### Unguarded Insertion
`insert` method of `set` first has to look for the place where the object has to be inserted. If we happen to know
where that place is, we can insert the element directly there. Unguarded insert methods don't do any checks to see whether the place we're inserting is valid for the given element. We must insure ourselves that the invariants aren't broken(the element before the place we're inserting must be less than or equal to, and the element at the place we're inserting must be greater than or equal to the element we want to insert). Consdering `str2d::seg::set` is build on top of `str2d::seg::vector`, you can probaly guess that `insert_unguarded` methods are just wrappers for `str2d::seg::vector::insert`.
```cpp
void set_insert_unguarded_example() {
   seg_vec_t sset = init_set();
   auto it = sset.lower_bound(x);
   auto[first, last] = sset.insert_unguarded(it, x);
}
```
### Sorted Range Unguarded Insertion
Sometimes we know that inserting an entire range at some position won't break the `set` invariants(inserted range must be sorted + the element before the place we're inserting must be less than or equal to the first element of the inserted range, and the element at the place we're inserting must be greater than or equal to the last element of the inserted range).
```cpp
void set_insert_sorted_unguarded_example() {
   seg_vec_t sset = init_set();
   auto it = str2d::seg::successor(sset.begin(), set.size() >> 1);
   std::vector<int> v(100, *it);
   auto[first, last] = sset.insert_sorted_unguarded(it, v.begin(), 100);
   // inserts 100 new objects which are equal to "*it" at the "it" position(middle of the range in this case)
   // we could have also used sset.insert_move_sorted_unguarded(it, v.begin(), 100) which
   // move constructs the new range from the objects in the range [v.begin(), v.begin() + 100) 
   
   if(last == sset.end() || *last != *first) {
      str2d::seg::for_each(first, last, increment());
      // iterating over the inserted range and incrementing the value of every object in it
      // we made sure it won't break the invariant by assuring that the element after the range has a 
      // greater value the the elemnts in the range
   }
}
```
## Erasure
If an element is erased from a segment which holds more than `limit` elements, all operations are confided to that segment; otherwise
a deallocation of the segment and/or rebalancing to neighbouring segments have to occur. It has the same good cache locality and same problems with the index size affecting performance, as does insertion.
```cpp
void erase_example() {
   seg_vec_t svec = init_vec();
   seg_set_t sset = init_set();

   auto it = str2d::seg::find_if(svec.begin(), svec.end(), [](int x) { return x > 100; });
   if(it != svec.end()) {
      svec.erase(it);
      // erasing the first object whose value is greater than 100
   }
   
   auto[first, last] = sset.equal_range(100);
   sset.erase(first, last);
   // erasing all objects whose value is equal to 100
}
```

## Segment Header
`SegmentHeader` is a concept which allows us to abstract the way we access data inside segments.
The segments used in Str2D library are double-ended; which means that the begining of user data isn't necessarily at the beginning of the segment; becase of that, two indices are needed; one indicating the beginning of user data, other indicating the ending.
Taking this into consideration there are, to my knowledge, two types which model `SegmentHeader` concept.

### Small Segment Header
Stores only a pointer to a segment. Exactly next to the memory allocated for the segmented, there is an area of memory allocated for the two indices.

### Big Segment Header
Stores both the pointer to a segment, and the two indices indicating begininng and ending. 

Smaller header means smaller index. On the other hand, one extra cache miss which might occur, when we need to find the beginning or the ending of user data, mean that almost all operations are slower wth small than big segment header(this will be shown in Benchmarks section). By default the library uses big headers; if the need arises, another type which satisfies `SegmentHeader` concept can easily replace the default.


# Memory 

All segments(except the first one) are at least half full. For every segment allocated we need a pointer plus two (16 bit)indices. So the equation for the amount of all memory(in bytes) allocated on heap(index + segments) which is not used to store our objects is :
```cpp
float memory_overhead(const seg_vec_t& svec) {
   using value_type = str2d::ValueType<svec>;
   float per_segment_index_overhead = static_cast<float>(sizeof(std::tuple<value_type*, uint16_t, uint16_t>)); 
   float segment_capacity = static_cast<float>(str2d::seg::SegmentCapacity<seg_vec_t>);
   float nm_segments = static_cast<float>(svec.index.size());
   float used_segment_bytes = static_cast<float>(svec.index.size() * sizeof(T));
   float unused_segment_bytes = nm_segments * segment_capacity - used_segment_bytes;
   float index_bytes = static_cast<float>(svec.index.capacity()) * per_segment_index_overhead;
   return (index_bytes + unused_segment_bytes) / used_segment_bytes;
}
```
If we're storing small objects, for example up to 16 bytes or less, we'll almost certainly save up some memory in comparison to `std::set`, but not in comparison to google's `btree`. 

Note : If anyone is willing(and unlike me, able) to the statistical calculations to show the exact memory utilization in comparison to other data structures and/or do tests which show how much memory is being used, please do so, and send me the results. 


# Exception Safety
I didn't know of a way to implement exception safety so that there's always basic exception guarantee, without losing efficiency.
Basically if the type we're storing is POD(Plain Old Data), all segmented vector operations have basic exception guarantee.

## Erasure
If the object type we're storing has a move constructor or a copy constructor which don't throw, we have basic exception guarantee; otherwise no guarantee is given.

## Copy Insertion
By copy insertion we mean calling `insert` with an lvalue reference or `insert_sorted_unguarded` or `insert_sorted`.
If both the copy and the move constructor don't throw, we have basic exception guarantee, otherwise no guarantee.

## Move Insertion
By copy insertion we mean calling `insert` with an rvalue reference or `insert_move_sorted_unguarded` or `insert_move_sorted`.
If the move constructor doesn't throw, we have basic exception guarantee, otherwise no guarantee.

# UnitTests

# Benchmarks

# Installation
You'll need a c++17 compiler. 
Place all files inside Str2D directory of this repository, into a directory of your choice. Include str2d.h header file and you're ready to go.

# Conclusion
In a sense, the segmented vector extends the application area of the "flat" vector so it can be used as a set or as a container where insertion order matters, for a large number of elements. As benchmarks show, that extension has limits which have to be taken into account. 

Google's btree is probably a safe bet as a drop in replacement for the `std::map` and `std::map` data structures. If on the other hand iterations dominate other operations, or you're constantly erasing and inserting entire ranges and not single elements, you could consider using the segmented vector.

Needless to say, these opinions mean little in comparison to actual benchmarks of your code.
