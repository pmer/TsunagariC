///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
// Copyright 2017 Paul Merrill
///////////////////////////////////////////////////////////////////////////////

#ifndef SRC_UTIL_VECTOR_H_
#define SRC_UTIL_VECTOR_H_


#include <stddef.h>
#include <stdlib.h>

#include <utility>

#include <new>

#include "util/assert.h"
#include "util/likely.h"
#include "util/move.h"


/// vector
///
/// Implements a dynamic array.
///
template <typename T>
class vector {
    typedef size_t       size_type;

 protected:
    T* mpBegin;
    T* mpEnd;
    T* mCapacity;

 protected:
    T*        DoAllocate(size_type n);
    void      DoFree(T* p);
    size_type GetNewCapacity(size_type currentCapacity);

 private:
    typedef vector<T>   this_type;

 public:
    typedef T           value_type;
    typedef T*          pointer;
    typedef const T*    const_pointer;
    typedef T&          reference;
    typedef const T&    const_reference;  // Maintainer note: We want to leave iterator defined as T* -- at least in release builds -- as this gives some algorithms an advantage that optimizers cannot get around.
    typedef T*          iterator;         // Note: iterator is simply T* right now, but this will likely change in the future, at least for debug builds.
    typedef const T*    const_iterator;   //       Do not write code that relies on iterator being T*. The reason it will

 public:
    vector();
    explicit vector(size_type n);
    vector(size_type n, const value_type& value);
    vector(const this_type& x);
    vector(this_type&& x);

    ~vector();

    this_type& operator=(const this_type& x);
    this_type& operator=(this_type&& x);

    void swap(this_type& x);

    iterator       begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;

    iterator       end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;

    bool      empty() const noexcept;
    size_type size() const noexcept;
    size_type capacity() const noexcept;

    void reserve(size_type n);

    pointer       data() noexcept;
    const_pointer data() const noexcept;

    reference       operator[](size_type n);
    const_reference operator[](size_type n) const;

    reference       at(size_type n);
    const_reference at(size_type n) const;

    reference       front();
    const_reference front() const;

    reference       back();
    const_reference back() const;

    void      push_back(const value_type& value);
    reference push_back();
    void      push_back(value_type&& value);
    void      pop_back();

    template<class... Args>
    iterator emplace(const_iterator position, Args&&... args);

    template<class... Args>
    void emplace_back(Args&&... args);

    iterator insert(const_iterator position, const value_type& value);
    iterator insert(const_iterator position, value_type&& value);

    iterator erase(const_iterator position);
    iterator erase_unsorted(const_iterator position);         // Same as erase, except it doesn't preserve order, but is faster because it simply copies the last item in the vector over the erased position.

    void clear() noexcept;
    void reset_lose_memory() noexcept;                       // This is a unilateral reset to an initially empty state. No destructors are called, no deallocation occurs.

 protected:
    struct should_copy_tag{}; struct should_move_tag : public should_copy_tag{};

    template <typename InputIterator>
    void DoAssign(InputIterator first, InputIterator last);

    template<typename... Args>
    void DoInsertValue(const_iterator position, Args&&... args);


    template<typename... Args>
    void DoInsertValueEnd(Args&&... args);

    void DoInsertValueEnd(const value_type& value);

    void DoClearCapacity();

    void DoGrow(size_type n);

    void DoSwap(this_type& x);

}; // class vector





template <typename T>
inline T* move(T* first, T* last, T* dest) {
    while(first != last)
        *dest++ = move_(*first++);
    return dest;
}

template <typename T>
inline T* uninitialized_move(T* first, T* last, T* dest) {
    while(first != last) {
        ::new ((void*)dest++) T(move_(*first++));
    }

    return dest;
}


///////////////////////////////////////////////////////////////////////
// vector
///////////////////////////////////////////////////////////////////////

