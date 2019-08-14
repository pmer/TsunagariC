/********************************
** Tsunagari Tile Engine       **
** sort.h                      **
** Copyright 2019 Paul Merrill **
********************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

// Original source downloaded from: https://github.com/orlp/pdqsort

// **********
// pdqsort.h - Pattern-defeating quicksort.
//
// Copyright (c) 2015 Orson Peters
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not claim
//    that you wrote the original software. If you use this software in a
//    product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
// **********

#ifndef SRC_UTIL_SORT_H_
#define SRC_UTIL_SORT_H_

#include "util/algorithm.h"
#include "util/int.h"
#include "util/move.h"
#include "util/noexcept.h"

namespace pdqsort_detail {
    template<typename T>
    void sift_down(T* first, T* /*last*/, size_t len, T* start) noexcept {
        // left-child of start is at 2 * start + 1
        // right-child of start is at 2 * start + 2
        size_t child = start - first;

        if (len < 2 || (len - 2) / 2 < child)
            return;

        child = 2 * child + 1;
        T* child_i = first + child;

        if ((child + 1) < len && child_i[0] < child_i[1]) {
            // right-child exists and is greater than left-child
            ++child_i;
            ++child;
        }

        // check if we are in heap-order
        if (*child_i < *start) {
            // we are, start is larger than its largest child
            return;
        }

        T top(move_(*start));
        do {
            // we are not in heap-order, swap the parent with it's largest child
            *start = move_(*child_i);
            start = child_i;

            if ((len - 2) / 2 < child)
                break;

            // recompute the child based off of the updated parent
            child = 2 * child + 1;
            child_i = first + child;

            if ((child + 1) < len && child_i[0] < child_i[1]) {
                // right-child exists and is greater than left-child
                ++child_i;
                ++child;
            }

            // check if we are in heap-order
        } while (!(*child_i < top));
        *start = move_(top);
    }

    template<typename T> void make_heap(T* first, T* last) noexcept {
        size_t n = last - first;
        if (n <= 1) {
            return;
        }
        for (size_t start = (n - 2) / 2; start >= 0; start--) {
            sift_down(first, last, n, first + start);
        }
    }

    template<typename T> void pop_heap(T* first, T* last, size_t len) noexcept {
        if (len > 1) {
            swap_(*first, *--last);
            sift_down(first, last, len - 1, first);
        }
    }

    template<typename T> void sort_heap(T* first, T* last) noexcept {
        for (size_t n = last - first; n > 1; last--, n--) {
            pop_heap(first, last, n);
        }
    }
}  // namespace pdqsort_detail

namespace pdqsort_detail {
    enum {
        // Partitions below this size are sorted using insertion sort.
        insertion_sort_threshold = 24,

        // Partitions above this size use Tukey's ninther to select the pivot.
        ninther_threshold = 128,

        // When we detect an already sorted partition, attempt an insertion sort
        // that allows this
        // amount of element moves before giving up.
        partial_insertion_sort_limit = 8,

        // Must be multiple of 8 due to loop unrolling, and < 256 to fit in
        // unsigned char.
        block_size = 64,

        // Cacheline size, assumes power of two.
        cacheline_size = 64

    };

    // Returns floor(log2(n)), assumes n > 0.
    template<typename T> inline int log2(T n) noexcept {
        int log = 0;
        while (n >>= 1) {
            ++log;
        }
        return log;
    }

    // Sorts [begin, end) using insertion sort.
    template<typename T> inline void insertion_sort(T* begin, T* end) noexcept {
        if (begin == end) {
            return;
        }

        for (T* cur = begin + 1; cur != end; ++cur) {
            T* sift = cur;
            T* sift_1 = cur - 1;

            // Compare first so we can avoid 2 moves for an element already
            // positioned correctly.
            if (*sift < *sift_1) {
                T tmp = move_(*sift);

                do {
                    *sift-- = move_(*sift_1);
                } while (sift != begin && tmp < *--sift_1);

                *sift = move_(tmp);
            }
        }
    }

    // Sorts [begin, end) using insertion sort with the given comparison
    // function. Assumes
    // *(begin - 1) is an element smaller than or equal to any element in
    // [begin, end).
    template<typename T>
    inline void unguarded_insertion_sort(T* begin, T* end) noexcept {
        if (begin == end)
            return;

        for (T* cur = begin + 1; cur != end; ++cur) {
            T* sift = cur;
            T* sift_1 = cur - 1;

            // Compare first so we can avoid 2 moves for an element already
            // positioned correctly.
            if (*sift < *sift_1) {
                T tmp = move_(*sift);

                do {
                    *sift-- = move_(*sift_1);
                } while (tmp < *--sift_1);

                *sift = move_(tmp);
            }
        }
    }

