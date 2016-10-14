- Flat containers wording (DRAFT)
- Document number: D0460R0
- Date: 2016-10-12
- Reply-to: Sean Middleditch <sean@middleditch.us>
- Project: ISO JTC1/SC22/WG21: Programming Language C++
- Audience: Library Evolution Working Group

Summary
===

This proposal is a follow-up to
[P0038R0 - Flat Containers](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0038r0.html).
Flat containers are a heterogenous set of containers similar to `map` or `set` but which are not
node-based. The primary purposes are to achieve better performance for small-medium element
counts (a dozen to several hundreds) where CPU cache access patterns dominate performance. This
proposal answers outstanding questions in P0038R0 and provides proposed wording for `flat_map`,
`flat_set`, `flat_multimap`, and `flat_multiset`.

Design
===

Layout
---

Performance measurements have indicated that a sorted layout (as opposed to a level-ordered
layout) is faster for sorted flat containers, and lookup/insert are roughly equal between both
approaches for the sizes of data used with flat containers.

Measurements available as part of an alternative proposal:

http://pubby.github.io/proposal.html

The layout choice would only affect public interface if `flat_map::data()`, `flat_map::keys()`,
or `flat_map::values()` members were made available. Iterator access can ensure in-order
iteration no matter the internal layout choice.

Such members are not proposed here, though may be proposed as a future addition. The author
strongly recommends implementations to use a sorted layout.

Non-interleaved
---

Non-interleaved keys and values are faster for flat maps and multimaps for larger sizes of value
element, as illustrated in the previous link. The author has also received a number of requests
both in person and via e-mail asking for this proposal to require non-interleaved flat map
implementation. 

A `flat_map::data()` member, if added, would effectively mandate interleaved storage. A
`flat_map::keys()` or `flat_map::values()` member would mandate non-interleaved storage.

Non-interleaved access requires that elements be referenced as a pair of references rather
than a pair of value, which does have some pecularities not found in other standard
containers. In particular, the references in this pair will be invalidated on insert or
removal of elements; this is in addition to iterator invalidation. It essentially means that
the following code will invoke undefined behavior:

    auto element = *flat_container.find(key);
    flat_container.insert({new_key, new_value});
    auto value = element.second; // undefine behavior, element.second is invalidated

Given the prior feedback from committee review of P0038R0 suggesting that design decisions
focus on efficiency rather than compatibility with existing containers, this proposal will
require that flat map elements be represented as a pair of references.

Bulk insert
---

The author has received several very strong requests to ensure that a bulk insert is
available based on real-world usage scenarios from large projects, such as Chromium.

There are several possible forms of bulk insert. The first is a bulk insert of unsorted data,
suitable for raw input from an untrusted source. The second is a bulk insert of pre-sorted data,
which can be an efficiency improvement for certain use cases. The final is a bulk initialization
from already sorted data, which is the most efficient form of all.

Additional cases can be added as a pure extension. This proposal will only suggest the most
generic form of bulk insert that makes no assumptions about sorted status, which is easily
represented by the standard container member `flat_map::insert(first, second)` or corresponding
constructor.

Adaptor vs container
---

Pubby makes some strong arguments for making flat containers a set of adaptors. Namely, this
allows flat containers to layer over `vector` or the common `small_vector` types found in
many performance-sensitive applications.

Chandler Carruth in his CppCon 2016 talk "High Performance Code 201: Hybrid Data Structures"
makes a strong case for using real containers rather than using adaptors.

This paper does not propose any small flat containers, but such containers may be useful to
propose in the future for all the same reasons that small vectors are useful.

This paper proposes real containers and does not propose adaptors.

Wording
===

