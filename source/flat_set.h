// Copyright (C) 2014-2016 Sean Middleditch.

#if !defined(SG14_FLAT_SET_H__)
#define SG14_FLAT_SET_H__

#include <vector>
#include <algorithm>

namespace std
{
	template <typename, typename, typename> class flat_set;
}

template <typename ValueT, typename CompareT = std::less<>, typename AllocatorT = std::default_allocator>
class std::flat_set final
{
	using map_type = std::vector<ValueT, AllocatorT>;
	
	map_type _data;

	template <typename T, typename U>
	static T cast_iterator(U&& src) { return const_cast<T>(std::addressof(*src)); }

public:
	using value_type = ValueT;
	using iterator = typename map_type::iterator;
	using const_iterator = typename map_type::const_iterator;
	using size_type = size_t;

	flat_set() = default;

	iterator begin() { return _data.begin(); }
	const_iterator begin() const { return _data.cbegin(); }
	const_iterator cbegin() const { return _data.cbegin(); }

	iterator end() { return _data.end(); }
	const_iterator end() const { return _data.cend(); }
	const_iterator cend() const { return _data.cend(); }

	value_type const& at(size_t index) const { return _data[index]; }

	value_type const& back() const { return _data.back(); }
	void pop_back() { _data.pop_back(); }

	size_type size() const { return _data.size(); }
	bool empty() const { return _data.empty(); }

	template <typename FindT>
	inline iterator find(FindT const& value);
	inline iterator find(ValueT const& value) { return find<ValueT>(value); }

	template <typename FindT>
	inline const_iterator find(FindT const& value) const;
	inline const_iterator find(ValueT const& value) const { return find<ValueT>(value); }

	inline std::pair<iterator, bool> insert(value_type const& value);
	inline std::pair<iterator, bool> insert(value_type&& value);

	template <typename... P0toN>
	inline std::pair<iterator, bool> emplace(P0toN&&... params);

	inline iterator erase(const_iterator iter);
	inline iterator erase(value_type const& value);

	inline void clear();
};

template <typename ValueT, typename CompareT, typename AllocatorT>
template <typename FindT>
auto std::flat_set<ValueT, CompareT, AllocatorT>::find(FindT const& value) -> iterator
{
	auto compare = CompareT();
	auto const rng = std::lower_bound(_data, value, compare);
	if (!rng.empty() && !compare(value, rng.front()))
		return cast_iterator<iterator>(rng.begin());
	else
		return end();
}

template <typename ValueT, typename CompareT, typename AllocatorT>
template <typename FindT>
auto std::flat_set<ValueT, CompareT, AllocatorT>::find(FindT const& value) const -> const_iterator
{
	auto compare = CompareT();
	auto const rng = std::lower_bound(_data, value, compare);
	if (!rng.empty() && !compare(value, rng.front()))
		return cast_iterator<iterator>(rng.begin());
	else
		return end();
}

template <typename ValueT, typename CompareT, typename AllocatorT>
auto std::flat_set<ValueT, CompareT, AllocatorT>::insert(value_type const& value) -> std::pair<iterator, bool>
{
	auto compare = CompareT();
	auto const rng = std::lower_bound(_data, value, compare);
	if (!rng.empty() && !compare(value, rng.front()))
		return std::make_pair(cast_iterator<iterator>(rng.begin()), false);
	else
		return std::make_pair(cast_iterator<iterator>(_data.insert(rng.begin(), value)), true);
}

template <typename ValueT, typename CompareT, typename AllocatorT>
auto std::flat_set<ValueT, CompareT, AllocatorT>::insert(value_type&& value) -> std::pair<iterator, bool>
{
	auto compare = CompareT();
	auto const rng = std::lower_bound(_data, value, compare);
	if (!rng.empty() && !compare(value, rng.front()))
		return std::make_pair(cast_iterator<iterator>(rng.begin()), false);
	else
		return std::make_pair(cast_iterator<iterator>(_data.insert(rng.begin(), std::move(value))), true);
}

template <typename ValueT, typename CompareT, typename AllocatorT>
template <typename... P0toN>
auto std::flat_set<ValueT, CompareT, AllocatorT>::emplace(P0toN&&... params) -> std::pair<iterator, bool>
{
	auto value = ValueT(std::forward<P0toN>(params)...);
	auto compare = CompareT();
	auto const rng = std::lower_bound(_data, value, compare);
	if (!rng.empty() && !compare(value, rng.front()))
		return std::make_pair(cast_iterator<iterator>(rng.begin()), false);
	else
		return std::make_pair(cast_iterator<iterator>(_data.emplace(rng.begin(), std::move(value))), true);
}

template <typename ValueT, typename CompareT, typename AllocatorT>
auto std::flat_set<ValueT, CompareT, AllocatorT>::erase(const_iterator iter) -> iterator
{
	return _data.erase(iter);
}

template <typename ValueT, typename CompareT, typename AllocatorT>
auto std::flat_set<ValueT, CompareT, AllocatorT>::erase(value_type const& value) -> iterator
{
	auto it = find(value);
	if (it != end())
		return _data.erase(it);
	else
		return it;
}

template <typename ValueT, typename CompareT, typename AllocatorT>
void std::flat_set<ValueT, CompareT, AllocatorT>::clear()
{
	_data.clear();
}

#endif // !defined(SG14_FLAT_SET_H__)