    // Attempts to use insertion sort on [begin, end). Will return false if more
    // than partial_insertion_sort_limit elements were moved, and abort sorting.
    // Otherwise it will successfully sort and return true.
    template<typename T>
    inline bool partial_insertion_sort(T* begin, T* end) noexcept {
        if (begin == end) {
            return true;
        }

        int limit = 0;
        for (T* cur = begin + 1; cur != end; ++cur) {
            if (limit > partial_insertion_sort_limit) {
                return false;
            }

            T* sift = cur;
            T* sift_1 = cur - 1;

            // Compare first so we can avoid 2 moves for an element already
            // positioned correctly.
            if (*sift < *sift_1) {
                T tmp = move_(*sift);

                do {
                    *sift-- = move_(*sift_1);
                } while (sift != begin && tmp < *--sift_1);

                *sift = move_(tmp);
                limit += static_cast<int>(cur - sift);
            }
        }

        return true;
    }

    template<typename T> inline void sort2(T* a, T* b) noexcept {
        if (*b < *a) {
            swap_(*a, *b);
        }
    }

    // Sorts the elements *a, *b and *c.
    template<typename T> inline void sort3(T* a, T* b, T* c) noexcept {
        sort2(a, b);
        sort2(b, c);
        sort2(a, b);
    }

    template<typename T> inline T* align_cacheline(T* p) noexcept {
        size_t ip = reinterpret_cast<size_t>(p);
        ip = (ip + cacheline_size - 1) & -cacheline_size;
        return reinterpret_cast<T*>(ip);
    }

    template<typename T>
    inline void swap_offsets(T* first,
                             T* last,
                             unsigned char* offsets_l,
                             unsigned char* offsets_r,
                             int num,
                             bool use_swaps) noexcept {
        if (use_swaps) {
            // This case is needed for the descending distribution, where we
            // need to have proper swapping for pdqsort to remain O(n).
            for (int i = 0; i < num; ++i) {
                swap_(*(first + offsets_l[i]), *(last - offsets_r[i]));
            }
        }
        else if (num > 0) {
            T* l = first + offsets_l[0];
            T* r = last - offsets_r[0];
            T tmp(move_(*l));
            *l = move_(*r);
            for (int i = 1; i < num; ++i) {
                l = first + offsets_l[i];
                *r = move_(*l);
                r = last - offsets_r[i];
                *l = move_(*r);
            }
            *r = move_(tmp);
        }
    }

    template<typename T> struct partition_result {
        T* pivot_pos;
        bool already_partitioned;
    };

    // Partitions [begin, end) around pivot *begin using comparison function
    // comp. Elements equal to the pivot are put in the right-hand partition.
    // Returns the position of the pivot after partitioning and whether the
    // passed sequence already was correctly partitioned. Assumes the pivot is a
    // median of at least 3 elements and that [begin, end) is at least
    // insertion_sort_threshold long.
    template<typename T>
    inline partition_result<T> partition_right(T* begin, T* end) noexcept {
        // Move pivot into local for speed.
        T pivot(move_(*begin));

        T* first = begin;
        T* last = end;

        // Find the first element greater than or equal than the pivot (the
        // median of 3 guarantees this exists).
        while (*++first < pivot)
            ;

        // Find the first element strictly smaller than the pivot. We have to
        // guard this search if there was no element before *first.
        if (first - 1 == begin)
            while (first < last && !(*--last < pivot))
                ;
        else
            while (!(*--last < pivot))
                ;

        // If the first pair of elements that should be swapped to partition are
        // the same element, the passed in sequence already was correctly
        // partitioned.
        bool already_partitioned = first >= last;

        // Keep swapping pairs of elements that are on the wrong side of the
        // pivot. Previously swapped pairs guard the searches, which is why the
        // first iteration is special-cased above.
        while (first < last) {
            swap_(*first, *last);
            while (*++first < pivot)
                ;
            while (!(*--last < pivot))
                ;
        }

        // Put the pivot in the right place.
        T* pivot_pos = first - 1;
        *begin = move_(*pivot_pos);
        *pivot_pos = move_(pivot);

        return partition_result<T>{pivot_pos, already_partitioned};
    }

    // Similar function to the one above, except elements equal to the pivot are
    // put to the left of the pivot and it doesn't check or return if the passed
    // sequence already was partitioned. Since this is rarely used (the many
    // equal case), and in that case pdqsort already has O(n) performance, no
    // block quicksort is applied here for simplicity.
    template<typename T> inline T* partition_left(T* begin, T* end) noexcept {
        T pivot(move_(*begin));
        T* first = begin;
        T* last = end;

        while (pivot < *--last)
            ;

        if (last + 1 == end)
            while (first < last && !(pivot < *++first))
                ;
        else
            while (!(pivot < *++first))
                ;

        while (first < last) {
            swap_(*first, *last);
            while (pivot < *--last)
                ;
            while (!(pivot < *++first))
                ;
        }

        T* pivot_pos = last;
        *begin = move_(*pivot_pos);
        *pivot_pos = move_(pivot);

        return pivot_pos;
    }

