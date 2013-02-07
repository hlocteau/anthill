#ifndef DISTANCE_TRANFORM_TEMPLATE_HEADER
#define DISTANCE_TRANFORM_TEMPLATE_HEADER

#include <billon.h>

template <typename Gray,typename Distance>
class DistanceTransform {
public:
	typedef Distance value_type ;
	DistanceTransform( const arma::Cube<Gray> &input, Distance xSpacing=1.0f, Distance ySpacing=1.0f, Distance zSpacing=1.0f ) ;
	DistanceTransform( const BillonTpl<Gray> &input ) ;
	
	inline Distance 				maxDistance	( ) const { return _maxDistance ; }
	const arma::Cube<Distance> & 	result		( ) const { return _result ; }
protected:
	void 							compute		( ) ;
private:
	arma::Cube<Distance> 	_result ;
	Distance 				_xSpacing ;
	Distance 				_ySpacing ;
	Distance 				_zSpacing ;
	Distance 				_maxDistance ;
} ;	

#include <DistanceTransform.ih>

#endif
