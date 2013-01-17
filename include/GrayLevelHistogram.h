#ifndef GRAY_LEVEL_HISTOGRAM_HEADER
#define GRAY_LEVEL_HISTOGRAM_HEADER

#include <billon.h>
#include <map>

template <typename T>
class GrayLevelHistogram{ 
	public:
		typedef T TColor ;
		typedef std::map<T,uint32_t> 	THistogram ;
		inline GrayLevelHistogram( const BillonTpl<T> & img, int xmin=0,int xmax=-1, int ymin=0, int ymax=-1, int zmin=0, int zmax=-1  ) ;
		inline GrayLevelHistogram( const arma::Cube<T> & img, int xmin=0,int xmax=-1, int ymin=0, int ymax=-1, int zmin=0, int zmax=-1 ) ;
	public:
		THistogram _bin ;
} ;

template<typename T> inline GrayLevelHistogram<T>::GrayLevelHistogram( const BillonTpl<T> & img, int xmin,int xmax, int ymin, int ymax, int zmin, int zmax  ) {
	int z,y,x;
	zmax = ( zmax == -1 ? (int)img.n_slices : std::min( (int)img.n_slices, zmax) ) ;
	ymax = ( ymax == -1 ? (int)img.n_rows : std::min( (int)img.n_rows, ymax) ) ;
	xmax = ( xmax == -1 ? (int)img.n_cols : std::min( (int)img.n_cols, xmax) ) ;
	xmin = std::min( std::max(0,xmin), xmax-1 ) ;
	ymin = std::min( std::max(0,ymin), ymax-1 ) ;
	zmin = std::min( std::max(0,zmin), zmax-1 ) ;

	for ( z=zmin;z<zmax; z++ )
		for ( y=ymin;y<ymax;y++)
			for ( x=xmin;x<xmax;x++) {
				T value = img(y,x,z);
				if ( value == 0 ) continue ; /// Do not care about background
				if ( _bin.find( value ) == _bin.end() )
					_bin[ value ] = 1 ;
				else
					_bin[value]++ ;
			}
}

template<typename T> inline GrayLevelHistogram<T>::GrayLevelHistogram( const arma::Cube<T> & img, int xmin,int xmax, int ymin, int ymax, int zmin, int zmax ) {
	int z,y,x;
	zmax = ( zmax == -1 ? (int)img.n_slices : std::min( (int)img.n_slices, zmax) ) ;
	ymax = ( ymax == -1 ? (int)img.n_rows : std::min( (int)img.n_rows, ymax) ) ;
	xmax = ( xmax == -1 ? (int)img.n_cols : std::min( (int)img.n_cols, xmax) ) ;
	xmin = std::min( std::max(0,xmin), xmax-1 ) ;
	ymin = std::min( std::max(0,ymin), ymax-1 ) ;
	zmin = std::min( std::max(0,zmin), zmax-1 ) ;

	for ( z=zmin;z<zmax; z++ )
		for ( y=ymin;y<ymax;y++)
			for ( x=xmin;x<xmax;x++) {
				T value = img(y,x,z);
				if ( value == 0 ) continue ; /// Do not care about background
				if ( _bin.find( value ) == _bin.end() )
					_bin[ value ] = 1 ;
				else
					_bin[value]++ ;
			}
}

#endif
