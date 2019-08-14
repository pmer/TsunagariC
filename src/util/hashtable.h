/********************************
** Tsunagari Tile Engine       **
** hashtable.h                 **
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

// Original source downloaded from: https://github.com/Tessil/hopscotch-map

// Copyright (c) 2018 Tessil
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef SRC_UTIL_HASHTABLE_H_
#define SRC_UTIL_HASHTABLE_H_

#include "os/c.h"
#include "util/algorithm.h"
#include "util/assert.h"
#include "util/constexpr.h"
#include "util/hash.h"
#include "util/int.h"
#include "util/list.h"
#include "util/meta.h"
#include "util/move.h"
#include "util/new.h"
#include "util/noexcept.h"
#include "util/optional.h"
#include "util/vector.h"

namespace hopscotch {
    /**
     * Grow the hash table by a factor of 2 keeping the bucket count to a power
     * of two. It allows the table to use a mask operation instead of a modulo
     * operation to map a hash to a bucket.
     */
    class GrowthPolicy {
     public:
        /**
         * Called on the hash table creation and on rehash. The number of
         * buckets for the table is passed in parameter. This number is a
         * minimum, the policy may update this value with a higher value if
         * needed (but not lower).
         *
         * If 0 is given, minBucketCountInOut must still be 0 after the policy
         * creation and bucketForHash must always return 0 in this case.
         */
        explicit GrowthPolicy(size_t& minBucketCountInOut) noexcept {
            if (minBucketCountInOut > 0) {
                minBucketCountInOut = roundUpToPowerOfTwo(minBucketCountInOut);
                mMask = minBucketCountInOut - 1;
            }
            else {
                mMask = 0;
            }
        }

        /**
         * Return the bucket [0, bucketCount()) to which the hash belongs.
         * If bucketCount() is 0, it must always return 0.
         */
        size_t bucketForHash(size_t hash) const noexcept {
            return hash & mMask;
        }

        /**
         * Return the bucket count to use when the bucket array grows on rehash.
         */
        size_t nextBucketCount() const noexcept { return (mMask + 1) * 2; }

        /**
         * Reset the growth policy as if it was created with a bucket count of
         * 0. After a clear, the policy must always return 0 when bucketForHash
         * is called.
         */
        void clear() noexcept { mMask = 0; }

     private:
        static size_t roundUpToPowerOfTwo(size_t value) noexcept {
            if (isPowerOfTwo(value)) {
                return value;
            }

            if (value == 0) {
                return 1;
            }

            --value;
            for (size_t i = 1; i < sizeof(size_t) * 8; i *= 2) {
                value |= value >> i;
            }

            return value + 1;
        }

        static CONSTEXPR11 bool isPowerOfTwo(size_t value) noexcept {
            return value != 0 && (value & (value - 1)) == 0;
        }

     private:
        size_t mMask;
    };


    /*
     * Each bucket may store up to three elements:
     * - An aligned storage to store a KV object with placement-new.
     * - An unsigned integer of type NeighborhoodBitmap used to tell us which
     * buckets in the neighborhood of the current bucket contain a value with a
     * hash belonging to the current bucket.
     *
     * For a bucket 'bct', a bit 'i' (counting from 0 and from the least
     * significant bit to the most significant) set to 1 means that the bucket
     * 'bct + i' contains a value with a hash belonging to bucket 'bct'. The
     * bits used for that, start from the third least significant bit. The two
     * least significant bits are reserved:
     * - The least significant bit is set to 1 if there is a value in the bucket
     * storage.
     * - The second least significant bit is set to 1 if there is an overflow.
     * More than NeighborhoodSize values give the same hash, all overflow values
     * are stored in the mOverflowElements list of the map.
     *
     * Details regarding hopscotch hashing an its implementation can be found
     * here: https://tessil.github.io/2016/08/29/hopscotch-hashing.html
     */
    static CONSTEXPR11 size_t NB_RESERVED_BITS_IN_NEIGHBORHOOD = 2;


    template<typename V, unsigned int NeighborhoodSize> class Bucket {
     private:
        static_assert(NeighborhoodSize >= 4,
                      "NeighborhoodSize should be >= 4.");
        static_assert(NeighborhoodSize <= 62,
                      "NeighborhoodSize should be <= 62.");

     public:
        using NeighborhoodBitmap = uint64_t;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26495)
