/**
 * \file squareshuntthin.cpp
 */
#include <DGtal/helpers/StdDefs.h>

#include <io/Pgm3dFactory.h>

#include <queue>

#include <QtGui/qapplication.h>
#include <DGtal/io/viewers/Viewer3D.h>
#include <utils.h>

using namespace std;
using namespace DGtal;
using namespace Z3i;

int main( int narg, char **argv ) {

	int 	width = 100,
			height = 100,
			depth = 300,
			margin = 10,
			thick_x = 9,
			thick_y = 21 ;
	Point 	p( 0,0,0 ),
			q( width, height, depth ) ;

	Domain 	domain( p, q ) ;
	
	DigitalSet shunt( domain ) ;
	
	for ( int y = margin ; y < height - margin ; y++ )
		for ( int x = margin ; x < width - margin ; x++ )
			if ( ( y < (margin+thick_y) || y >= (height - margin - thick_y) ) || (
				 ( x < (margin+thick_x) || x >= (width - margin - thick_x) ) ) )
				for ( int z = margin ; z < depth - margin ; z++ )
					shunt.insertNew( Point( x,y,z ) ) ;
	
	GrayCube scene3d( domain.upperBound().at(0), domain.upperBound().at(1), domain.upperBound().at(2) );
	scene3d.fill( 0 ) ;
	for ( DigitalSet::ConstIterator pt = shunt.begin() ; pt != shunt.end() ; pt++ )
		scene3d.at( (*pt).at(0), (*pt).at(1), (*pt).at(2) ) = 255 ;
	
	io::IOPgm3d< qint8, false >::write( scene3d, QString( argv[1] ) ) ; // "/tmp/shunt.pgm"
	
	/// run skeleton program
	//std::string cmd( ". stuff_skel.sh /tmp/shunt.pgm 7 3 /tmp/shunt.skel.pgm" ) ;
	//execvp( "stuff_skel3D.sh", argv ) ;
	
	/// import output of the program
	GrayCube *skelImg = io::Pgm3dFactory::read( argv[4] ) ; // "/tmp/shunt.skel.pgm" ;
	
	DigitalSet shuntSkel( domain ) ;
	for ( int z = 0 ; z < skelImg->n_slices ; z++ )
		for ( int y = 0 ; y < skelImg->n_rows ; y++ )
			for ( int x = 0 ; x < skelImg->n_cols ; x++ )
				if ( skelImg->at(x,y,z) > 0 )
					shuntSkel.insertNew( Point(x,y,z) ) ;
	#if 0
	Object6_26 shape( dt6_26, shuntSkel );
	int nb_simple=0;
	int layer = 0;
	do {
		DigitalSet & S = shape.pointSet();
		std::queue<DigitalSet::Iterator> Q;
		for ( DigitalSet::Iterator it = S.begin(); it != S.end(); ++it )
			if ( shape.isSimple( *it ) )
				Q.push( it );
		nb_simple = 0;
		while ( ! Q.empty() ) {
			DigitalSet::Iterator it = Q.front();
			Q.pop();
			if ( shape.isSimple( *it ) ) {
				S.erase( *it );
				++nb_simple;
			}
		}
		++layer;
	} while ( nb_simple != 0 );
	DigitalSet & S = shape.pointSet();
	#endif
	
	QApplication application(narg,argv);
	Viewer3D viewer ;
	 viewer.show();
	//viewer <<domain;
	//viewer <<  SetMode3D( domain.className(), "Paving" );
	viewer <<CustomColors3D(Color(25,25,255, 255), Color(25,25,255, 255));
	viewer << shuntSkel ;

	viewer <<  SetMode3D( domain.className(), "Transparent" );
	viewer <<CustomColors3D(Color(255,0,0, 25), Color(250,0,0, 5));
	//viewer << shunt ;
	
	viewer << Viewer3D::updateDisplay;
	
	delete skelImg ;
	return application.exec();
}
