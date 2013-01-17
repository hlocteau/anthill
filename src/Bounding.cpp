#include <Bounding.hpp>
#include <geom2d.h>
#include <billon.h>

unsigned int Bounding::_s_planesBounds[] = {0, 1, 0, 1, 2,
											1, 2, 2, 0, 1, 
											0, 2, 0, 2, 1 } ;

using std::map ;
using std::pair ;

#define DEBUG_BOUNDING

void Bounding::convexHull2D( const GrayCube &labels, int plane, int coord, TBBoxes::const_iterator &iterBboxes, GrayCube &convexPlane ) {
	unsigned int 	width 	= labels.n_cols,
					height	= labels.n_rows,
					depth	= labels.n_slices ;
	register unsigned int x, y, z ;
	register unsigned int index[ 3 ] ;

	const unsigned int *bounds = _s_planesBounds + plane * 5 ;
#ifdef DEBUG_BOUNDING
	std::cout<<"As plane is "<<plane<<" we get parameters : ";
	std::copy( bounds, bounds+5, std::ostream_iterator< unsigned int >( std::cout, " " ) ) ;
	std::cout<<std::endl;
#endif
	const TBBoxes::key_type &current_label = iterBboxes->first ;
	const TBBox &bbox = iterBboxes->second ;
	const Point &lower = bbox.first ;
	const Point &upper = bbox.second ;

	arma::Mat< char > volume = arma::zeros< arma::Mat<char> >( upper.at( bounds[0] )+1-lower.at( bounds[0] ), upper.at( bounds[1] )+1-lower.at( bounds[1] ) ) ;
	__GrayCube_type__ value ;
	index[2] = coord ;
#ifdef DEBUG_BOUNDING
std::cout<<"First loop interval : "<<lower.at( bounds[0] )<<" "<<upper.at( bounds[0] )<<" Second loop interval "<<lower.at( bounds[0] )<<" "<<upper.at( bounds[0] )<<std::endl;
index[0] = lower.at( bounds[0] ) ;
index[1] = lower.at( bounds[1] ) ;
std::cout<<"First voxel evaluated x y z = "<<index[ bounds[2] ]<<" "<<index[ bounds[3] ]<<" "<<index[ bounds[4] ]<<std::endl;
#endif
	for ( index[0] = lower.at( bounds[0] ) ; index[0] <= upper.at( bounds[0] ) ; index[0]++ )
		for ( index[1] = lower.at( bounds[1] ) ; index[1] <= upper.at( bounds[1] ) ; index[1]++ ) {
			value = labels.at( index[ bounds[2] ], index[ bounds[3] ], index[ bounds[4] ] ) ;
			if ( value == current_label )
				volume.at( index[0]-lower.at( bounds[0] ), index[1]-lower.at( bounds[1] ) ) = FOREGROUND_COMP ;
			else if ( value != BACKGROUND_COMP )
				volume.at( index[0]-lower.at( bounds[0] ), index[1]-lower.at( bounds[1] ) ) = FOREIGN_COMP ;
		}
#ifdef DEBUG_BOUNDING
for ( y = 0 ; y < volume.n_rows ; y++, std::cout<<std::endl )
	for ( x = 0 ; x < volume.n_cols ; x++ ) std::cout<<(int)volume.at(x,y)<<" " ;
#endif
	/// identify boundary points
	Z2i::Domain domain( Z2i::Point(0,0), Z2i::Point(volume.n_rows, volume.n_cols) ) ;
	Z2i::DigitalSet boundary( domain ) ;
	computeBoundaryPoints( volume, boundary ) ;
#ifdef DEBUG_BOUNDING
for ( Z2i::DigitalSet::ConstIterator pt = boundary.begin() ; pt != boundary.end() ; pt++ )
	std::cout<<(*pt).at(0)<<","<<(*pt).at(1)<<" " ;
std::cout<<std::endl;
#endif
	Geom2D::VertexPolygon convexhull ;
	Geom2D::ConvexHull( boundary, convexhull ) ;
#ifdef DEBUG_BOUNDING
std::cout<<"Convex hull vertices are " ;
for ( Geom2D::VertexPolygon::iterator pt = convexhull.begin() ; pt != convexhull.end() ; pt++ )
	std::cout<<(*pt).at(0)<<","<<(*pt).at(1)<<" " ;
std::cout<<std::endl;
#endif
	Geom2D::EdgePolygon segs ;
	Geom2D::init_edges_polygon( segs, convexhull ) ;
	if ( segs.size() >= 3 ) {
		Z2i::Point *pts_edge ;
		unsigned int size, iPt ;
		map< int,pair<int,int> > shape_XBound ;
		map< int,pair<int,int> >::iterator pos ;
		for ( int iSeg = 0 ; iSeg < segs.size() ; iSeg++ ) {
			pts_edge = Geom2D::computeD8Segment( segs[ iSeg ].p(), segs[ iSeg ].q(), size ) ;
			for ( iPt = 0 ; iPt < size ; iPt++ ) {
				#ifdef DEBUG_BOUNDING
				std::cout<<pts_edge[iPt].at(0)<<","<<pts_edge[iPt].at(1)<<std::endl ;
				#endif
				pos = shape_XBound.find( pts_edge[ iPt ].at(0) ) ;
				if ( pos == shape_XBound.end() )
					shape_XBound[ pts_edge[ iPt ].at(0) ] = std::pair< int,int>( pts_edge[ iPt ].at(1), pts_edge[ iPt ].at(1) ) ;
				else {
					if ( pos->second.first > pts_edge[ iPt ].at(1) ) 	pos->second.first = pts_edge[ iPt ].at(1) ;
					if ( pos->second.second < pts_edge[ iPt ].at(1) ) 	pos->second.second = pts_edge[ iPt ].at(1) ;
				}
			}
			delete [] pts_edge ;
		}
		#ifdef DEBUG_BOUNDING
		std::cout<<std::endl;
		#endif
		map< int,pair<int,int> >::iterator shape_XBound_iter ;
		int t ;
		for ( shape_XBound_iter = shape_XBound.begin() ; shape_XBound_iter != shape_XBound.end() ; shape_XBound_iter++ ) {
			#ifdef DEBUG_BOUNDING
			std::cout<<"At "<<shape_XBound_iter->first<<" 	range is "<<shape_XBound_iter->second.first<<" to "<<shape_XBound_iter->second.second<<std::endl;
			#endif
			index[0] = shape_XBound_iter->first + lower.at( bounds[0] ) ;
			for ( t = shape_XBound_iter->second.first ; t <= shape_XBound_iter->second.second ; t++ ) {
				index[1] = t + lower.at( bounds[1] ) ;
				convexPlane.at( index[ bounds[2] ], index[ bounds[3] ], index[ bounds[4] ] ) = 1 ;
			}
		}
	}
}