#endif
        Bucket() noexcept : mNeighborhoodInfos(0) {}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

        Bucket(const Bucket& b) noexcept : mNeighborhoodInfos(0) {
            if (!b.empty()) {
                ::new (mValue) V(b.value());
            }

            mNeighborhoodInfos = b.mNeighborhoodInfos;
        }

        Bucket(Bucket&& b) noexcept : mNeighborhoodInfos(0) {
            if (!b.empty()) {
                ::new (mValue) V(move_(b.value()));
            }

            mNeighborhoodInfos = b.mNeighborhoodInfos;
        }

        Bucket& operator=(const Bucket& b) noexcept {
            if (this != &b) {
                removeValue();

                if (!b.empty()) {
                    ::new (mValue) V(b.value());
                }

                mNeighborhoodInfos = b.mNeighborhoodInfos;
            }

            return *this;
        }

        Bucket& operator=(Bucket&&) = delete;

        ~Bucket() noexcept {
            if (!empty()) {
                destroyValue();
            }
        }

        NeighborhoodBitmap neighborhoodInfos() const noexcept {
            return NeighborhoodBitmap(mNeighborhoodInfos >>
                                      NB_RESERVED_BITS_IN_NEIGHBORHOOD);
        }

        void setOverflow(bool hasOverflow) noexcept {
            if (hasOverflow) {
                mNeighborhoodInfos = NeighborhoodBitmap(mNeighborhoodInfos | 2);
            }
            else {
                mNeighborhoodInfos =
                        NeighborhoodBitmap(mNeighborhoodInfos & ~2);
            }
        }

        bool hasOverflow() const noexcept {
            return (mNeighborhoodInfos & 2) != 0;
        }

        bool empty() const noexcept { return (mNeighborhoodInfos & 1) == 0; }

        void toggleNeighborPresence(size_t ineighbor) noexcept {
            assert_(ineighbor <= NeighborhoodSize);
            mNeighborhoodInfos = NeighborhoodBitmap(
                    mNeighborhoodInfos ^
                    (1ull << (ineighbor + NB_RESERVED_BITS_IN_NEIGHBORHOOD)));
        }

        bool checkNeighborPresence(size_t ineighbor) const noexcept {
            assert_(ineighbor <= NeighborhoodSize);
            if (((mNeighborhoodInfos >>
                  (ineighbor + NB_RESERVED_BITS_IN_NEIGHBORHOOD)) &
                 1) == 1) {
                return true;
            }

            return false;
        }

        V& value() noexcept {
            assert_(!empty());
            return *reinterpret_cast<V*>(mValue);
        }

        const V& value() const noexcept {
            assert_(!empty());
            return *reinterpret_cast<const V*>(mValue);
        }

        template<typename... Args>
        void setValueOfEmptyBucket(Args&&... valueArgs) noexcept {
            assert_(empty());

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 6011)
#endif
            ::new (mValue) V{forward_<Args>(valueArgs)...};
#ifdef _MSC_VER
#pragma warning(pop)
#endif

            setEmpty(false);
        }

        void swapValueIntoEmptyBucket(Bucket& emptyBucket) noexcept {
            assert_(emptyBucket.empty());
            if (!empty()) {
                ::new (static_cast<void*>(&emptyBucket.mValue))
                        V(move_(value()));

                destroyValue();
                setEmpty(true);
            }
        }

        void removeValue() noexcept {
            if (!empty()) {
                destroyValue();
                setEmpty(true);
            }
        }

        void clear() noexcept {
            if (!empty()) {
                destroyValue();
            }

            mNeighborhoodInfos = 0;
            assert_(empty());
        }

     private:
        void setEmpty(bool isEmpty) noexcept {
            if (isEmpty) {
                mNeighborhoodInfos =
                        NeighborhoodBitmap(mNeighborhoodInfos & ~1);
            }
            else {
                mNeighborhoodInfos = NeighborhoodBitmap(mNeighborhoodInfos | 1);
            }
        }

        void destroyValue() noexcept {
            assert_(!empty());
            value().~V();
        }

     private:
        NeighborhoodBitmap mNeighborhoodInfos;
        alignas(alignof(V)) char mValue[sizeof(V)];
    };
}  // namespace hopscotch


/**
 * Implementation of a hash map using the hopscotch hashing algorithm.
 *
 * The size of the neighborhood (NeighborhoodSize) must be > 0 and <= 62.
 *
 * Iterators invalidation:
 *  - clear, operator=, reserve: always invalidate the iterators.
 *  - insert, operator[]: if there is an effective insert, invalidate the
 * iterators if a displacement is needed to resolve a collision (which mean that
 * most of the time, insert will invalidate the iterators). Or if there is a
 * rehash.
 *  - erase: iterator on the erased element is the only one which become
 * invalid.
 */
