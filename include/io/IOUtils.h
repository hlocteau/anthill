#ifndef IO_UTILITY_HEADER
#define IO_UTILITY_HEADER

#include <io/Pgm3dFactory.h>

BillonTpl< arma::u8 > * load_maincc( QString filename ) ;

template < typename T > BillonTpl< T > * load_data_withmask( QString filename, const BillonTpl< arma::u8 > *pmask ) {
	Pgm3dFactory< T > factory ;
	BillonTpl< T > *pdata = factory.read( filename ) ;
	factory.correctEncoding( pdata );
	
	assert( pmask->n_slices == pdata->n_slices && pmask->n_rows == pdata->n_rows && pmask->n_cols == pdata->n_cols ) ;
	
	BillonTpl< arma::u8 >::const_iterator iterMask = pmask->begin(),
										  iterMaskEnd = pmask->end() ;
	typename BillonTpl< T >::iterator iterData = pdata->begin() ;
	for ( ; iterMask != iterMaskEnd ; iterMask++, iterData++ ) {
		if ( ! *iterMask ) *iterData = 0 ;
	}
	return pdata ;
}

template< typename T> void save_minspace( const BillonTpl<T> &img, QString filename ) {
	if ( cast_integer<arma::u8,T>( cast_integer<T,arma::u8>( img.max() ) ) == img.max() ) {
		IOPgm3d< T, qint8, false >::write( img, filename ) ;
	} else if ( cast_integer<arma::u16,T>( cast_integer<T,arma::u16>( img.max() ) ) == img.max() ) {
		IOPgm3d< T, qint16, false >::write( img, filename ) ;
	} else {
		IOPgm3d< T, qint32, false >::write( img, filename ) ;
	}
}

#endif
