///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
// Copyright 2017-2019 Paul Merrill
///////////////////////////////////////////////////////////////////////////////

#ifndef SRC_UTIL_VECTOR_H_
#define SRC_UTIL_VECTOR_H_


#include <stddef.h>
#include <stdlib.h>

#include "util/algorithm.h"
#include "util/assert.h"
#include "util/likely.h"
#include "util/move.h"
#include "util/new.h"

template<typename T>
class vector {
 protected:
    T* mpBegin;
    T* mpEnd;
    T* mCapacity;

 protected:
    T*     DoAllocate(size_t n);
    void   DoFree(T* p);
    size_t GetNewCapacity(size_t currentCapacity);

 public:
    typedef T*       iterator;
    typedef const T* const_iterator;

 public:
    vector();
    explicit vector(size_t n);
    vector(size_t n, const T& value);
    vector(const vector<T>& x);
    vector(vector<T>&& x);

    ~vector();

    vector<T>& operator=(const vector<T>& x);
    vector<T>& operator=(vector<T>&& x);

    void swap(vector<T>& x);

    iterator       begin() noexcept;
    const_iterator begin() const noexcept;

    iterator       end() noexcept;
    const_iterator end() const noexcept;

    bool   empty() const noexcept;
    size_t size() const noexcept;
    size_t capacity() const noexcept;

    void resize(size_t n);
    void reserve(size_t n);

    T*       data() noexcept;
    const T* data() const noexcept;

    T&       operator[](size_t n);
    const T& operator[](size_t n) const;

    T&       at(size_t n);
    const T& at(size_t n) const;

    T&       front();
    const T& front() const;

    T&       back();
    const T& back() const;

    void push_back(const T& value);
    void push_back(T&& value);
    void pop_back();

    template<class... Args>
    iterator emplace(iterator position, Args&&... args);

    template<class... Args>
    void emplace_back(Args&&... args);

    iterator insert(iterator position, const T& value);
    iterator insert(iterator position, T&& value);

    void append(const T* range, size_t n);
    
    iterator erase(iterator position);
    iterator erase(iterator first, iterator last);
    // Same as erase, except it doesn't preserve order, but is faster because it
    // simply copies the last item in the vector over the erased position.
    iterator erase_unsorted(iterator position);

    void clear() noexcept;
    // This is a unilateral reset to an initially empty state. No destructors
    // are called, no deallocation occurs.
    void reset_lose_memory() noexcept;

 protected:
    void DoAssign(const_iterator first, const_iterator last);

    template<typename... Args>
    void DoInsertValue(iterator position, Args&&... args);

    void DoInsertValuesEnd(size_t n); // Default constructs n values

    void DoInsertValuesEnd(const T* range, size_t n);
    
    template<typename... Args>
    void DoInsertValueEnd(Args&&... args);

    void DoClearCapacity();

    void DoGrow(size_t n);

    void DoSwap(vector<T>& x);
};




///////////////////////////////////////////////////////////////////////
// memory helpers
///////////////////////////////////////////////////////////////////////

template<typename T>
inline T* move(T* first, T* last, T* dest) {
    for (; first != last; ++first, ++dest) {
        *dest = move_(*first);
    }
    return dest;
}

template<typename T>
inline T* uninitialized_move(T* first, T* last, T* dest) {
    for (; first != last; ++dest, ++first) {
        new (static_cast<void*>(dest)) T(move_(*first));
    }
    return dest;
}

template<typename T, typename Count>
inline void uninitialized_default_fill_n(T* first, Count n) {
    for (; n > 0; --n, ++first) {
        new (static_cast<void*>(first)) T;
    }
}

template<typename T>
inline void destruct(T* first, T* last) {
    for (; first != last; ++first) {
        (*first).~T();
    }
}

template<typename T>
inline T* uninitialized_copy(T* first, T* last, T* dest) {
    for (; first != last; ++dest, ++first) {
        new (static_cast<void*>(dest)) T(*first);
    }
    return dest;
}

template<typename T>
inline T* move_backward(T* first, T* last, T* resultEnd) {
    for (; last != first; --resultEnd, --last) {
        *resultEnd = move_(*last);
    }
    return resultEnd;
}




///////////////////////////////////////////////////////////////////////
// vector
///////////////////////////////////////////////////////////////////////

template<typename T>
inline T* vector<T>::DoAllocate(size_t n) {
    // Make sure we are not allocating 2GB of data.
    assert_(n < 0x80000000 / sizeof(T));

    // If n is zero, then we allocate no memory and just return nullptr.
    // This is fine, as our default ctor initializes with NULL pointers.
    if (likely(n)) {
        auto* p = (T*)malloc(n * sizeof(T));
        assert_(p != nullptr);
        return p;
    }
    else {
        return nullptr;
    }
}


