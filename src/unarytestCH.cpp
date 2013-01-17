#include <geom2d.h>
#include <DGtal/io/boards/Board2D.h>

int main ( int narg, char **argv ) {
	Point 	pUL( 4,9),
			pLR( 81,91 );
	Domain domain( pUL, pLR ) ;

	DigitalSet curve( domain ) ;
	
	/*
	Point 	p1( 2,3),
			p2( 3,3),
			p3( 4,3),
			p4( 4,4),
			p5( 5,5),
			p6(6,6),
			p7(5,6),
			p8(4,6) ;
	*/
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
	

	Board2D board ;
	board << domain << curve ;
	Color 	blue(0,0,255),
			dblue(0,0,192) ;
	board << CustomStyle( p1.className(), new CustomColors(blue, dblue) ) ;

	std::cout<<"Convex hull obtained : "<<std::endl;
	for ( std::list<Point>::iterator pt = chcurve.begin() ; pt != chcurve.end() ; pt++ )
	{
		std::cout<<pt->at(0)<<","<<pt->at(1)<<" " ;
		board << *pt ;
	}
	std::cout<<std::endl;
	board.saveSVG( "unarytestCH.svg");

	return 0 ;
}