In chapter [containers] add:

    Flat associative containers [flatassoc]

    In general [flatassoc.general]

    The header <flat_map> defines the class templates flat_map and flat_multimap; the header
    <flat_set> defines the class templates flat_set and flat_multiset.

    Header <flat_map> synopsis [flatassoc.map.syn]

    namespace std {
        template <class Key, class T, class Compare = default_order_t<Key>,
                  class Allocator = allocator<pair<const Key&, T&>>>
        class flat_map;

        template <class Key, class T, class Compare, class Allocator>
        bool operator==(const flat_map<Key, T, Compare, Allocator>& x,
                        const flat_map<Key, T, Compare, Allocator>& y);
        template <class Key, class T, class Compare, class Allocator>
        bool operator< (const flat_map<Key, T, Compare, Allocator>& x,
                        const flat_map<Key, T, Compare, Allocator>& y);
        template <class Key, class T, class Compare, class Allocator>
        bool operator!=(const flat_map<Key, T, Compare, Allocator>& x,
                        const flat_map<Key, T, Compare, Allocator>& y);
        template <class Key, class T, class Compare, class Allocator>
        bool operator> (const flat_map<Key, T, Compare, Allocator>& x,
                        const flat_map<Key, T, Compare, Allocator>& y);
        template <class Key, class T, class Compare, class Allocator>
        bool operator>=(const flat_map<Key, T, Compare, Allocator>& x,
                        const flat_map<Key, T, Compare, Allocator>& y);
        template <class Key, class T, class Compare, class Allocator>
        bool operator<=(const flat_map<Key, T, Compare, Allocator>& x,
                        const flat_map<Key, T, Compare, Allocator>& y);
        template <class Key, class T, class Compare, class Allocator>
        void swap(flat_map<Key, T, Compare, Allocator>& x,
                  flat_map<Key, T, Compare, Allocator>& y)
                noexcept(noexcept(x.swap(y)));

        template <class Key, class T, class Compare = default_order_t<Key>,
                  class Allocator = allocator<pair<const Key&, T&>>>
        class flat_multimap;

        // TODO: comparison operators
    }

    Header <set> synopsis [flatassoc.set.syn]

    namespace std {
        template <class Key, class Compare = default_order_t<Key>,
                  class Allocator = allocator<Key>>
        class flat_set;

        // TODO: comparison operators

        template <class Key, class Compare = default_order_t<Key>,
                  class Allocator = allocator<Key>>
        class flat_multiset;

        // TODO: comparison operators
    }

    Class template flat_map [flat_map]

    Class template flat_map overview [flat_map.overview]

    A flat_map is an associative container that supports unique keys (contains at most one
    of each key value) and provides for retrieval of values of another type T based on the
    keys, as well as fast iteration of the contained elements. Storage for all keys is
    guaranteed to be contiguous in memory and storage for all values is guaranteed to be
    contiguous in memory. The flat_map class supports bidirectional iterators.

    A flat_map satisfies all the requirements of a container and of a reversible container.
    A flat_map also provides most operations of an associative container and most
    operations for unique keys. This means that a flat_map supports the a_uniq operations but
    not the a_eq operations. For a flat_map<Key, T> the key_type is Key and the value_type is
    pair<const Key&, T&>. Descriptions are provided here only for operations on flat_map that
    are not described in one of those tables or for operations where there is additional
    semantic operation.

    The basic exception guarantee is provided by flat_map. Objects of value_type retrieved
    from a flat_map contain references that are invalidated by any operation that invalidates
    iterators for the container.

    namespace std {
        template <class Key, class T, class Compare = default_order_t<Key>,
                  class Allocator = allocator<pair<const Key&, T&>>>
        class flat_map {
        public:
            using key_type = Key;
            using mapped_typed = T;
            using value_type = pair<const Key&, T&>;
            using key_compare = Compare;
            using allocator_type = Allocator;
            using size_type = implementation-defined;
            using difference_type = implementation-defined;
            using iterator = implementation-defined;
            using const_iterator = implementation-defined;
            using reverse_iterator = implementation-defined;
            using const_reverse_iterator = implementation-defined;

            flat_map() : flat_map(Compare()) {}
            explicit flat_map(const Compare& comp, const Allocator& = Allocator());
            template <class InputIterator, class Sentinel>
            flat_map(InputIterator first, Sentinel second, const Compare& comp = Compare{},
                     const Allocator& = Allocator{});
            flat_map(const flat_map&);
            flat_map(flat_map&&)
                noexcept(allocator_traits<Allocator>::is_always_equal::value &&
                         is_nothrow_move_assignable_v<Compare>);
            explicit flat_map(const Allocator&);
            flat_map(const flat_map&, const Allocator&);
            flat_map(flat_map&&, const Allocator&);
            template <class InputIterator, class Sentinel>
            flat_map(InputIterator first, Sentinel second, const Allocator& a = Allocator{}) :
                flat_map(first, second, Compare{}, a) {}
            ~flat_map();
            flat_map& operator=(flat_map const&);
            flat_map& operator=(flat_map&&)
                noexcept(allocator_traits<Allocator>::is_always_equal::value &&
                         is_nothrow_move_assignable_v<Compare>);
            allocator_type get_allocator() const noexcept;
            
            // iterators:
            iterator begin() noexcept;
            const_iterator begin() const noexcept;
            iterator end() noexcept;
            const_iterator end() const noexcept;

            reverse_iterator rbegin() noexcept;
            const_reverse_iterator rbegin() const noexcept;
            reverse_iterator rend() noexcept;
            const_reverse_iterator rend() const noexcept;

            const_iterator cbegin() const noexcept;
            const_iterator cend() const noexcept;
            const_reverse_iterator crbegin() const noexcept;
            const_reverse_iterator crend() const noexcept;

            // capacity:
            bool empty() const noexcept;
            size_type size() const noexcept;
            size_type max_size() const noexcept;

            // element access:
            T& operator[](const key_type& x);
            T& operator[](key_type&& x);
            T& at(const key_type& x);
            const T& at(const key_type& x) const;

            // modifiers:
            template <class... Args> pair<iterator, bool> emplace(Args&&... args);
            template <class... Args> iterator emplace_hint(const_iterator position,
                                                           Args&&... args);
            pair<iterator, bool> insert(const value_type& x);
            pair<iterator, bool> insert(value_type&& x);
            template <class P> pair<iterator, bool> insert(P&& x);
            iterator insert(const_iterator position, const value_type& x);
            iterator insert(const_iterator position, value_type&& x);
            template <class P> iterator insert(const_iterator position, P&&);
            template <class InputIterator, class Sentinel>
            void insert(InputIterator first, Sentinel second);

            template <class... Args>
            pair<iterator, bool> try_emplace(const key_type& k, Args&&... args);
            template <class... Args>
            pair<iterator, bool> try_emplace(key_type&& k, Args&&... args);
            template <class... Args>
            iterator try_emplace(const_iterator hint, const key_type& k, Args&&... args);
            template <class... Args>
            iterator try_emplace(const_iterator hint, key_type&& k, Args&&... args);
            template <class M>
            pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj);
            template <class M>
            pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj);
            template <class M>
            iterator insert_or_assign(const_iterator hint, const key_type& k, M&& obj);
            template <class M>
            iterator insert_or_assign(const_iterator hint, key_type&& k, M&& obj);

            iterator erase(iterator position);
            iterator erase(const_iterator position);
            size_type erase(const key_type& x);
            iterator erase(const_iterator first, const_iterator last);

            void swap(map&)
                noexcept(allocator_traits<Allocator>::is_always_equal::value &&
                         is_nothrow_swappable_v<Compare>);
            void clear() noexcept;

            // observers:
            key_compare key_comp() const;
            value_compare value_comp() const;

            // map operations:
            iterator find(const key_type& x);
            const_iterator find(const key_type& x) const;
            template <class K> iterator find(const K& x);
            template <class K> const_iterator find(const K& x) const;
            size_type count(const key_type& x) const;
            template <class K> size_type count(const K& x) const;
            iterator lower_bound(const key_type& x);
            const_iterator lower_bound(const key_type& x) const;
            template <class K> iterator lower_bound(const K& x);
            template <class K> const_iterator lower_bound(const K& x) const;
            iterator upper_bound(const key_type& x);
            const_iterator upper_bound(const key_type& x) const;
            template <class K> iterator upper_bound(const K& x);
            template <class K> const_iterator upper_bound(const K& x) const;
            pair<iterator, iterator> equal_range(const key_type& x);
            pair<const_iterator, const_iterator> equal_range(const key_type& x) const;
            template <class K>
            pair<iterator, iterator> equal_range(const K& x);
            template <class K>
            pair<const_iterator, const_iterator> equal_range(const K& x) const;
        }

        template <class Key, class T, class Compare, class Allocator>
        bool operator==(const flat_map<Key, T, Compare, Allocator>& x,
                        const flat_map<Key, T, Compare, Allocator>& y);
        template <class Key, class T, class Compare, class Allocator>
        bool operator< (const flat_map<Key, T, Compare, Allocator>& x,
                        const flat_map<Key, T, Compare, Allocator>& y);
        template <class Key, class T, class Compare, class Allocator>
        bool operator!=(const flat_map<Key, T, Compare, Allocator>& x,
                        const flat_map<Key, T, Compare, Allocator>& y);
        template <class Key, class T, class Compare, class Allocator>
        bool operator> (const flat_map<Key, T, Compare, Allocator>& x,
                        const flat_map<Key, T, Compare, Allocator>& y);
        template <class Key, class T, class Compare, class Allocator>
        bool operator>=(const flat_map<Key, T, Compare, Allocator>& x,
                        const flat_map<Key, T, Compare, Allocator>& y);
        template <class Key, class T, class Compare, class Allocator>
        bool operator<=(const flat_map<Key, T, Compare, Allocator>& x,
                        const flat_map<Key, T, Compare, Allocator>& y);

        // specialized algorithms:
        template <class Key, class T, class Compare, class Allocator>
        void swap(flat_map<Key, T, Compare, Allocator>& x,
                  flat_map<Key, T, Compare, Allocator>& y)
                noexcept(noexcept(x.swap(y)));
    }

    flat_map constructors, copy, and assignment [flat_map.cons]

    explicit flat_map(const Compare& comp, const Allocator& = Allocator{});

        Effects: Constructs an empty flat_map using the specified comparison objects
                 and allocator.
        Complexity: Constant.

    template <class InputIterator, class Sentinel>
    flat_map(InputIterator first, Sentinel second, const Compare& comp = Compare{},
             const Allocator& = Allocator{});
    
        Effects: Constructs an empty flat_map using the specified comparison objects and
                 allocator, and inserts elements from the range [first, last).
        Complexity: Linear in N if the range [first, last) is already sorted using comp and
                    otherwise N logN, where N is last - first.

    flat_map element access [flat_map.access]

    T& operator[](const key_type& x);

        Effects: Equivalent to: return try_emplace(x).first->second;
    
    T& operator[](key_type&& x);

        Effects: Equivalent to: return try_emplace(move(x)).first->second;

    T& at(const key_type& x);
    const T& at(const key_type& x) const;

        Returns: A reference to the mapped_type corresponding to x in *this.
        Effects: An exception object of type out_of_range if no such element is present.
        Complexity: Logarithmic.

    flat_map modifiers [flat_map.modifiers]

    template <class P> pair<iterator, bool> insert(P&& x);
    template <class P> iterator insert(const_iterator position, P&& x);

        Effects: The first form is equivalent to return emplace(forward<P>(x)). The second
                 form is equivalent to return emplace_hint(position, forward<P>(x)).
        Remarks: These signatures shall not participate in overload resolution unless
                 std::is_constructible_v<key_type, decltype(get<0>(forward<P>(x)))> and
                 std::is_constructible_v<mapped_type, decltype(get<1>(forward<P>(x)))> are true.
                 The references in objects of type value_type will be invalidated.

    template <class... Args>
    pair<iterator, bool> try_emplace(const key_type& k, Args&&... args);
    template <class... Args>
    iterator try_emplace(const_iterator hint, const key_type& k, Args&&... args);
    
        Requires: mapped_type shall be EmplaceConstructible into map from
                  forward_as_tuple(forward<Args>(args)...).
        Effects: If the flat_map already contains an element whose key is equivalent to k, there
                 is no effect. Otherwise inserts an object at the key of type mapped_type
                 constructed with forward_as_tuple(forward<Args>(args)...).
        Returns: In the first overload, the bool component of the returned pair is true if
                 and only if the insertion took place. The returned iterator points to the
                 flat_map element whose key is equivalent to k. The references in objects of type
                 value_type will be invalidated.
        Complexity: The same as emplace and emplace_hint, respectively.
    
    template <class... Args>
    pair<iterator, bool> try_emplace(key_type&& k, Args&&... args);
    template <class... Args>
    iterator try_emplace(const_iterator hint, key_type&& k, Args&&... args);
    
        Requires: mapped_type shall be EmplaceConstructible into map from
                  forward_as_tuple(forward<Args>(args)...).
        Effects: If the flat_map already contains an element whose key is equivalent to k, there
                 is no effect. Otherwise inserts an object at the key of type mapped_type
                 constructed with forward_as_tuple(forward<Args>(args)...).
        Returns: In the first overload, the bool component of the returned pair is true if
                 and only if the insertion took place. The returned iterator points to the
                 flat_map element whose key is equivalent to k. The references in objects of type
                 value_type will be invalidated.
        Complexity: The same as emplace and emplace_hint, respectively.
    
    template <class M>
    pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj);
    template <class M>
    iterator insert_or_assign(const_iterator hint, const key_type& k, M&& obj);
    
        Requires: is_assignable_v<mapped_type&, M&&> and is_constructible_v<mapped_type, M&&>
                  shall both be true. 
        Effects: If the map already contains an element e whose key is equivalent to k, assigns
                 forward<M>(obj) to e.second. Otherwise inserts an object at the key of type
                 mapped_type constructed with forward<M>(obj).
        Returns: In the first overload, the bool component of the returned pair is true if and
                 only if the insertion took place. The returned iterator points to the flat_map
                 element whose key is equivalent to k. The references in objects of type
                 value_type will be invalidated.
        Complexity: The same as emplace and emplace_hint, respectively.

    template <class M> pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj);
    template <class M> iterator insert_or_assign(const_iterator hint, key_type&& k, M&& obj);
    
        Requires: is_assignable_v<mapped_type&, M&&> and is_constructible_v<mapped_type, M&&>
                  shall both be true. 
        Effects: If the map already contains an element e whose key is equivalent to k, assigns
                 forward<M>(obj) to e.second. Otherwise inserts an object at the key of type
                 mapped_type constructed with forward<M>(obj).
        Returns: In the first overload, the bool component of the returned pair is true if and
                 only if the insertion took place. The returned iterator points to the flat_map
                 element whose key is equivalent to k. The references in objects of type
                 value_type will be invalidated.
        Complexity: The same as emplace and emplace_hint, respectively.

    flat_map specialized algorithms [flat_map.special]

    template <class Key, class T, class Compare, class Allocator>
    void swap(flat_map<Key, T, Compare, Allocator>& x,
              flat_map<Key, T, Compare, Allocator>& y)
        noexcept(noexcept(x.swap(y)));

        Effects: As if by x.swap(y).