template<class K, class V, unsigned int NeighborhoodSize = 62>
class Hashmap : private hopscotch::GrowthPolicy {
 private:
    struct KV {
        K k;
        V v;
    };

    static CONSTEXPR11 bool HasValue = !IsUnit<V>::value;

    typedef List<KV> OverflowContainer;

 public:
    template<bool IsConst> class HashmapIterator;

    using Iterator = HashmapIterator<false>;
    using ConstIterator = HashmapIterator<true>;

 private:
    using Bucket = hopscotch::Bucket<KV, NeighborhoodSize>;
    using NeighborhoodBitmap = typename Bucket::NeighborhoodBitmap;

    using BucketsContainer = Vector<Bucket>;

    using BucketsIterator = typename BucketsContainer::iterator;
    using ConstBucketIterator = typename BucketsContainer::const_iterator;

    using OverflowIterator = typename OverflowContainer::Iterator;
    using ConstOverflowIterator = typename OverflowContainer::ConstIterator;

 public:
    template<bool IsConst> class HashmapIterator {
        friend class Hashmap;

     private:
        using BucketIterator = If<IsConst,
                                  typename Hashmap::ConstBucketIterator,
                                  typename Hashmap::BucketsIterator>;
        using OverflowIterator = If<IsConst,
                                    typename Hashmap::ConstOverflowIterator,
                                    typename Hashmap::OverflowIterator>;


        HashmapIterator(BucketIterator bucketsIterator,
                        BucketIterator bucketsEndIterator,
                        OverflowIterator overflowIterator) noexcept
                : mBucketsIterator(bucketsIterator),
                  mBucketsEndIterator(bucketsEndIterator),
                  mOverflowIterator(overflowIterator) {}

     public:
        HashmapIterator() noexcept = default;
        HashmapIterator(const HashmapIterator& other) = default;
        HashmapIterator(HashmapIterator&& other) = default;
        HashmapIterator& operator=(const HashmapIterator& other) = default;
        HashmapIterator& operator=(HashmapIterator&& other) = default;

        const K& key() const noexcept {
            if (mBucketsIterator != mBucketsEndIterator) {
                return mBucketsIterator->value().k;
            }

            return mOverflowIterator->k;
        }

        If<IsConst, const V&, V&> value() const noexcept {
            static_assert(HasValue, "");

            if (mBucketsIterator != mBucketsEndIterator) {
                return mBucketsIterator->value().v;
            }

            return mOverflowIterator->v;
        }

        HashmapIterator& operator++() noexcept {
            if (mBucketsIterator == mBucketsEndIterator) {
                ++mOverflowIterator;
                return *this;
            }

            do {
                ++mBucketsIterator;
            } while (mBucketsIterator != mBucketsEndIterator &&
                     mBucketsIterator->empty());

            return *this;
        }

        HashmapIterator operator++(int) noexcept {
            HashmapIterator tmp(*this);
            ++*this;

            return tmp;
        }

        friend bool operator==(const HashmapIterator& lhs,
                               const HashmapIterator& rhs) noexcept {
            return lhs.mBucketsIterator == rhs.mBucketsIterator &&
                   lhs.mOverflowIterator == rhs.mOverflowIterator;
        }

        friend bool operator!=(const HashmapIterator& lhs,
                               const HashmapIterator& rhs) noexcept {
            return !(lhs == rhs);
        }

     private:
        BucketIterator mBucketsIterator;
        BucketIterator mBucketsEndIterator;
        OverflowIterator mOverflowIterator;
    };