template<typename T>
inline void vector<T>::DoFree(T* p) {
    if (p) {
        free(p);
    }
}


template<typename T>
inline size_t vector<T>::GetNewCapacity(size_t currentCapacity) {
    // This needs to return a value of at least currentCapacity and at least 1.
    return (currentCapacity > 0) ? (2 * currentCapacity) : 1;
}




template<typename T>
inline vector<T>::vector()
        : mpBegin(NULL),
          mpEnd(NULL),
          mCapacity(NULL) {}


template<typename T>
inline vector<T>::vector(size_t n) {
    mpBegin   = DoAllocate(n);
    mpEnd     = mpBegin;
    mCapacity = mpBegin + n;

    uninitialized_default_fill_n(mpBegin, n);
    mpEnd = mpBegin + n;
}


template<typename T>
inline vector<T>::vector(const vector<T>& x) {
    mpBegin   = DoAllocate(x.size());
    mpEnd     = mpBegin;
    mCapacity = mpBegin + x.size();

    mpEnd = uninitialized_copy(x.mpBegin, x.mpEnd, mpBegin);
}


template<typename T>
inline vector<T>::vector(vector<T>&& x)
        : mpBegin(nullptr),
          mpEnd(nullptr),
          mCapacity(nullptr) {
    DoSwap(x);
}


template<typename T>
inline vector<T>::~vector() {
    destruct(mpBegin, mpEnd);
    free(mpBegin);
}


template<typename T>
vector<T>& vector<T>::operator=(const vector<T>& x) {
    assert_(false && "Copying a vector by value.");
    if (this != &x) {
        DoAssign(x.begin(), x.end());
    }

    return *this;
}


template<typename T>
vector<T>& vector<T>::operator=(vector<T>&& x) {
    if (this != &x) {
        swap(x);
    }
    return *this;
}


template<typename T>
inline typename vector<T>::iterator
vector<T>::begin() noexcept {
    return mpBegin;
}


template<typename T>
inline typename vector<T>::const_iterator
vector<T>::begin() const noexcept {
    return mpBegin;
}


template<typename T>
inline typename vector<T>::iterator
vector<T>::end() noexcept {
    return mpEnd;
}


template<typename T>
inline typename vector<T>::const_iterator
vector<T>::end() const noexcept {
    return mpEnd;
}


template<typename T>
bool vector<T>::empty() const noexcept {
    return (mpBegin == mpEnd);
}


template<typename T>
inline size_t
vector<T>::size() const noexcept {
    return (size_t)(mpEnd - mpBegin);
}


template<typename T>
inline size_t
vector<T>::capacity() const noexcept {
    return (size_t)(mCapacity - mpBegin);
}


template<typename T>
inline void vector<T>::resize(size_t n) {
    if (n > (size_t)(mpEnd - mpBegin)) { // We expect that more often than not, resizes will be upsizes.
        DoInsertValuesEnd(n - ((size_t) (mpEnd - mpBegin)));
    }
    else {
        destruct(mpBegin + n, mpEnd);
        mpEnd = mpBegin + n;
    }
}


template<typename T>
void vector<T>::reserve(size_t n) {
    // If the user wants to reduce the reserved memory, there is the set_capacity function.
    if (n > size_t(mCapacity - mpBegin)) {
        // If n > capacity ...
        DoGrow(n);
    }
}


template<typename T>
inline T* vector<T>::data() noexcept {
    return mpBegin;
}


template<typename T>
inline const T* vector<T>::data() const noexcept {
    return mpBegin;
}


template<typename T>
inline T& vector<T>::operator[](size_t n) {
    assert_(n < static_cast<size_t>(mpEnd - mpBegin));

    return *(mpBegin + n);
}


template<typename T>
inline const T& vector<T>::operator[](size_t n) const {
    assert_(n < static_cast<size_t>(mpEnd - mpBegin));

    return *(mpBegin + n);
}


template<typename T>
inline T& vector<T>::at(size_t n) {
    // The difference between at() and operator[] is it signals
    // the requested position is out of range by throwing an
    // out_of_range exception.

    assert_(n < static_cast<size_t>(mpEnd - mpBegin));

    return *(mpBegin + n);
}


template<typename T>
inline const T& vector<T>::at(size_t n) const {
    assert_(n < static_cast<size_t>(mpEnd - mpBegin));

    return *(mpBegin + n);
}


