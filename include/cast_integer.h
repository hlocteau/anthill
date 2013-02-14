#ifndef CAST_INTEGER_HEADER
#define CAST_INTEGER_HEADER

#include <limits>

template < typename SRC, typename DST > DST cast_integer( SRC value ) {
	size_t src_size = sizeof( SRC ) ;
	size_t dst_size = sizeof( DST ) ;
	if ( src_size == dst_size ) {
		return (DST) value ;
	} else if ( src_size < dst_size ) {
		DST cast_value = value ;
		if ( !std::numeric_limits<DST>::is_signed ) {
			DST mask = 0x00ff ;
			uint shift = src_size-1 ;
			while ( shift != 0 ) {
				mask = mask << 8 | mask ;
				shift-= 1 ;
			}
			cast_value = cast_value & mask ;
		}
		return cast_value ;
	} else {
		return (DST) value ; ///?
	}
}

#endif
