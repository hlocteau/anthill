#include <DGtal/shapes/Shapes.h>
#include <DGtal/io/boards/Board2D.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/shapes/fromPoints/StraightLineFrom2Points.h>

using namespace DGtal::Z2i ;

int main ( int narg, char **argv ) {
	Point 	pUL( 9,29),
			pLR( 61,81 );
	Domain domain( pUL, pLR ) ;

	DigitalSet 	shape( domain );
	Point 	p1( 20,30),
			p2( 50,40),
			p3( 60,80),
			p4( 10,75) ;
	
	/** curve is a convex shape whose convex hull is given by the sorted points p1, p2, p3, p4 */
	/** the purpose is to define the interior shape thanks to StraightLineFrom2Points::signedDistance(...) */
	
	StraightLineFrom2Points<Point> segs[4] ;
	segs[0].init( p1, p2 ) ;
	segs[1].init( p2, p3 ) ;
	segs[2].init( p3, p4 ) ;
	segs[3].init( p4, p1 ) ;
	
	for ( Domain::ConstIterator pt = domain.begin() ; pt != domain.end() ; pt++ )
	{
		bool bInside = true ;
		for ( uint iSeg = 0 ; iSeg != 4 && bInside; iSeg++ )
			if ( segs[iSeg].signedDistance( *pt ) < 0 ) bInside = false ;
		if ( bInside )
			shape.insertNew( *pt ) ;
			}
	Board2D board ;
	board << domain << shape ;
	

	Object4_8 obj( dt, shape) ;
	Object4_8 boundary = obj.border() ;
	Color 	blue(0,0,255),
			dblue(0,0,192) ;
	board << CustomStyle( boundary.className(), new CustomColors(blue, dblue) ) << boundary ;
	board.saveSVG( "unarytestSignedDistLine.svg");

	return 0 ;
}
