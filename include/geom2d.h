#ifndef GEOM2D_H
#define GEOM2D_H

#include "DGtal/shapes/Shapes.h"
#include "DGtal/helpers/StdDefs.h"
#include <armadillo>

#include <DGtal/shapes/fromPoints/StraightLineFrom2Points.h>

using namespace DGtal ;
using std::list ;
using std::vector ;
class Geom2D
{
public:
    Geom2D();
    
    typedef list< Z2i::Point > 							VertexPolygon ;
	typedef StraightLineFrom2Points<Z2i::Point>			EdgeType ;
	typedef vector< EdgeType > 							EdgePolygon ;

    static void init_edges_polygon( EdgePolygon &edges, const VertexPolygon &vertices ) ;
    
    static void ConvexHull(Z2i::DigitalSet &input, std::list<Z2i::Point> &output);
    static int vectorial_product(const Z2i::Point &a, const Z2i::Point &b, const Z2i::Point &c);
    static void sort_per_angle(Z2i::Point *tab, int size, const Z2i::Point ref);
    static bool comp_points_location(const Z2i::Point &a, const Z2i::Point &b, const Z2i::Point &ref);

    static void sub_sort_per_angle(Z2i::Point *tab, uint id, uint max_position, const Z2i::Point ref);
    static Z2i::Point * computeD8Segment( const Z2i::Point &a, const Z2i::Point &b, unsigned int &size  );
};

#endif // GEOM2D_H