    template<typename T>
    inline void pdqsort_loop(T* begin,
                             T* end,
                             int bad_allowed,
                             bool leftmost = true) noexcept {
        // Use a while loop for tail recursion elimination.
        while (true) {
            size_t size = end - begin;

            // Insertion sort is faster for small arrays.
            if (size < insertion_sort_threshold) {
                if (leftmost) {
                    insertion_sort(begin, end);
                }
                else {
                    unguarded_insertion_sort(begin, end);
                }
                return;
            }

            // Choose pivot as median of 3 or pseudomedian of 9.
            size_t s2 = size / 2;
            if (size > ninther_threshold) {
                sort3(begin, begin + s2, end - 1);
                sort3(begin + 1, begin + (s2 - 1), end - 2);
                sort3(begin + 2, begin + (s2 + 1), end - 3);
                sort3(begin + (s2 - 1), begin + s2, begin + (s2 + 1));
                swap_(begin[0], begin[s2]);
            }
            else {
                sort3(begin + s2, begin, end - 1);
            }

            // If *(begin - 1) is the end of the right partition of a previous
            // partition operation there is no element in [begin, end) that is
            // smaller than *(begin - 1). Then if our pivot compares equal to
            // *(begin - 1) we change strategy, putting equal elements in the
            // left partition, greater elements in the right partition. We do
            // not have to recurse on the left partition, since it's sorted (all
            // equal).
            if (!leftmost && !(*(begin - 1) < *begin)) {
                begin = partition_left(begin, end) + 1;
                continue;
            }

            // Partition and get results.
            partition_result<T> part_result = partition_right(begin, end);
            T* pivot_pos = part_result.pivot_pos;
            bool already_partitioned = part_result.already_partitioned;

            // Check for a highly unbalanced partition.
            size_t l_size = pivot_pos - begin;
            size_t r_size = end - (pivot_pos + 1);
            bool highly_unbalanced = l_size < size / 8 || r_size < size / 8;

            // If we got a highly unbalanced partition we shuffle elements to
            // break many patterns.
            if (highly_unbalanced) {
                // If we had too many bad partitions, switch to heapsort to
                // guarantee O(n log n).
                if (--bad_allowed == 0) {
                    make_heap(begin, end);
                    sort_heap(begin, end);
                    return;
                }

                if (l_size >= insertion_sort_threshold) {
                    swap_(begin[0], begin[l_size / 4]);
                    swap_(pivot_pos[-1], pivot_pos[0 - l_size / 4]);

                    if (l_size > ninther_threshold) {
                        swap_(begin[1], begin[l_size / 4 + 1]);
                        swap_(begin[2], begin[l_size / 4 + 2]);
                        swap_(pivot_pos[-2], pivot_pos[0 - (l_size / 4 + 1)]);
                        swap_(pivot_pos[-3], pivot_pos[0 - (l_size / 4 + 2)]);
                    }
                }

                if (r_size >= insertion_sort_threshold) {
                    swap_(pivot_pos[1], pivot_pos[1 + r_size / 4]);
                    swap_(end[-1], end[0 - r_size / 4]);

                    if (r_size > ninther_threshold) {
                        swap_(pivot_pos[2], pivot_pos[2 + r_size / 4]);
                        swap_(pivot_pos[3], pivot_pos[3 + r_size / 4]);
                        swap_(end[-2], end[0 - (1 + r_size / 4)]);
                        swap_(end[-3], end[0 - (2 + r_size / 4)]);
                    }
                }
            }
            else {
                // If we were decently balanced and we tried to sort an already
                // partitioned sequence try to use insertion sort.
                if (already_partitioned &&
                    partial_insertion_sort(begin, pivot_pos) &&
                    partial_insertion_sort(pivot_pos + 1, end)) {
                    return;
                }
            }

            // Sort the left partition first using recursion and do tail
            // recursion elimination for the right-hand partition.
            pdqsort_loop<T>(begin, pivot_pos, bad_allowed, leftmost);
            begin = pivot_pos + 1;
            leftmost = false;
        }
    }
}  // namespace pdqsort_detail

template<typename T>
inline void
pdqsort(T* begin, T* end) noexcept {
    if (begin == end) {
        return;
    }

    pdqsort_detail::pdqsort_loop<T>(
            begin, end, pdqsort_detail::log2(end - begin));
}

#endif  // SRC_UTIL_SORT_H_
