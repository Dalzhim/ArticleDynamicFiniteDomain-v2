//
//  main.cpp
//  ArticleDynamicFiniteDomain
//
//  Created by Gabriel Aubut-Lussier on 2017-07-15.
//

#include <boost/icl/interval_map.hpp>
#include <boost/icl/concept/interval.hpp>
#include <boost/icl/type_traits/interval_type_default.hpp>
#include <iostream>
#include <set>

/**
 * In real code, we'd expect some payload to be associated to elements of the domain.
 * This empty struct symbolizes that payload.
 */
struct some_payload
{
};

/**
 * This class represents elements of a dynamic domain. An example domain could be the set
 * of people present inside a room, ordered by a unique number those people were given prior
 * to entering the room. With this image, insertions and removals can be done at any location
 * within the set, as illustrated by people entering or leaving the room with whatever unique
 * number they were holding on to.
 */
class dynamic_domain
{
public:
	using identifier_type = unsigned int;
	using payload_type = some_payload;
	
public:
	dynamic_domain(identifier_type identifier) : identifier(identifier) {}
	
	bool operator<(const dynamic_domain& rhs) const
	{
		return identifier < rhs.identifier;
	}
	
	payload_type& get_payload()
	{
		return payload;
	}
	
	const payload_type& get_payload() const
	{
		return payload;
	}
	
	int get_identifier() const
	{
		return identifier;
	}
	
private:
	identifier_type identifier;
	payload_type payload;
};

using container_type = std::map<dynamic_domain::identifier_type, dynamic_domain>;
using iterator_type = container_type::const_iterator;

/**
 * This iterator class wraps a std::map<K, V>::const_iterator and adds one more
 * operator to it : operator<.
 */
class dynamic_domain_iterator
{
public:
	dynamic_domain_iterator() : it(g_default_container.begin()) {}
	dynamic_domain_iterator(const iterator_type& it) : it(it) {}
	
	const dynamic_domain& operator*() const
	{
		return it->second;
	}
	
	const dynamic_domain* operator->() const
	{
		return std::addressof(it->second);
	}
	
	dynamic_domain_iterator& operator--()
	{
		--it;
		return *this;
	}
	
	dynamic_domain_iterator& operator++()
	{
		++it;
		return *this;
	}
	
	bool operator==(const dynamic_domain_iterator& rhs) const
	{
		return it == rhs.it;
	}
	
	bool operator!=(const dynamic_domain_iterator& rhs) const
	{
		return it != rhs.it;
	}
	
	bool operator<(const dynamic_domain_iterator& rhs) const
	{
		return it->second < rhs.it->second;
	}
	
private:
	iterator_type it;
	
	/**
	 * This default static container makes it possible to default construct an iterator and
	 * allow it to be incremented twice. This is an optimization that makes it possible to
	 * default construct empty intervals which can use the general is_empty algorithm of
	 * Boost.ICL which is : empty = upper < lower. In order for this comparison to be well
	 * defined, both iterators (upper and lower) must be dereferenced, and thus, must point
	 * to valid elements.
	 */
	const static container_type g_default_container;
};

/**
 * For the interval to be empty, the second element of the container must compare less than the first element.
 * That is why the second element has a smaller number than the first.
 */
const container_type dynamic_domain_iterator::g_default_container{{0x9BADBEEF, dynamic_domain{0x9BADBEEF}}, {0x8BADBEEF, dynamic_domain{0x8BADBEEF}}};

namespace std
{
	template <>
	struct iterator_traits<dynamic_domain_iterator>
	{
		using difference_type = iterator_traits<iterator_type>::difference_type;
		using value_type = dynamic_domain_iterator;
		using pointer = const dynamic_domain_iterator*;
		using reference = const dynamic_domain_iterator&;
		using iterator_category = std::bidirectional_iterator_tag;
	};
}

/**
 * This template specialization makes sure that interval containers use
 * boost::icl::closed_interval when the DomainType is dynamic_domain_iterator.
 */
namespace boost { namespace icl {
	template<>
	struct interval_type_default<dynamic_domain_iterator>
	{
		using type = boost::icl::closed_interval<dynamic_domain_iterator>;
	};
}}

/**
 * Thanks to the interval_type_default specialization, we can simply provide
 * the DomainType and CodomainType parameters to the template class.
 */
using MapType = boost::icl::interval_map<dynamic_domain_iterator, int>;

int main(int argc, const char * argv[])
{
	container_type elements;
	elements.insert(std::make_pair(7, dynamic_domain{7}));
	elements.insert(std::make_pair(1, dynamic_domain{1}));
	elements.insert(std::make_pair(2, dynamic_domain{2}));
	elements.insert(std::make_pair(6, dynamic_domain{6}));
	elements.insert(std::make_pair(3, dynamic_domain{3}));
	elements.insert(std::make_pair(4, dynamic_domain{4}));
	elements.insert(std::make_pair(9001, dynamic_domain{9001}));
	elements.insert(std::make_pair(5, dynamic_domain{5}));
	
	MapType intervalMap;
	auto it = dynamic_domain_iterator{elements.cbegin()};
	intervalMap.add(std::make_pair(MapType::interval_type{it, std::next(it, 1)}, 1));
	intervalMap.add(std::make_pair(MapType::interval_type{std::next(it, 2), std::next(it, 3)}, 1));
	intervalMap.add(std::make_pair(MapType::interval_type{std::next(it, 1), std::next(it, 3)}, 1));
	for (auto element : intervalMap) {
		std::cout << "Domain: [" << element.first.lower()->get_identifier() << ", " << element.first.upper()->get_identifier() << "] = " << element.second << std::endl;
	}
}
