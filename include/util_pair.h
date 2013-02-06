#ifndef UTILITY_HOMOGENEOUS_PAIR_HEADER
#define UTILITY_HOMOGENEOUS_PAIR_HEADER

#include <bits/stl_pair.h>

template <typename T> const T & get_first( const std::pair< T, T > & value ) {
	return value.first ;
}
template <typename T> const T & get_second( const std::pair< T, T > & value ) {
	return value.second ;
}

#endif
