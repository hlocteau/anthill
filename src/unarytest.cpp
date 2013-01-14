#include <geom2d.h>

int main ( int narg, char **argv ) {
	Point 	pUL( 0,512),
			pLR( 512,0 );
	Domain domain( pUL, pLR ) ;

	DigitalSet curve( domain ) ;
	
	Point 	p1( 2,3),
			p2( 3,3),
			p3( 4,3),
			p4( 4,4),
			p5( 5,5),
			p6(6,6),
			p7(5,6),
			p8(4,6) ;
	
	curve.insertNew( p1 ) ;
	curve.insertNew( p2 ) ;
	curve.insertNew( p3 ) ;
	curve.insertNew( p4 ) ;
	curve.insertNew( p5 ) ;
	curve.insertNew( p6 ) ;
	curve.insertNew( p7 ) ;
	curve.insertNew( p8 ) ;

	std::list<Point> chcurve; ;
	Geom2D::ConvexHull( curve, chcurve ) ;
	
	std::cout<<"Convex hull obtained : "<<std::endl;
	for ( std::list<Point>::iterator pt = chcurve.begin() ; pt != chcurve.end() ; pt++ )
		std::cout<<pt->at(0)<<","<<pt->at(1)<<" " ;
	std::cout<<std::endl;

	return 0 ;
}