template <typename T>
inline T* vector<T>::DoAllocate(size_type n) {
    // Make sure we are not allocating 2GB of data.
    assert_(n < 0x80000000 / sizeof(T));

    // If n is zero, then we allocate no memory and just return nullptr.
    // This is fine, as our default ctor initializes with NULL pointers.
    if(likely(n)) {
        auto* p = (T*)malloc(n * sizeof(T));
        assert_(p != nullptr);
        return p;
    }
    else {
        return nullptr;
    }
}


template <typename T>
inline void vector<T>::DoFree(T* p) {
    if(p) {
        delete[] (char *) p;
    }
}


template <typename T>
inline typename vector<T>::size_type
vector<T>::GetNewCapacity(size_type currentCapacity) {
    // This needs to return a value of at least currentCapacity and at least 1.
    return (currentCapacity > 0) ? (2 * currentCapacity) : 1;
}




template <typename T>
inline vector<T>::vector()
        : mpBegin(NULL),
          mpEnd(NULL),
          mCapacity(NULL) {
    // Empty
}


template <typename T>
inline vector<T>::vector(size_type n) {
    mpBegin   = DoAllocate(n);
    mpEnd     = mpBegin;
    mCapacity = mpBegin + n;

    T* dest = mpBegin;
    for (; n > 0; --n, ++dest) {
        new (static_cast<void*>(dest)) T;
    }

    mpEnd = mpBegin + n;
}


template <typename T>
inline vector<T>::vector(const this_type& x) {
    mpBegin   = DoAllocate(x.size());
    mpEnd     = mpBegin;
    mCapacity = mpBegin + x.size();

    iterator first = x.mpBegin;
    iterator last = x.mpEnd;
    iterator dest = mpBegin;

    for(; first != last; ++first, ++dest) {
        new(static_cast<void*>(&*dest)) value_type(*first);
    }

    mpEnd = dest;
}


template <typename T>
inline vector<T>::vector(this_type&& x)
        : mpBegin(NULL),
          mpEnd(NULL),
          mCapacity(NULL) {
    DoSwap(x);
}


template <typename T>
inline vector<T>::~vector() {
    for (T* x = mpBegin; x != mpEnd; x++) {
        (*x).~T();
    }
    free(mpBegin);
}


template <typename T>
typename vector<T>::this_type&
vector<T>::operator=(const this_type& x) {
    if(this != &x) {
        DoAssign<const_iterator>(x.begin(), x.end());
    }

    return *this;
}


template <typename T>
typename vector<T>::this_type&
vector<T>::operator=(this_type&& x) {
    if(this != &x) {
        swap(x);
    }
    return *this;
}


template <typename T>
inline typename vector<T>::iterator
vector<T>::begin() noexcept {
    return mpBegin;
}


template <typename T>
inline typename vector<T>::const_iterator
vector<T>::begin() const noexcept {
    return mpBegin;
}


template <typename T>
inline typename vector<T>::const_iterator
vector<T>::cbegin() const noexcept {
    return mpBegin;
}


template <typename T>
inline typename vector<T>::iterator
vector<T>::end() noexcept {
    return mpEnd;
}


template <typename T>
inline typename vector<T>::const_iterator
vector<T>::end() const noexcept {
    return mpEnd;
}


template <typename T>
inline typename vector<T>::const_iterator
vector<T>::cend() const noexcept {
    return mpEnd;
}


template <typename T>
bool vector<T>::empty() const noexcept {
    return (mpBegin == mpEnd);
}


template <typename T>
inline typename vector<T>::size_type
vector<T>::size() const noexcept {
    return (size_type)(mpEnd - mpBegin);
}


template <typename T>
inline typename vector<T>::size_type
vector<T>::capacity() const noexcept {
    return (size_type)(mCapacity - mpBegin);
}


template <typename T>
void vector<T>::reserve(size_type n) {
    // If the user wants to reduce the reserved memory, there is the set_capacity function.
    if(n > size_type(mCapacity - mpBegin)) {
        // If n > capacity ...
        DoGrow(n);
    }
}


template <typename T>
inline typename vector<T>::pointer
vector<T>::data() noexcept {
    return mpBegin;
}


