#ifndef UTILITY_HEADER
#define UTILITY_HEADER

#include <GrayLevelHistogram.h>

arma::Mat<arma::u8> * dilate( const arma::Mat<arma::u8> &im, int row_radius, int col_radius ) ;


template<typename T> BillonTpl<arma::u8> * filter_low( const BillonTpl<T> &data, T th ) {
	BillonTpl< arma::u8 > *plow = new BillonTpl< arma::u8 > ( data.n_rows, data.n_cols, data.n_slices ) ;
	BillonTpl< arma::u8 >::iterator iterLow = plow->begin(),
	                                iterLowEnd = plow->end() ;
	typename BillonTpl< T >::const_iterator iterData = data.begin() ;
	for ( ; iterLow != iterLowEnd ; iterLow++, iterData++ )
		*iterLow = ( *iterData > 0 && *iterData < th ? 1 : 0 ) ;
	return plow ;
}
template<typename T> BillonTpl<arma::u8> * filter_high( const BillonTpl<T> &data, T th ) {
	BillonTpl< arma::u8 > *phigh = new BillonTpl< arma::u8 > ( data.n_rows, data.n_cols, data.n_slices ) ;
	BillonTpl< arma::u8 >::iterator iterHigh = phigh->begin(),
	                                iterHighEnd = phigh->end() ;
	typename BillonTpl< T >::const_iterator iterData = data.begin() ;
	for ( ; iterHigh != iterHighEnd ; iterHigh++, iterData++ )
		*iterHigh = ( *iterData > th ? 1 : 0 ) ;
	return phigh ;
}

#endif