 public:
    Hashmap(size_t bucketCount = 0) noexcept
            : GrowthPolicy(bucketCount),
              mBucketsData(bucketCount + NeighborhoodSize - 1),
              mOverflowElements(),
              mBuckets(staticEmptyBucketPtr()),
              mNumElements(0),
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26451)
#endif
              mMaxLoadThresholdRehash(
                      size_t(float(bucketCount) * MAX_LOAD_FACTOR)),
              mMinLoadThresholdRehash(
                      size_t(float(bucketCount) * MIN_LOAD_FACTOR_FOR_REHASH)) {
#ifdef _MSC_VER
#pragma warning(pop)
#endif
        if (bucketCount > 0) {
            mBuckets = mBucketsData.data();
        }
    }

    Hashmap(const Hashmap& other) noexcept
            : GrowthPolicy(other),
              mBucketsData(other.mBucketsData),
              mOverflowElements(other.mOverflowElements),
              mBuckets(other.mBuckets),
              mNumElements(other.mNumElements),
              mMaxLoadThresholdRehash(other.mMaxLoadThresholdRehash),
              mMinLoadThresholdRehash(other.mMinLoadThresholdRehash) {}

    Hashmap(Hashmap&& other) noexcept
            : GrowthPolicy(move_(static_cast<GrowthPolicy&>(other))),
              mBucketsData(move_(other.mBucketsData)),
              mOverflowElements(move_(other.mOverflowElements)),
              mBuckets(other.mBuckets),
              mNumElements(other.mNumElements),
              mMaxLoadThresholdRehash(other.mMaxLoadThresholdRehash),
              mMinLoadThresholdRehash(other.mMinLoadThresholdRehash) {
        other.GrowthPolicy::clear();
        other.mBucketsData.clear();
        other.mOverflowElements.clear();
        other.mBuckets = staticEmptyBucketPtr();
        other.mNumElements = 0;
        other.mMaxLoadThresholdRehash = 0;
        other.mMinLoadThresholdRehash = 0;
    }

    Hashmap& operator=(const Hashmap& other) noexcept {
        if (&other != this) {
            GrowthPolicy::operator=(other);
            mBucketsData = other.mBucketsData;
            mOverflowElements = other.mOverflowElements;
            mBuckets = other.mBuckets;
            mNumElements = other.mNumElements;
            mMaxLoadThresholdRehash = other.mMaxLoadThresholdRehash;
            mMinLoadThresholdRehash = other.mMinLoadThresholdRehash;
        }

        return *this;
    }

    Hashmap& operator=(Hashmap&& other) noexcept {
        other.swap(*this);
        other.clear();

        return *this;
    }


    /*
     * Iterators
     */
    Iterator begin() noexcept {
        auto begin = mBucketsData.begin();
        while (begin != mBucketsData.end() && begin->empty()) {
            ++begin;
        }

        return Iterator(begin, mBucketsData.end(), mOverflowElements.begin());
    }

    ConstIterator begin() const noexcept {
        auto begin = mBucketsData.begin();
        while (begin != mBucketsData.end() && begin->empty()) {
            ++begin;
        }

        return ConstIterator(
                begin, mBucketsData.end(), mOverflowElements.begin());
    }

    Iterator end() noexcept {
        return Iterator(mBucketsData.end(),
                        mBucketsData.end(),
                        mOverflowElements.end());
    }

    ConstIterator end() const noexcept {
        return ConstIterator(mBucketsData.end(),
                             mBucketsData.end(),
                             mOverflowElements.end());
    }


    /*
     * Capacity
     */
    bool empty() const noexcept { return mNumElements == 0; }

    size_t size() const noexcept { return mNumElements; }

    /*
     * Modifiers
     */
    void clear() noexcept {
        for (auto& b : mBucketsData) {
            b.clear();
        }

        mOverflowElements.clear();
        mNumElements = 0;
    }


    bool insert(const KV& value) noexcept { return insertImpl(value); }

    bool insert(KV&& value) noexcept { return insertImpl(move_(value)); }


    /**
     * Here to avoid `template<class K> size_t erase(const K& key)` being used
     * when we use an Iterator instead of a ConstIterator.
     */
    Iterator erase(Iterator pos) noexcept {
        size_t ibucketForHash = bucketForHash(hash_(pos.key()));

        if (pos.mBucketsIterator != pos.mBucketsEndIterator) {
            auto itBucket = mBucketsData.begin() + pos.mBucketsIterator -
                            mBucketsData.begin();
            eraseFromBucket(*itBucket, ibucketForHash);

            return ++Iterator(
                    itBucket, mBucketsData.end(), mOverflowElements.begin());
        }
        else {
            auto itNextOverflow =
                    eraseFromOverflow(pos.mOverflowIterator, ibucketForHash);
            return Iterator(
                    mBucketsData.end(), mBucketsData.end(), itNextOverflow);
        }
    }

    bool erase(const K& key) noexcept { return erase(key, hash_(key)); }

    bool erase(const K& key, size_t hash) noexcept {
        size_t ibucketForHash = bucketForHash(hash);

        Bucket* bucketFound = findInBuckets(key, mBuckets + ibucketForHash);
        if (bucketFound != nullptr) {
            eraseFromBucket(*bucketFound, ibucketForHash);

            return true;
        }

        if (mBuckets[ibucketForHash].hasOverflow()) {
            OverflowIterator itOverflow = findInOverflow(key);
            if (itOverflow != mOverflowElements.end()) {
                eraseFromOverflow(itOverflow, ibucketForHash);

                return true;
            }
        }

        return false;
    }

    void swap(Hashmap& other) noexcept {
        swap_(static_cast<GrowthPolicy&>(*this),
              static_cast<GrowthPolicy&>(other));
        swap_(mBucketsData, other.mBucketsData);
        swap_(mOverflowElements, other.mOverflowElements);
        swap_(mBuckets, other.mBuckets);
        swap_(mNumElements, other.mNumElements);
        swap_(mMaxLoadThresholdRehash, other.mMaxLoadThresholdRehash);
        swap_(mMinLoadThresholdRehash, other.mMinLoadThresholdRehash);
    }

    /*
     * Lookup
     */
    V& at(const K& key) noexcept {
        static_assert(HasValue, "");

        return at(key, hash_(key));
    }

    V& at(const K& key, size_t hash) noexcept {
        static_assert(HasValue, "");

        return const_cast<V&>(static_cast<const Hashmap*>(this)->at(key, hash));
    }


    Optional<const V&> at(const K& key) const noexcept {
        static_assert(HasValue, "");

        return at(key, hash_(key));
    }

    Optional<const V&> at(const K& key, size_t hash) const noexcept {
        static_assert(HasValue, "");

        const V* value =
                findValueImpl(key, hash, mBuckets + bucketForHash(hash));
        if (value == nullptr) {
            return none;
        }
        else {
            return Optional<const V&>(*value);
        }
    }


    template<class K2> V& operator[](K2&& key) noexcept {
        static_assert(HasValue, "");

        size_t hash = hash_(key);
        size_t ibucketForHash = bucketForHash(hash);

        V* value = findValueImpl(key, mBuckets + ibucketForHash);
        if (value != nullptr) {
            return *value;
        }
        else {
            return insertValue(ibucketForHash, hash, forward_<K2>(key), V())
                    .value();
        }
    }


    size_t contains(const K& key) const noexcept {
        return contains(key, hash_(key));
    }

    size_t contains(const K& key, size_t hash) const noexcept {
        return containsImpl(key, mBuckets + bucketForHash(hash));
    }


    Iterator find(const K& key) noexcept { return find(key, hash_(key)); }

    Iterator find(const K& key, size_t hash) {
        return findImpl(key, mBuckets + bucketForHash(hash));
    }


    ConstIterator find(const K& key) const noexcept {
        return find(key, hash_(key));
    }

    ConstIterator find(const K& key, size_t hash) const noexcept {
        return findImpl(key, mBuckets + bucketForHash(hash));
    }


    /*
     *  Hash policy
     */
    void rehash(size_t count_) noexcept {
        count_ = max_(count_, size_t(ceilf(float(size()) / MAX_LOAD_FACTOR)));
        rehashImpl(count_);
    }

    void reserve(size_t count_) noexcept {
        rehash(size_t(ceilf(float(count_) / MAX_LOAD_FACTOR)));
    }


    friend bool operator==(const Hashmap& lhs, const Hashmap& rhs) noexcept {
        if (lhs.size() != rhs.size()) {
            return false;
        }

        for (const auto& elementLhs : lhs) {
            const auto itElementRhs = rhs.find(elementLhs);
            if (itElementRhs == rhs.end()) {
                return false;
            }
        }

        return true;
    }

    friend bool operator!=(const Hashmap& lhs, const Hashmap& rhs) noexcept {
        return !operator==(lhs, rhs);
    }


 private:
    size_t bucketForHash(size_t hash) const noexcept {
        size_t b = GrowthPolicy::bucketForHash(hash);
        assert_(b < mBucketsData.size() || (b == 0 && mBucketsData.empty()));

        return b;
    }

    void rehashImpl(size_t count_) noexcept {
        Hashmap newMap(count_);

        if (!mOverflowElements.empty()) {
            newMap.mOverflowElements = move_(mOverflowElements);
            newMap.mNumElements += newMap.mOverflowElements.size();

            for (const KV& value : newMap.mOverflowElements) {
                size_t ibucketForHash = newMap.bucketForHash(hash_(value.k));
                newMap.mBuckets[ibucketForHash].setOverflow(true);
            }
        }

        for (auto itBucket = mBucketsData.begin();
             itBucket != mBucketsData.end();
             ++itBucket) {
            if (itBucket->empty()) {
                continue;
            }

            size_t hash = hash_(itBucket->value().k);
            size_t ibucketForHash = newMap.bucketForHash(hash);

            newMap.insertValue(ibucketForHash, hash, move_(itBucket->value()));

            eraseFromBucket(*itBucket, bucketForHash(hash));
        }

        newMap.swap(*this);
    }

    OverflowIterator eraseFromOverflow(OverflowIterator pos,
                                       size_t ibucketForHash) noexcept {
        auto itNext = mOverflowElements.erase(pos);
        mNumElements--;

        // Check if we can remove the overflow flag
        assert_(mBuckets[ibucketForHash].hasOverflow());
        for (KV& kv : mOverflowElements) {
            size_t bucketForValue = bucketForHash(hash_(kv.k));
            if (bucketForValue == ibucketForHash) {
                return itNext;
            }
        }

        mBuckets[ibucketForHash].setOverflow(false);
        return itNext;
    }

    /**
     * bucketForValue is the bucket in which the value is.
     * ibucketForHash is the bucket where the value belongs.
     */
    void eraseFromBucket(Bucket& bucketForValue,
                         size_t ibucketForHash) noexcept {
        size_t ibucketForValue = &bucketForValue - mBucketsData.data();
        assert_(ibucketForValue >= ibucketForHash);

        bucketForValue.removeValue();
        mBuckets[ibucketForHash].toggleNeighborPresence(ibucketForValue -
                                                        ibucketForHash);
        mNumElements--;
    }


    template<typename P> bool insertImpl(P&& value) noexcept {
        size_t hash = hash_(value.k);
        size_t ibucketForHash = bucketForHash(hash);

        // Check if already presents
        auto itFind = findImpl(value.k, hash, mBuckets + ibucketForHash);
        if (itFind != end()) {
            return false;
        }

        insertValue(ibucketForHash, hash, forward_<P>(value));
        return true;
    }

    template<typename... Args>
    Iterator insertValue(size_t ibucketForHash,
                         size_t hash,
                         Args&&... valueArgs) noexcept {
        if ((mNumElements - mOverflowElements.size()) >=
            mMaxLoadThresholdRehash) {
            rehash(GrowthPolicy::nextBucketCount());
            ibucketForHash = bucketForHash(hash);
        }

        size_t ibucketEmpty = findEmptyBucket(ibucketForHash);
        if (ibucketEmpty < mBucketsData.size()) {
            do {
                assert_(ibucketEmpty >= ibucketForHash);

                // Empty bucket is in range of NeighborhoodSize, use it
                if (ibucketEmpty - ibucketForHash < NeighborhoodSize) {
                    auto it = insertInBucket(ibucketEmpty,
                                             ibucketForHash,
                                             hash,
                                             forward_<Args>(valueArgs)...);
                    return Iterator(
                            it, mBucketsData.end(), mOverflowElements.begin());
                }
            }
            // Else, try to swap values to get a closer empty bucket.
            while (swapEmptyBucketCloser(ibucketEmpty));
        }

        // Load factor is too low or a rehash will not change the neighborhood,
        // put the value in overflow list.
        if (size() < mMinLoadThresholdRehash ||
            !willNeighborhoodChangeOnRehash(ibucketForHash)) {
            auto it = insertInOverflow(ibucketForHash,
                                       forward_<Args>(valueArgs)...);
            return Iterator(mBucketsData.end(), mBucketsData.end(), it);
        }

        rehash(GrowthPolicy::nextBucketCount());
        ibucketForHash = bucketForHash(hash);

        return insertValue(ibucketForHash, hash, forward_<Args>(valueArgs)...);
    }

    /*
     * Return true if a rehash will change the position of a key-value in the
     * neighborhood of ibucketNeighborhoodCheck. In this case a rehash is
     * needed instead of puting the value in overflow list.
     */
    bool willNeighborhoodChangeOnRehash(size_t ibucketNeighborhoodCheck) const
            noexcept {
        size_t expandBucketCount = GrowthPolicy::nextBucketCount();
        GrowthPolicy expandGrowthPolicy(expandBucketCount);

        for (size_t ibucket = ibucketNeighborhoodCheck;
             ibucket < mBucketsData.size() &&
             (ibucket - ibucketNeighborhoodCheck) < NeighborhoodSize;
             ++ibucket) {
            assert_(!mBuckets[ibucket].empty());

            size_t hash = hash_(mBuckets[ibucket].value().k);
            if (bucketForHash(hash) != expandGrowthPolicy.bucketForHash(hash)) {
                return true;
            }
        }

        return false;
    }

    /*
     * Return the index of an empty bucket in mBucketsData.
     * If none, the returned index equals mBucketsData.size().
     */
    size_t findEmptyBucket(size_t ibucketStart) const noexcept {
        size_t limit = min_(ibucketStart + MAX_PROBES_FOR_EMPTY_BUCKET,
                            mBucketsData.size());
        for (; ibucketStart < limit; ibucketStart++) {
            if (mBuckets[ibucketStart].empty()) {
                return ibucketStart;
            }
        }

        return mBucketsData.size();
    }

    /*
     * Insert value in ibucketEmpty where value originally belongs to
     * ibucketForHash.
     *
     * Return bucket iterator to ibucketEmpty.
     */
    template<typename... Args>
    BucketsIterator insertInBucket(size_t ibucketEmpty,
                                   size_t ibucketForHash,
                                   size_t hash,
                                   Args&&... valueArgs) noexcept {
        assert_(ibucketEmpty >= ibucketForHash);
        assert_(mBuckets[ibucketEmpty].empty());

        mBuckets[ibucketEmpty].setValueOfEmptyBucket(
                forward_<Args>(valueArgs)...);

        assert_(!mBuckets[ibucketForHash].empty());

        mBuckets[ibucketForHash].toggleNeighborPresence(ibucketEmpty -
                                                        ibucketForHash);
        mNumElements++;

        return mBucketsData.begin() + ibucketEmpty;
    }

    template<class... Args>
    OverflowIterator insertInOverflow(size_t ibucketForHash,
                                      Args&&... valueArgs) noexcept {
        auto it = mOverflowElements.emplace_back(forward_<Args>(valueArgs)...);

        mBuckets[ibucketForHash].setOverflow(true);
        mNumElements++;

        return it;
    }

    /*
     * Try to swap the bucket ibucketEmptyInOut with a bucket preceding it
     * while keeping the neighborhood conditions correct.
     *
     * If a swap was possible, the position of ibucketEmptyInOut will be
     * closer to 0 and true will re returned.
     */
    bool swapEmptyBucketCloser(size_t& ibucketEmptyInOut) noexcept {
        assert_(ibucketEmptyInOut >= NeighborhoodSize);
        size_t neighborhoodStart = ibucketEmptyInOut - NeighborhoodSize + 1;

        for (size_t toCheck = neighborhoodStart; toCheck < ibucketEmptyInOut;
             toCheck++) {
            NeighborhoodBitmap neighborhoodInfos =
                    mBuckets[toCheck].neighborhoodInfos();
            size_t toSwap = toCheck;

            while (neighborhoodInfos != 0 && toSwap < ibucketEmptyInOut) {
                if ((neighborhoodInfos & 1) == 1) {
                    assert_(mBuckets[ibucketEmptyInOut].empty());
                    assert_(!mBuckets[toSwap].empty());

                    mBuckets[toSwap].swapValueIntoEmptyBucket(
                            mBuckets[ibucketEmptyInOut]);

                    assert_(!mBuckets[toCheck].checkNeighborPresence(
                            ibucketEmptyInOut - toCheck));
                    assert_(mBuckets[toCheck].checkNeighborPresence(toSwap -
                                                                    toCheck));

                    mBuckets[toCheck].toggleNeighborPresence(ibucketEmptyInOut -
                                                             toCheck);
                    mBuckets[toCheck].toggleNeighborPresence(toSwap - toCheck);

                    ibucketEmptyInOut = toSwap;

                    return true;
                }

                toSwap++;
                neighborhoodInfos = NeighborhoodBitmap(neighborhoodInfos >> 1);
            }
        }

        return false;
    }


    V* findValueImpl(const K& key, Bucket* bucketForHash) noexcept {
        static_assert(HasValue, "");

        return const_cast<V*>(static_cast<const Hashmap*>(this)->findValueImpl(
                key, bucketForHash));
    }

    /*
     * Avoid the creation of an iterator to just get the value for operator[]
     * and at() in maps. Faster this way.
     *
     * Return null if no value for the key (TODO use Optional when available).
     */
    const V* findValueImpl(const K& key, const Bucket* bucketForHash) const
            noexcept {
        static_assert(HasValue, "");

        const Bucket* bucketFound = findInBuckets(key, bucketForHash);
        if (bucketFound != nullptr) {
            return &bucketFound->value().v;
        }

        if (bucketForHash->hasOverflow()) {
            auto itOverflow = findInOverflow(key);
            if (itOverflow != mOverflowElements.end()) {
                return &itOverflow->v;
            }
        }

        return nullptr;
    }

    size_t containsImpl(const K& key, const Bucket* bucketForHash) const
            noexcept {
        if (findInBuckets(key, bucketForHash) != nullptr) {
            return true;
        }
        return bucketForHash->hasOverflow() &&
               findInOverflow(key) != mOverflowElements.end();
    }

    Iterator findImpl(const K& key, Bucket* bucketForHash) noexcept {
        Bucket* bucketFound = findInBuckets(key, bucketForHash);
        if (bucketFound != nullptr) {
            return Iterator(
                    mBucketsData.begin() + (bucketFound - mBucketsData.data()),
                    mBucketsData.end(),
                    mOverflowElements.begin());
        }

        if (!bucketForHash->hasOverflow()) {
            return end();
        }

        return Iterator(
                mBucketsData.end(), mBucketsData.end(), findInOverflow(key));
    }

    ConstIterator findImpl(const K& key, const Bucket* bucketForHash) const
            noexcept {
        const Bucket* bucketFound = findInBuckets(key, bucketForHash);
        if (bucketFound != nullptr) {
            return ConstIterator(
                    mBucketsData.begin() + (bucketFound - mBucketsData.data()),
                    mBucketsData.end(),
                    mOverflowElements.begin());
        }

        if (!bucketForHash->hasOverflow()) {
            return end();
        }

        return ConstIterator(
                mBucketsData.end(), mBucketsData.end(), findInOverflow(key));
    }

    Bucket* findInBuckets(const K& key, Bucket* bucketForHash) noexcept {
        const Bucket* bucketFound =
                static_cast<const Hashmap*>(this)->findInBuckets(key,
                                                                 bucketForHash);
        return const_cast<Bucket*>(bucketFound);
    }

    /**
     * Return a pointer to the bucket which has the value, nullptr otherwise.
     */
    const Bucket* findInBuckets(const K& key, const Bucket* bucketForHash) const
            noexcept {
        NeighborhoodBitmap neighborhoodInfos =
                bucketForHash->neighborhoodInfos();
        while (neighborhoodInfos != 0) {
            if ((neighborhoodInfos & 1) == 1) {
                if (bucketForHash->value().k == key) {
                    return bucketForHash;
                }
            }

            ++bucketForHash;
            neighborhoodInfos = NeighborhoodBitmap(neighborhoodInfos >> 1);
        }

        return nullptr;
    }

    OverflowIterator findInOverflow(const K& key) noexcept {
        for (auto it = mOverflowElements.begin(); it != mOverflowElements.end();
             ++it) {
            if (key == it->k) {
                return it;
            }
        }
        return mOverflowElements.end();
    }

    ConstOverflowIterator findInOverflow(const K& key) const noexcept {
        for (auto it = mOverflowElements.begin(); it != mOverflowElements.end();
             ++it) {
            if (key == it->k) {
                return it;
            }
        }
        return mOverflowElements.end();
    }


 private:
    static constexpr float MAX_LOAD_FACTOR =
            (NeighborhoodSize <= 30) ? 0.8f : 0.9f;
    static constexpr size_t MAX_PROBES_FOR_EMPTY_BUCKET = 12 * NeighborhoodSize;
    static constexpr float MIN_LOAD_FACTOR_FOR_REHASH = 0.1f;

    /**
     * Return an always valid pointer to an static empty bucket.
     */
    Bucket* staticEmptyBucketPtr() noexcept {
        static Bucket emptyBucket;
        return &emptyBucket;
    }

 private:
    BucketsContainer mBucketsData;
    OverflowContainer mOverflowElements;

    /**
     * Points to mBucketsData() if !mBucketsData() otherwise points to
     * stateEmptyBucketPtr. This variable is useful to avoid the cost of
     * checking if mBucketsData is empty when trying to find an element.
     *
     * TODO Remove mBucketsData and only use a pointer+size instead of a
     * pointer+vector to save some space in the hashmap object.
     */
    Bucket* mBuckets;

    size_t mNumElements;

    /**
     * Max size of the hash table before a rehash occurs automatically to grow
     * the table.
     */
    size_t mMaxLoadThresholdRehash;

    /**
     * Min size of the hash table before a rehash can occurs automatically
     * (except if mMaxLoadThresholdRehash os reached). If the neighborhood of a
     * bucket is full before the min is reacher, the elements are put into
     * mOverflowElements.
     */
    size_t mMinLoadThresholdRehash;
};

/**
 * Implementation of a hash set using the hopscotch hashing algorithm.
 *
 * The size of the neighborhood (NeighborhoodSize) must be > 0 and <= 62.
 *
 * Iterators invalidation:
 *  - clear, operator=, reserve: always invalidate the iterators.
 *  - insert, operator[]: if there is an effective insert, invalidate the
 * iterators if a displacement is needed to resolve a collision (which mean that
 * most of the time, insert will invalidate the iterators). Or if there is a
 * rehash.
 *  - erase: iterator on the erased element is the only one which become
 * invalid.
 */
template<class K, unsigned int NeighborhoodSize = 62>
using Hashset = Hashmap<K, Unit, NeighborhoodSize>;

#endif  // SRC_UTIL_HASHTABLE_H_
