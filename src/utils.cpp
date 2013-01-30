#include <utils.h>
#include <billon.h>



arma::Mat<arma::u8> * dilate( const arma::Mat<arma::u8> &im, int row_radius, int col_radius ) {
	arma::Mat<arma::u8> * result = new /*arma::zeros< */arma::Mat<arma::u8>/* >*/( im.n_rows, im.n_cols ) ;
std::cerr<<__FILE__<<" @ line "<<__LINE__<<std::endl;
	result->fill(0);
	register int y, x, x_radius, y_radius ;
	int n_rows = im.n_rows,
		n_cols = im.n_cols ;
std::cerr<<__FILE__<<" @ line "<<__LINE__<<std::endl;
	for ( y = 0 ; y < n_rows ; y++ )
		for ( x = 0 ; x < n_cols ; x++ )
			if ( im (y,x) == 1 ) {
				(*result)(y, x ) = 1 ;
				for ( y_radius = 1 ; y_radius < row_radius ; y_radius++ )
					for ( x_radius = 1 ; x_radius < col_radius ; x_radius++ ) {
						if ( x >= x_radius ) {
							(*result)(y,x-x_radius ) = 1 ;
							if ( y >= y_radius )			(*result)(y-y_radius,x-x_radius ) = 1 ;
							if ( y < n_rows-y_radius )	(*result)(y+y_radius,x-x_radius ) = 1 ;
						}
						if ( x < n_cols-x_radius ) {
							(*result)(y,x+x_radius ) = 1 ;
							if ( y >= y_radius )			(*result)(y-y_radius, x+x_radius ) = 1 ;
							if ( y < n_rows-y_radius )	(*result)(y+y_radius, x+x_radius ) = 1 ;
						}
						if ( y >= y_radius ) 				(*result)(y-y_radius, x ) = 1 ;
						if ( y < n_rows-y_radius )		(*result)(y+y_radius, x ) = 1 ;
					}
			}
std::cerr<<__FILE__<<" @ line "<<__LINE__<<std::endl;
	return result ;
}