template<typename T>
inline T& vector<T>::front() {
    assert_(mpEnd > mpBegin);

    return *mpBegin;
}


template<typename T>
inline const T& vector<T>::front() const {
    assert_(mpEnd > mpBegin);

    return *mpBegin;
}


template<typename T>
inline T& vector<T>::back() {
    assert_(mpEnd > mpBegin);

    return *(mpEnd - 1);
}


template<typename T>
inline const T& vector<T>::back() const {
    assert_(mpEnd > mpBegin);

    return *(mpEnd - 1);
}


template<typename T>
inline void vector<T>::push_back(const T& value) {
    if (mpEnd < mCapacity) {
        new (static_cast<void*>(mpEnd++)) T(value);
    }
    else {
        DoInsertValueEnd(value);
    }
}


template<typename T>
inline void vector<T>::push_back(T&& value) {
    if (mpEnd < mCapacity) {
        new (static_cast<void*>(mpEnd++)) T(move_(value));
    }
    else {
        DoInsertValueEnd(move_(value));
    }
}


template<typename T>
inline void vector<T>::pop_back() {
    assert_(mpEnd > mpBegin);

    --mpEnd;
    mpEnd->~T();
}


template<typename T>
inline void vector<T>::append(const T* range, size_t n) {
    DoInsertValuesEnd(range, n);
}


template<typename T>
template<class... Args>
inline typename vector<T>::iterator
vector<T>::emplace(iterator position, Args&&... args) {
    const T* n = position - mpBegin; // Save this because we might reallocate.

    if ((mpEnd == mCapacity) || (position != mpEnd)) {
        DoInsertValue(position, forward_<Args>(args)...);
    }
    else {
        new (static_cast<void*>(mpEnd)) T(forward_<Args>(args)...);
        ++mpEnd; // Increment this after the construction above in case the construction throws an exception.
    }

    return mpBegin + n;
}

template<typename T>
template<class... Args>
inline void vector<T>::emplace_back(Args&&... args) {
    if (mpEnd < mCapacity) {
        new (static_cast<void*>(mpEnd)) T(forward_<Args>(args)...);  // If T has a move constructor, it will use it and this operation may be faster than otherwise.
        ++mpEnd; // Increment this after the construction above in case the construction throws an exception.
    }
    else {
        DoInsertValueEnd(forward_<Args>(args)...);
    }
}


template<typename T>
inline typename vector<T>::iterator
vector<T>::insert(iterator position, const T& value) {
    assert_(position >= mpBegin && position <= mpEnd);

    // We implment a quick pathway for the case that the insertion position is
    // at the end and we have free capacity for it.
    
    // Save this because we might reallocate.
    size_t n = position - mpBegin;

    if ((mpEnd == mCapacity) || (position != mpEnd)) {
        DoInsertValue(position, value);
    }
    else {
        new (static_cast<void*>(mpEnd)) T(value);
        
        // Increment this after the construction above in case the construction
        // throws an exception.
        ++mpEnd;
    }

    return mpBegin + n;
}


template<typename T>
inline typename vector<T>::iterator
vector<T>::insert(iterator position, T&& value) {
    return emplace(position, move_(value));
}


template<typename T>
inline typename vector<T>::iterator
vector<T>::erase(iterator position) {
    assert_(position >= mpBegin && position < mpEnd);
    
    if ((position + 1) < mpEnd) {
        move(position + 1, mpEnd, position);
    }
 
    --mpEnd;
    mpEnd->~T();
    
    return position;
}


template<typename T>
inline typename vector<T>::iterator
vector<T>::erase(iterator first, iterator last) {
    assert_(first >= mpBegin && first < mpEnd);
    assert_(last >= first && last < mpEnd);

    if ((last + 1) < mpEnd) {
        move(last + 1, mpEnd, first);
    }
    for (auto it = mpEnd - last + first; it != mpEnd; ++it) {
        it->~T();
    }
    mpEnd -= last - first;

    return first;
}


template<typename T>
inline typename vector<T>::iterator
vector<T>::erase_unsorted(iterator position) {
    assert_(position >= mpBegin && position < mpEnd);

    *position = move_(*(mpEnd - 1));

    // pop_back();
    --mpEnd;
    mpEnd->~T();

    return position;
}


template<typename T>
inline void vector<T>::clear() noexcept {
    destruct(mpBegin, mpEnd);
    mpEnd = mpBegin;
}


template<typename T>
inline void vector<T>::reset_lose_memory() noexcept {
// The reset function is a special extension function which unilaterally
// resets the container to an empty state without freeing the memory of
// the contained objects. This is useful for very quickly tearing down a
// container built into scratch memory.
    mpBegin = mpEnd = mCapacity = NULL;
}