template <typename T>
inline typename vector<T>::const_pointer
vector<T>::data() const noexcept {
    return mpBegin;
}


template <typename T>
inline typename vector<T>::reference
vector<T>::operator[](size_type n) {
    assert_(n < static_cast<size_type>(mpEnd - mpBegin));

    return *(mpBegin + n);
}


template <typename T>
inline typename vector<T>::const_reference
vector<T>::operator[](size_type n) const {
    assert_(n < static_cast<size_type>(mpEnd - mpBegin));

    return *(mpBegin + n);
}


template <typename T>
inline typename vector<T>::reference
vector<T>::at(size_type n) {
    // The difference between at() and operator[] is it signals
    // the requested position is out of range by throwing an
    // out_of_range exception.

    assert_(n < static_cast<size_type>(mpEnd - mpBegin));

    return *(mpBegin + n);
}


template <typename T>
inline typename vector<T>::const_reference
vector<T>::at(size_type n) const {
    assert_(n < static_cast<size_type>(mpEnd - mpBegin));

    return *(mpBegin + n);
}


template <typename T>
inline typename vector<T>::reference
vector<T>::front() {
    assert_(mpEnd > mpBegin);

    return *mpBegin;
}


template <typename T>
inline typename vector<T>::const_reference
vector<T>::front() const {
    assert_(mpEnd > mpBegin);

    return *mpBegin;
}


template <typename T>
inline typename vector<T>::reference
vector<T>::back() {
    assert_(mpEnd > mpBegin);

    return *(mpEnd - 1);
}


template <typename T>
inline typename vector<T>::const_reference
vector<T>::back() const {
    assert_(mpEnd > mpBegin);

    return *(mpEnd - 1);
}


template <typename T>
inline void vector<T>::push_back(const value_type& value) {
    if(mpEnd < mCapacity) {
        new ((void*)mpEnd++) value_type(value);
    }
    else {
        DoInsertValueEnd(value);
    }
}


template <typename T>
inline void vector<T>::push_back(value_type&& value) {
    if (mpEnd < mCapacity) {
        ::new((void*)mpEnd++) value_type(move_(value));
    }
    else {
        DoInsertValueEnd(move_(value));
    }
}


template <typename T>
inline typename vector<T>::reference
vector<T>::push_back() {
    if(mpEnd < mCapacity) {
        ::new((void*)mpEnd++) value_type();
    }
    else {  // Note that in this case we create a temporary, which is less desirable.
        DoInsertValueEnd(value_type());
    }

    return *(mpEnd - 1); // Same as return back();
}


template <typename T>
inline void vector<T>::pop_back() {
    assert_(mpEnd > mpBegin);

    --mpEnd;
    mpEnd->~value_type();
}


template <typename T>
template<class... Args>
inline typename vector<T>::iterator
vector<T>::emplace(const_iterator position, Args&&... args) {
    const ptrdiff_t n = position - mpBegin; // Save this because we might reallocate.

    if((mpEnd == mCapacity) || (position != mpEnd)) {
        DoInsertValue(position, forward_<Args>(args)...);
    }
    else {
        ::new((void*)mpEnd) value_type(forward_<Args>(args)...);
        ++mpEnd; // Increment this after the construction above in case the construction throws an exception.
    }

    return mpBegin + n;
}

template <typename T>
template<class... Args>
inline void vector<T>::emplace_back(Args&&... args) {
    if(mpEnd < mCapacity) {
        ::new((void*)mpEnd) value_type(forward_<Args>(args)...);  // If value_type has a move constructor, it will use it and this operation may be faster than otherwise.
        ++mpEnd; // Increment this after the construction above in case the construction throws an exception.
    }
    else {
        DoInsertValueEnd(forward_<Args>(args)...);
    }
}


