#include "geom2d.h"

#include <iomanip>
using std::setw ;

Geom2D::Geom2D()
{
}

bool Geom2D::comp_points_location( const Z2i::Point &a, const Z2i::Point &b, const Z2i::Point &ref )
{
    int value = vectorial_product( ref, a, b ) ;
    if ( value > 0 ) return true ;
    if ( value < 0 ) return false ;
    return ( a - ref ).norm1() > ( b - ref ).norm1() ;
}

void Geom2D::sub_sort_per_angle( Z2i::Point * tab, uint id, uint max_position, const Z2i::Point ref )
{
    uint k = id ;
    uint j = 2 * k ;
    while ( j <= max_position )
    {
        if ( j < max_position && comp_points_location( tab[j], tab[j+1], ref ) )
            j++ ;
        if ( comp_points_location( tab[k], tab[j], ref ) )
        {
            Z2i::Point buf = tab[k] ;
            tab[k] = tab[j] ;
            tab[j] = buf ;
            k = j ;
            j = 2 * k ;
        }
        else
            break ;
    }
}

void Geom2D::sort_per_angle( Z2i::Point * tab, int size, const Z2i::Point ref )
{
    for ( uint i = size/2 ; i > 0 ; i-- )
        sub_sort_per_angle( tab, i, size-1, ref );
    sub_sort_per_angle( tab, 0, size-1, ref );
    for ( uint i = size-1 ; i >= 1 ; i-- )
    {
        Z2i::Point buf = tab[i] ;
        tab[i] = tab[0] ;
        tab[0] = buf ;
        sub_sort_per_angle( tab, 0, i-1, ref );
    }
}

int Geom2D::vectorial_product( const Z2i::Point &a, const Z2i::Point &b, const Z2i::Point &c )
{
    return (c.at(0)-a.at(0) )*( b.at(1) - a.at(1) ) - ( b.at(0) - a.at(0) )*(c.at(1)-a.at(1) );
}

void Geom2D::ConvexHull( Z2i::DigitalSet &input, std::list<Z2i::Point> &output )
{
    assert( !input.empty() ) ;

    Z2i::Point pivot = * input.begin();
    Z2i::Point *tab = new Z2i::Point[ input.size() ] ;
    uint position = 0 ;
    // select pivot
    for ( Z2i::DigitalSet::ConstIterator pt = input.begin() ; pt != input.end() ; pt++ )
    {
        if ( pt->at(0) < pivot.at(0) )
            pivot = *pt ;
        else if ( pt->at(0) == pivot.at(0) && pt->at(1) < pivot.at(1) )
            pivot = *pt ;
        tab[ position ] = *pt ;
        position++ ;
    }
	if ( input.size() > 1 ) {
		// sort table of points according to x-axis and pivot
		sort_per_angle( tab, input.size(), pivot ) ;
    
		vector< Z2i::Point > queue_ ;
		queue_.reserve( input.size());
		queue_.push_back( tab[input.size()-1] );
		queue_.push_back( tab[input.size()-2] );
		// selection of remaining points
		for ( uint i = 3 ; i <= input.size() ; i++ )
		{
			while ( queue_.size() >= 2 )
				if ( vectorial_product( queue_[queue_.size()-2], queue_.back(), tab[input.size()-i]) >= 0 )
					queue_.pop_back();
				else
					break ;
			queue_.push_back( tab[input.size()-i]) ;
		}
		output.clear();
		for ( uint i = 0 ; i < queue_.size() ; i++ )
			output.push_back( queue_[i] ) ;
	} else {
		output.push_back( pivot ) ;
	}
}

Z2i::Point * Geom2D::computeD8Segment( const Z2i::Point &p, const Z2i::Point &q, unsigned int &size ) {
	int a = q.at(1)-p.at(1) ;
	int b = q.at(0)-p.at(0) ;
	
	size = max( abs( a ), abs( b ) )+1 ;
	int i ;
	Z2i::Point *points = new Z2i::Point [ size ] ;

	if ( a * b == 0 ) {
		if ( a == 0 ) {
			int inc = ( b > 0 ? 1 : -1 ) ;
			for ( i = 0 ; i < size ; i++ )
				points[i] = Z2i::Point( p.at(0) + i*inc, p.at(1) ) ;
		} else {
			int inc = ( a > 0 ? 1 : -1 ) ;
			for ( i = 0 ; i < size ; i++ )
				points[i] = Z2i::Point( p.at(0), p.at(1)+inc*i ) ;		
		}
	} else if ( abs(a) == abs(b) ) {
		Z2i::Point inc = Z2i::Point( ( b > 0 ? 1 : -1 ),( a > 0 ? 1 : -1 ) ) ;
		points[ 0 ] = p ;
		for ( i = 1 ; i < size ; i++ )
			points[ i ] = points[ i-1 ] + inc ;
	} else if ( abs(a) > abs(b) ) {
		for ( i = 0 ; i < size ; i++ )
			points[ i ] = p + Z2i::Point( (int)floor(  ( b * i ) / (double)abs(a)+0.5), i*( a>0?1:-1) ) ;
	} else {
		for ( i = 0 ; i < size ; i++ )
			points[ i ] = p + Z2i::Point(i*( b>0?1:-1), (int)floor( ( a * i ) / (double)abs(b)+0.5 ) ) ;
	}
	if ( points[size-1] != q ) {
		std::cerr<<"Generation of segment pixels "<<p<<" to "<<q<<"\t"<<" | a,b is "<<a<<","<<b<<std::endl;
		for ( i = 0;i<size;i++ ) std::cerr<<"("<<points[i].at(0)<<","<<points[i].at(1)<<") " ;
		std::cerr<<std::endl;
	}
	assert( points[size-1] == q ) ;
	return points ;
}

void Geom2D::init_edges_polygon( EdgePolygon &edges, const VertexPolygon &vertices ) {
	VertexPolygon::const_iterator	vertex_begin = vertices.begin(),
									vertex_end ;
	for ( uint iSeg = 0 ; iSeg < vertices.size() ; iSeg++ )
	{
		vertex_end = vertex_begin ;
		vertex_end++ ;
		if ( vertex_end == vertices.end() ) vertex_end = vertices.begin() ;
		edges.push_back( EdgeType( *vertex_begin, *vertex_end ) );
		vertex_begin++ ;
	}
}