// swap exchanges the contents of two containers. With respect to the containers allocators,
// the C11++ Standard (23.2.1/7) states that the behavior of a call to a container's swap function
// is undefined unless the objects being swapped have allocators that compare equal or
// allocator_traits<allocator_type>::propagate_on_container_swap::value is true (propagate_on_container_swap
// is false by default). EASTL doesn't have allocator_traits and so this doesn't directly apply,
// but EASTL has the effective behavior of propagate_on_container_swap = false for all allocators.
template<typename T>
inline void vector<T>::swap(vector<T>& x) {
    // NOTE(rparolin): The previous implementation required T to be copy-constructible in the fall-back case where
    // allocators with unique instances copied elements.  This was an unnecessary restriction and prevented the common
    // usage of vector with non-copyable types (eg. eastl::vector<non_copyable> or eastl::vector<unique_ptr>).
    //
    // The previous implementation violated the following requirements of vector::swap so the fall-back code has
    // been removed.  EASTL implicitly defines 'propagate_on_container_swap = false' therefore the fall-back case is
    // undefined behaviour.  We simply swap the contents and the allocator as that is the common expectation of
    // users and does not put the container into an invalid state since it can not free its memory via its current
    // allocator instance.
    //
    // http://en.cppreference.com/w/cpp/container/vector/swap
    // "Exchanges the contents of the container with those of other. Does not invoke any move, copy, or swap
    // operations on individual elements."
    //
    // http://en.cppreference.com/w/cpp/concept/AllocatorAwareContainer
    // "Swapping two containers with unequal allocators if propagate_on_container_swap is false is undefined
    // behavior."

    DoSwap(x);
}


template<typename T>
inline void vector<T>::DoAssign(const_iterator first, const_iterator last) {
    iterator position(mpBegin);

    while ((position != mpEnd) && (first != last)) {
        *position = *first;
        ++first;
        ++position;
    }
    if (first == last) {
        erase(position, mpEnd);
    }
    else {
        DoInsertValuesEnd(first, last - first);
    }
}


template<typename T>
void vector<T>::DoClearCapacity() { // This function exists because set_capacity() currently indirectly requires T to be default-constructible,
                                    // and some functions that need to clear our capacity (e.g. operator=) aren't supposed to require default-constructibility.
    clear();
    vector<T> temp(move_(*this));  // This is the simplest way to accomplish this,
    swap(temp);             // and it is as efficient as any other.
}


template<typename T>
void vector<T>::DoGrow(size_t n) {
    T* pNewData = DoAllocate(n);

    T* pNewEnd = uninitialized_move(mpBegin, mpEnd, pNewData);

    for (auto first = mpBegin, last = mpEnd; first != last; ++first) {
        first->~T();
    }
    DoFree(mpBegin);

    mpBegin   = pNewData;
    mpEnd     = pNewEnd;
    mCapacity = pNewData + n;
}


template<typename T>
inline void vector<T>::DoSwap(vector<T>& x) {
    swap_(mpBegin,   x.mpBegin);
    swap_(mpEnd,     x.mpEnd);
    swap_(mCapacity, x.mCapacity);
}


template<typename T>
void vector<T>::DoInsertValuesEnd(size_t n) {
    if (n > size_t(mCapacity - mpEnd)) {
        size_t nPrevSize = size_t(mpEnd - mpBegin);
        size_t nGrowSize = GetNewCapacity(nPrevSize);
        size_t nNewSize  = max_(nGrowSize, nPrevSize + n);
        T*     pNewData  = DoAllocate(nNewSize);

        T* pNewEnd = uninitialized_move(mpBegin, mpEnd, pNewData);

        uninitialized_default_fill_n(pNewEnd, n);
        pNewEnd += n;

        destruct(mpBegin, mpEnd);
        DoFree(mpBegin);

        mpBegin = pNewData;
        mpEnd = pNewEnd;
        mCapacity = pNewData + nNewSize;
    }
    else {
        uninitialized_default_fill_n(mpEnd, n);
        mpEnd += n;
    }
}