void Bounding::computeBoundingBoxes( const GrayCube &labels, TBBoxes &bboxes ) {
	register unsigned int 	x, y, z ;
	unsigned int 			width 	= labels.n_cols,
							height	= labels.n_rows,
							depth	= labels.n_slices ;
	__GrayCube_type__		value ;
	TBBoxes::iterator 		position ;
	
	for ( x = 0 ; x < width ; x++ )
		for ( y = 0 ; y < height ; y++ )
			for ( z = 0 ; z < depth ; z++ ) {
				value = labels.at( y, x, z ) ;
				if ( value == BACKGROUND_COMP ) continue ;
				position = bboxes.find( value ) ;
				if ( position == bboxes.end() ) {
					bboxes[ value ] = TBBox( Point(x,y,z), Point(x,y,z) ) ;
				} else {
					Point & lower = position->second.first ;
					Point & upper = position->second.second ;
					if ( lower.at(0) > x ) lower.at(0) = x ;
					if ( lower.at(1) > y ) lower.at(1) = y ;
					if ( lower.at(2) > z ) lower.at(2) = z ;
					
					if ( upper.at(0) < x ) upper.at(0) = x ;
					if ( upper.at(1) < y ) upper.at(1) = y ;
					if ( upper.at(2) < z ) upper.at(2) = z ;
				}
			}

}

void Bounding::computeBoundaryPoints( const arma::Mat< char > &volume, Z2i::DigitalSet & boundary ) {
	unsigned int lastValue ;
	register unsigned int 	x, y, z ;
	for ( x = 0 ; x < volume.n_rows ; x++ ) {
		for ( y = 0 ; y < volume.n_cols ; y++ )
			if ( volume.at( x, y ) == 1 ) {
				boundary.insertNew( Z2i::Point( x, y ) ) ;
				lastValue = y ;
				break ;
			}
		if ( y == volume.n_cols ) continue ;
		for ( y = volume.n_cols-1 ; y > lastValue ; y-- )
			if ( volume.at( x, y ) == 1 ) {
				boundary.insertNew( Z2i::Point( x, y ) ) ;
				break ;
			}
	}
	for ( y = 0 ; y < volume.n_cols ; y++ ) {
		for ( x = 0 ; x < volume.n_rows ; x++ )
			if ( volume.at( x, y ) == 1 ) {
				boundary.insertNew( Z2i::Point( x, y ) ) ;
				lastValue = x ;
				break ;
			}
		if ( x == volume.n_rows ) continue ;
		for ( x = volume.n_rows-1 ; x > lastValue ; x-- )
			if ( volume.at( x, y ) == 1 ) {
				boundary.insertNew( Z2i::Point( x, y ) ) ;
				break ;
			}
	}
}
