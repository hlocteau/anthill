#include <utils.h>

void GrayLevelHistogram( const GrayCube & cube, std::map<int,int> &histogram ) {
	for ( uint z=0;z<cube->n_slices; z++ )
		for ( uint y=0;y<cube->n_rows;y++)
			for ( uint x=0;x<cube->n_cols;x++) {
				int value = (*cube).at(x,y,z);
				if ( value == 0 ) continue ; /// Do not care about background
				if ( histogram.find( value ) == histogram.end() )
					histogram[ value ] = 1 ;
				else
					histogram[value]++ ;
			}
}
