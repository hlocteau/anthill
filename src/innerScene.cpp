#include <io/Pgm3dFactory.h>
#include <ConnexComponentRebuilder.hpp>

typedef arma::u8  InType ;
typedef arma::u32 DistType ;
typedef arma::u8  OutType ;
typedef ConnexComponentRebuilder< InType, DistType, OutType > CCRB ;

using arma::span ;

int main( int narg, char **argv ) {
	
	BillonTpl< InType >   *Img ;
	BillonTpl< DistType > *Depth ;
	

	BillonTpl< OutType > scene( Img->n_rows, Img->n_cols, Img->n_slices ) ;
	scene.fill( 0 ) ;

	DistType margin = Depth.max() ;
	BillonTpl< InType >  subImg  ( params._yunit+2*margin, params._xunit+2*margin, params._zunit+2*margin ) ;
	BillonTpl< DistType > subDepth( params._yunit+2*margin, params._xunit+2*margin, params._zunit+2*margin ) ;
	
	for ( z = 0 ; z < Img->n_slices ; z+= params._zunit )
		for ( y = 0 ; y < Img->n_rows ; y+= params._yunit )
			for ( x = 0 ; x < Img->n_cols ; x+= params._xunit ) {
				n_rows = std::min(y + params._yunit,Img->n_rows) - y ;
				n_cols = std::min(x + params._xunit,Img->n_cols) - x ;
				n_slices = std::min(z + params._zunit,Img->n_slices) - z ;
				subImg.fill(0) ;
				subImg  ( span( margin,margin+n_rows), span( margin,margin+n_cols), span( margin, margin+n_slices ) ) =
				          (*Img)( span( y, y + n_rows ), span( x, x + n_cols ), span( z, z + n_slices ) ) ;
				subDepth.fill(0) ;
				subDepth( span( margin,margin+n_rows), span( margin,margin+n_cols), span( margin, margin+n_slices ) ) =
				          (*Depth)( span( y, y + n_rows ), span( x, x + n_cols ), span( z, z + n_slices ) ) ;
				CCRB ccr( subImg ) ;
				ccr.setDepth( &subDepth ) ;
				ccr.run( ) ;
				BillonTpl<OutType> &res = ccr.result() ;
				
				for ( zo = y-margin ; zo < ; zo++ )
					for ( yo = ; yo < ; yo++ )
						for ( xo = ; xo < ; xo++ )
							scene( yo,xo,zo ) += res(yo,xo,zo ) ;
			}
	delete Img ;
	delete Depth ;
	IOPgm3d< OutType, qint8, false >::write( scene, QString( params._outputFilePath.c_str() ) ) ;
	return 0 ;
}
