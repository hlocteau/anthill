#include <geom2d.h>
#include <io/Pgm3dFactory.h>
#include <utils.h>
#include <QtGui/qapplication.h>
#include <DGtal/io/viewers/Viewer3D.h>
using namespace DGtal ;
using namespace Z3i ;

int main( int narg, char **argv ) {

	assert( narg == 3 ) ;
	char *filename = argv[1] ;
	int id = atoi( argv[2] ) ;


	/// load skeleton
	GrayCube *img = io::Pgm3dFactory::read( QString( filename ) ) ;
	
	uint 	width = img->n_cols,
			height = img->n_rows,
			depth = img->n_slices ;
	
	Domain domain( Point(0,0,0), Point(width-1, height-1, depth-1) ) ;
	/// select voxels from the given connect component
	DigitalSet S( domain ) ;
	for ( Domain::ConstIterator pt = domain.begin() ; pt != domain.end() ; pt++ )
		if ( img->at( (*pt).at(1), (*pt).at(0), (*pt).at(2) ) == id )
			S.insertNew( *pt ) ;
	delete img ;
	if ( S.empty() ) {
		std::cerr<<"Error : Empty digital selection"<<std::endl;
		return -1;
	}
	/// computes the parameters of the plan
	arma::vec  	GC(3) ;
	arma::Mat< double >	B ;
	Geom2D::pca( S, B, GC ) ;

	/// display both the connected component and the plan

	QApplication application(narg,argv);
	Viewer3D viewer;
	viewer.show(); 
	Color c = Color( 0,255,0 );
	viewer << CustomColors3D( c, c );
	viewer << S ;
	c = Color( 255,255,0 );
	viewer << CustomColors3D( c, c );
	for ( DigitalSet::ConstIterator pt = S.begin() ; pt != S.end() ; pt++ ) {
		arma::vec projection(3) ;
		Geom2D::projection_on( *pt, B, GC, projection ) ;
		viewer << Point( round(projection.at(0)),round(projection.at(1)),round(projection.at(2)) ) ;
		//std::cout<<round(projection.at(0))<<";"<<round(projection.at(1))<<";"<<round(projection.at(2))<<std::endl;
	}
	
	viewer << Viewer3D::updateDisplay ;
	return application.exec();
}
