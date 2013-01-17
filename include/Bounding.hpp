#ifndef BOUNDING_VOLUME_HEADER
#define BOUNDING_VOLUME_HEADER
#include <DGtal/helpers/StdDefs.h>
#include <utils.h>
#include <armadillo>
#include <geom2d.h>
using namespace DGtal ;
using namespace Z3i ;
template <typename T>
class Bounding {
public:
	typedef T 														src_elem_type ;
	enum { BOUNDING_YZ_PLANE, BOUNDING_XZ_PLANE, BOUNDING_XY_PLANE } ;
	enum { BOUNDING_X_AXIS=1, BOUNDING_Y_AXIS=2, BOUNDING_Z_AXIS=4 } ;
	enum { BACKGROUND_COMP, FOREGROUND_COMP, FOREIGN_COMP } ;
	enum { COMB_OR, COMB_AND } ;
	typedef pair< Point, Point > 									TBBox ;
	typedef map< src_elem_type, TBBox > 							TBBoxes ;
	typedef typename map< src_elem_type, TBBox >::iterator 			TBBoxIterator ;
	typedef typename map< src_elem_type, TBBox >::const_iterator 	TBBoxConstIterator ;
	
	Bounding( const arma::Cube<T> & labels, int combination=COMB_AND );
	~Bounding();
	
	void convexHull2DPlane( int plane, int coord, TBBoxConstIterator &iterBboxes, arma::Cube< char > &convexPlane ) ;
	
	arma::Cube<char> * convexHull2DAxis( int axis, TBBoxConstIterator &iterBboxes ) ;
	
	inline const TBBoxes & bboxes() const { return _bboxes ; }
protected:
	void computeBoundingBoxes( ) ;
	void computeBoundaryPoints( const arma::Mat< char > &volume, Z2i::DigitalSet & boundary ) ;
	void error_outlying( const map< int,pair<int,int> > &shape_XBound, const arma::Mat< char > &volume, const Geom2D::EdgePolygon &segs, int plane, int coord, const Z2i::DigitalSet &boundary, const arma::Cube< char > &convexPlane, TBBoxConstIterator &iterBboxes ) ;
private:	
	static unsigned int _s_planesBounds[] ;
	
	const arma::Cube<src_elem_type> &_labels ;
	TBBoxes _bboxes ;
	int _comb ;
} ;
#include <Bounding.ih>

#endif
