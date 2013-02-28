#ifndef BILLON_H
#define BILLON_H

#include "interval.h"

#include <QDebug>
#include <def_billon.h>
#include <armadillo>

/*######################################################
  # DECLARATION
  ######################################################*/
/**
 * \brief BillonTpl is just an interface for arma::Cube, adding a header for voxels' dimensions
 * 
 * BillonTpl is issued from TKDetection project available at https://github.com/adrien057/TKDetection
 * \warning keeping only the voxels' dimensions, voxel access and image dimensions are conformed to armadillo scheme:
 * \code
   BillonTpl< arma::u8 > img ( 20, 30, 40 ) ;
   assert( img.n_rows == 20 );
   assert( img.n_cols == 30 );
   assert( img.n_slices == 40 );
   
   img.fill(0) ;
   uint x = 25, y = 10, z = 35
   img( y, x, z ) = 128 ;
   assert( img.at( y,x,z) == 128 ) ;
   \endcode
 */
template< typename T >
class BillonTpl : public arma::Cube<T> {
public:
	BillonTpl();
	BillonTpl( const int & width, const int & height, const int & depth );
	BillonTpl( const BillonTpl<T> &billon );
	BillonTpl( const BillonTpl<T> &billon, const Interval<uint> &sliceInterval );

	const T &minValue() const;
	const T &maxValue() const;
	const qreal &voxelWidth() const;
	const qreal &voxelHeight() const;
	const qreal &voxelDepth() const;

	void setMinValue( const T &value );
	void setMaxValue( const T &value );
	void setVoxelSize( const qreal &width, const qreal &height, const qreal &depth );

protected:
	T _minValue;        // Valeur minimum à considérer (mais une valeur PEUT être plus petite car pas de vérification à l'affectation)
	T _maxValue;        // Valeur maximum à considérer (mais une valeur PEUT être plus grande car pas de vérification à l'affectation)
	qreal _voxelWidth;  // Largeur d'un voxel en cm
	qreal _voxelHeight; // Hauteur d'un voxel en cm
	qreal _voxelDepth;  // Profondeur d'un voxel en cm
};

/*######################################################
  # IMPLEMENTATION
  ######################################################*/

/**********************************
 * Public constructors/destructors
 **********************************/

template< typename T >
BillonTpl<T>::BillonTpl() :
    arma::Cube<T>(), _minValue(T(0)), _maxValue(T(0)), _voxelWidth(1.), _voxelHeight(1.), _voxelDepth(1.) {}

template< typename T >
BillonTpl<T>::BillonTpl( const int & width, const int & height, const int & depth ) :
    arma::Cube<T>(width,height,depth),_minValue(T(0)), _maxValue(T(0)), _voxelWidth(1.), _voxelHeight(1.), _voxelDepth(1.) {}

template< typename T >
BillonTpl<T>::BillonTpl( const BillonTpl<T> & billon ) :
    arma::Cube<T>(billon),_minValue(billon._minValue), _maxValue(billon._maxValue),
	_voxelWidth(billon._voxelWidth), _voxelHeight(billon._voxelHeight), _voxelDepth(billon._voxelDepth) {}

template< typename T >
BillonTpl<T>::BillonTpl( const BillonTpl<T> &billon, const Interval<uint> &sliceInterval ) :
	arma::Cube<T>(billon.slices(sliceInterval.min(),sliceInterval.max())), _minValue(billon._minValue), _maxValue(billon._maxValue),
	_voxelWidth(billon._voxelWidth), _voxelHeight(billon._voxelHeight), _voxelDepth(billon._voxelDepth) {}

/**********************************
 * Public getters
 **********************************/

template< typename T >
const T &BillonTpl<T>::minValue() const
{
	return _minValue;
}

template< typename T >
const T &BillonTpl<T>::maxValue() const
{
	return _maxValue;
}

template< typename T >
const qreal &BillonTpl<T>::voxelWidth() const
{
	return _voxelWidth;
}

template< typename T >
const qreal &BillonTpl<T>::voxelHeight() const
{
	return _voxelHeight;
}

template< typename T >
const qreal &BillonTpl<T>::voxelDepth() const
{
	return _voxelDepth;
}

/**********************************
 * Public setters
 **********************************/

template< typename T >
void BillonTpl<T>::setMinValue( const T & value )
{
	_minValue = value;
}

template< typename T >
void BillonTpl<T>::setMaxValue( const T & value )
{
	_maxValue = value;
}

template< typename T >
void BillonTpl<T>::setVoxelSize( const qreal & width, const qreal & height, const qreal & depth )
{
	_voxelWidth = width;
	_voxelHeight = height;
	_voxelDepth = depth;
}

#endif // BILLON_H