template <typename T>
inline typename vector<T>::iterator
vector<T>::insert(const_iterator position, const value_type& value) {
    assert_(position >= mpBegin && position <= mpEnd);

    // We implment a quick pathway for the case that the insertion position is at the end and we have free capacity for it.
    const ptrdiff_t n = position - mpBegin; // Save this because we might reallocate.

    if((mpEnd == mCapacity) || (position != mpEnd)) {
        DoInsertValue(position, value);
    }
    else {
        ::new((void*)mpEnd) value_type(value);
        ++mpEnd; // Increment this after the construction above in case the construction throws an exception.
    }

    return mpBegin + n;
}


template <typename T>
inline typename vector<T>::iterator
vector<T>::insert(const_iterator position, value_type&& value) {
    return emplace(position, move_(value));
}


template <typename T>
inline typename vector<T>::iterator
vector<T>::erase(const_iterator position) {
    assert_(position >= mpBegin && position < mpEnd);

    // C++11 stipulates that position is const_iterator, but the return value is iterator.
    iterator destPosition = const_cast<value_type*>(position);

    if((position + 1) < mpEnd) {
        move(destPosition + 1, mpEnd, destPosition);
    }
    --mpEnd;
    mpEnd->~value_type();
    return destPosition;
}


template <typename T>
inline typename vector<T>::iterator
vector<T>::erase_unsorted(const_iterator position) {
    assert_(position >= mpBegin && position < mpEnd);

    // C++11 stipulates that position is const_iterator, but the return value is iterator.
    iterator destPosition = const_cast<value_type*>(position);
    *destPosition = move_(*(mpEnd - 1));

    // pop_back();
    --mpEnd;
    mpEnd->~value_type();

    return destPosition;
}


template <typename T>
inline void vector<T>::clear() noexcept {
    for(auto first = mpBegin, last = mpEnd; first != last; ++first) {
        (*first).~T();
    }
    mpEnd = mpBegin;
}


