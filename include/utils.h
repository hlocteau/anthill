#ifndef UTILITY_HEADER
#define UTILITY_HEADER

#include <GrayLevelHistogram.h>

arma::Mat<arma::u8> * dilate( const arma::Mat<arma::u8> &im, int row_radius, int col_radius ) ;
#endif
