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
	
	void convexHull2DPlane( int plane, int coord, TBBoxConstIterator &iterBboxes, arma::Cube< arma::u8 > &convexPlane ) ;
	
	arma::Cube<arma::u8> * convexHull2DAxis( int axis, TBBoxConstIterator &iterBboxes ) ;
	
	inline const TBBoxes & bboxes() const { return _bboxes ; }
protected:
	void computeBoundingBoxes( ) ;
	void computeBoundaryPoints( const arma::Mat< src_elem_type > &volume, Z2i::DigitalSet & boundary ) ;
	void error_outlying( const map< int,pair<int,int> > &shape_XBound, const arma::Mat< src_elem_type > &volume, const Geom2D::EdgePolygon &segs, int plane, int coord, const Z2i::DigitalSet &boundary, const arma::Cube< arma::u8 > &convexPlane, TBBoxConstIterator &iterBboxes ) ;
private:	
	static unsigned int _s_planesBounds[] ;
	
	const arma::Cube<src_elem_type> &_labels ;
	TBBoxes _bboxes ;
	int _comb ;
} ;

/** \example test_boundvol.cpp
 * This is an basic example of how to use the Bounding class.
 * Options are:
 *   - figure <id> where id ranges in 1 to 4; it depicts distinct input object to work on;
 *   - axis <x|y|z> e.g. x or xz or zy; it depicts the axis on which a 2D convex hull has to be computed;
 *   - combine and|or; it depicts how 2D convex hull are combined when several axis are selected;
 *   - problem 1; just display the input object.
 * 
 * \verbatim 
   yourshell$ test_boundVol --figure 4 --problem 1
   \endverbatim
 * \image html problem4.png "Output of test_boundVol --figure 4 --problem 1" 
 * \verbatim 
   yourshell$ test_boundVol --figure 4 --axis x
   \endverbatim
 * \image html x-scan-problem4.png "Output of test_boundVol --figure 4 --axis x" 
 * \verbatim 
   yourshell$ test_boundVol --figure 4 --axis y
   \endverbatim
 * \image html y-scan-problem4.png "Output of test_boundVol --figure 4 --axis y"
 * \verbatim 
   yourshell$ test_boundVol --figure 4 --axis z
   \endverbatim
 * \image html z-scan-problem4.png "Output of test_boundVol --figure 4 --axis z"
 * \verbatim 
   yourshell$ test_boundVol --figure 4 --axis xy
   \endverbatim
 * \image html x-and-y-scan-problem4.png "Output of test_boundVol --figure 4 --axis xy"
 */
 
#include <Bounding.ih>

#endif