template<typename T>
void vector<T>::DoInsertValuesEnd(const T* range, size_t n) {
    if (n > size_t(mCapacity - mpEnd)) {
        size_t nPrevSize = size_t(mpEnd - mpBegin);
        size_t nGrowSize = GetNewCapacity(nPrevSize);
        size_t nNewSize = max_(nGrowSize, nPrevSize + n);
        T* pNewData = DoAllocate(nNewSize);
        T* pNewEnd = pNewData;

        pNewEnd = uninitialized_move(mpBegin, mpEnd, pNewEnd);
        pNewEnd = uninitialized_copy(const_cast<T*>(range), const_cast<T*>(range + n), pNewEnd);

        destruct(mpBegin, mpEnd);
        DoFree(mpBegin);

        mpBegin = pNewData;
        mpEnd = pNewEnd;
        mCapacity = pNewData + nNewSize;
    }
    else {
        mpEnd = uninitialized_copy(const_cast<T*>(range), const_cast<T*>(range + n), mpEnd);
    }
}


template<typename T>
template<typename... Args>
void vector<T>::DoInsertValue(iterator position, Args&&... args) {
    // To consider: It's feasible that the args is from a T comes from within the current sequence itself and
    // so we need to be sure to handle that case. This is different from insert(position, const T&) because in
    // this case value is potentially being modified.

    assert_(position >= mpBegin && position <= mpEnd);

    if (mpEnd != mCapacity) { // If size < capacity ...
        // We need to take into account the possibility that args is a T that comes from within the vector itself.
        // creating a temporary value on the stack here is not an optimal way to solve this because sizeof(T) may be
        // too much for the given platform. An alternative solution may be to specialize this function for the case of the
        // argument being const T& or T&&.
        assert_(position < mpEnd);                                // While insert at end() is valid, our design is such that calling code should handle that case before getting here, as our streamlined logic directly doesn't handle this particular case due to resulting negative ranges.
        T value(forward_<Args>(args)...);           // Need to do this before the move_backward below because maybe args refers to something within the moving range.
        new (static_cast<void*>(mpEnd)) T(move_(*(mpEnd - 1)));      // mpEnd is uninitialized memory, so we must construct into it instead of move into it like we do with the other elements below.
        move_backward(position, mpEnd - 1, mpEnd);           // We need to go backward because of potential overlap issues.
        position->~T();
        new (static_cast<void*>(position)) T(move_(value));                             // Move the value argument to the given position.
        ++mpEnd;
    }
    else { // else (size == capacity)
        size_t nPosSize  = size_t(position - mpBegin); // Index of the insertion position.
        size_t nPrevSize = size_t(mpEnd - mpBegin);
        size_t nNewSize  = GetNewCapacity(nPrevSize);
        T*     pNewData  = DoAllocate(nNewSize);

        new (static_cast<void*>((pNewData + nPosSize))) T(forward_<Args>(args)...);                  // Because the old data is potentially being moved rather than copied, we need to move
        T* pNewEnd = uninitialized_move(mpBegin, position, pNewData);   // the value first, because it might possibly be a reference to the old data being moved.
        pNewEnd = uninitialized_move(position, mpEnd, ++pNewEnd);            // Question: with exceptions disabled, do we asssume all operations are noexcept and thus there's no need for uninitialized_move_ptr_if_noexcept?

        destruct(mpBegin, mpEnd);
        DoFree(mpBegin);

        mpBegin   = pNewData;
        mpEnd     = pNewEnd;
        mCapacity = pNewData + nNewSize;
    }
}


template<typename T>
template<typename... Args>
void vector<T>::DoInsertValueEnd(Args&&... args) {
    size_t nPrevSize = size_t(mpEnd - mpBegin);
    size_t nNewSize  = GetNewCapacity(nPrevSize);
    T*     pNewData  = DoAllocate(nNewSize);

    T* pNewEnd = uninitialized_move(mpBegin, mpEnd, pNewData);
    new (static_cast<void*>(pNewEnd)) T(forward_<Args>(args)...);
    pNewEnd++;

    destruct(mpBegin, mpEnd);
    DoFree(mpBegin);

    mpBegin   = pNewData;
    mpEnd     = pNewEnd;
    mCapacity = pNewData + nNewSize;
}


///////////////////////////////////////////////////////////////////////
// global operators
///////////////////////////////////////////////////////////////////////

template<typename T>
inline bool operator==(const vector<T>& a, const vector<T>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (auto abeg = a.begin(), bbeg = b.begin(), aend = a.end(); abeg != aend; ++abeg, ++bbeg) {
        if (!(*abeg == *bbeg)) {
            return false;
        }
    }
    return true;
}


template<typename T>
inline bool operator!=(const vector<T>& a, const vector<T>& b) {
    if (a.size() != b.size()) {
        return true;
    }
    for (auto abeg = a.begin(), bbeg = b.begin(), aend = a.end(); abeg != aend; ++abeg, ++bbeg) {
        if (!(*abeg == *bbeg)) {
            return true;
        }
    }
    return false;
}

#endif  // SRC_UTIL_VECTOR_H_
