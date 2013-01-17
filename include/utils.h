#ifndef UTILITY_HEADER
#define UTILITY_HEADER

#include <GrayLevelHistogram.h>

arma::Mat<char> * dilate( const arma::Mat<char> &im, int row_radius, int col_radius ) ;
#endif
