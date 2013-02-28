#include <geom2d.h>

using namespace DGtal::Z2i;

int main ( int narg, char **argv ) {
	Point 	pUL( 4,9),
			pLR( 81,91 );
	Domain domain( pUL, pLR ) ;

	DigitalSet curve( domain ) ;
	
	Point 	p1(10,10),
			p2(5,90),
			p3(30,30),
			p4(50,70),
			p5(60,12),
			p6(20,80),
			p7(60,90),
			p8(80,50) ;
	
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
	{
		std::cout<<pt->at(0)<<","<<pt->at(1)<<" " ;
	}
	std::cout<<std::endl;

	return 0 ;
}