template <typename T>
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
template <typename T>
inline void vector<T>::swap(this_type& x) {
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


template <typename T>
template <typename InputIterator>
inline void vector<T>::DoAssign(InputIterator first, InputIterator last) {
    iterator position(mpBegin);

    while((position != mpEnd) && (first != last)) {
        *position = *first;
        ++first;
        ++position;
    }
    if(first == last) {
        erase(position, mpEnd);
    }
    else {
        insert(mpEnd, first, last);
    }
}


template <typename T>
void vector<T>::DoClearCapacity() { // This function exists because set_capacity() currently indirectly requires value_type to be default-constructible,
                                    // and some functions that need to clear our capacity (e.g. operator=) aren't supposed to require default-constructibility.
    clear();
    this_type temp(move_(*this));  // This is the simplest way to accomplish this,
    swap(temp);             // and it is as efficient as any other.
}


template <typename T>
void vector<T>::DoGrow(size_type n) {
    pointer const pNewData = DoAllocate(n);

    pointer pNewEnd = uninitialized_move(mpBegin, mpEnd, pNewData);

    for(auto first = mpBegin, last = mpEnd; first != last; ++first) {
        (*first).~T();
    }
    DoFree(mpBegin);

    mpBegin   = pNewData;
    mpEnd     = pNewEnd;
    mCapacity = pNewData + n;
}


template <typename T>
inline void vector<T>::DoSwap(this_type& x) {
    swap_(mpBegin,   x.mpBegin);
    swap_(mpEnd,     x.mpEnd);
    swap_(mCapacity, x.mCapacity); // We do this even if EASTL_ALLOCATOR_COPY_ENABLED is 0.
}


template <typename T>
template<typename... Args>
void vector<T>::DoInsertValue(const_iterator position, Args&&... args) {
    // To consider: It's feasible that the args is from a value_type comes from within the current sequence itself and
    // so we need to be sure to handle that case. This is different from insert(position, const value_type&) because in
    // this case value is potentially being modified.

    assert_(position >= mpBegin && position <= mpEnd);

    // C++11 stipulates that position is const_iterator, but the return value is iterator.
    iterator destPosition = const_cast<value_type*>(position);

    if(mpEnd != mCapacity) { // If size < capacity ...
        // We need to take into account the possibility that args is a value_type that comes from within the vector itself.
        // creating a temporary value on the stack here is not an optimal way to solve this because sizeof(value_type) may be
        // too much for the given platform. An alternative solution may be to specialize this function for the case of the
        // argument being const value_type& or value_type&&.
        assert_(position < mpEnd);                                // While insert at end() is valid, our design is such that calling code should handle that case before getting here, as our streamlined logic directly doesn't handle this particular case due to resulting negative ranges.
        value_type  value(forward_<Args>(args)...);           // Need to do this before the move_backward below because maybe args refers to something within the moving range.
        ::new(static_cast<void*>(mpEnd)) value_type(move_(*(mpEnd - 1)));      // mpEnd is uninitialized memory, so we must construct into it instead of move into it like we do with the other elements below.

        // We need to go backward because of potential overlap issues.
        auto first = destPosition, last = mpEnd - 1, resultEnd = mpEnd;
        while (last != first) {
            *--resultEnd = move_(*--last);
        }

        destPosition->~T();
        ::new(static_cast<void*>(destPosition)) value_type(move_(value));                             // Move the value argument to the given position.
        ++mpEnd;
    }
    else { // else (size == capacity)
        const size_type nPosSize  = size_type(destPosition - mpBegin); // Index of the insertion position.
        const size_type nPrevSize = size_type(mpEnd - mpBegin);
        const size_type nNewSize  = GetNewCapacity(nPrevSize);
        pointer const   pNewData  = DoAllocate(nNewSize);

        ::new((void*)(pNewData + nPosSize)) value_type(forward_<Args>(args)...);                  // Because the old data is potentially being moved rather than copied, we need to move
        pointer pNewEnd = uninitialized_move(mpBegin, destPosition, pNewData);   // the value first, because it might possibly be a reference to the old data being moved.
        pNewEnd = uninitialized_move(destPosition, mpEnd, ++pNewEnd);            // Question: with exceptions disabled, do we asssume all operations are noexcept and thus there's no need for uninitialized_move_ptr_if_noexcept?

        for(auto first = mpBegin, last = mpEnd; first != last; ++first) {
            (*first).~T();
        }
        DoFree(mpBegin);

        mpBegin   = pNewData;
        mpEnd     = pNewEnd;
        mCapacity = pNewData + nNewSize;
    }
}


template <typename T>
template<typename... Args>
void vector<T>::DoInsertValueEnd(Args&&... args) {
    const size_type nPrevSize = size_type(mpEnd - mpBegin);
    const size_type nNewSize  = GetNewCapacity(nPrevSize);
    pointer const   pNewData  = DoAllocate(nNewSize);

    pointer pNewEnd = uninitialized_move(mpBegin, mpEnd, pNewData);
    ::new((void*)pNewEnd) value_type(forward_<Args>(args)...);
    pNewEnd++;

    for(auto first = mpBegin, last = mpEnd; first != last; ++first) {
        (*first).~T();
    }
    DoFree(mpBegin);

    mpBegin   = pNewData;
    mpEnd     = pNewEnd;
    mCapacity = pNewData + nNewSize;
}


template <typename T>
void vector<T>::DoInsertValueEnd(const value_type& value) {
    const size_type nPrevSize = size_type(mpEnd - mpBegin);
    const size_type nNewSize  = GetNewCapacity(nPrevSize);
    pointer const   pNewData  = DoAllocate(nNewSize);

    pointer pNewEnd = uninitialized_move(mpBegin, mpEnd, pNewData);
    ::new((void*)pNewEnd) value_type(value);
    pNewEnd++;

    for(auto first = mpBegin, last = mpEnd; first != last; ++first) {
        (*first).~T();
    }
    DoFree(mpBegin);

    mpBegin   = pNewData;
    mpEnd     = pNewEnd;
    mCapacity = pNewData + nNewSize;
}



///////////////////////////////////////////////////////////////////////
// global operators
///////////////////////////////////////////////////////////////////////

template <typename T>
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


template <typename T>
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
