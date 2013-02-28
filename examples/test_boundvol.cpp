#include <Bounding.hpp>
#include <DGtal/helpers/StdDefs.h>
#include <QtGui/qapplication.h>
#include <DGtal/io/viewers/Viewer3D.h>
#include <billon.h>
#include <utils.h>
#include <geom2d.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;
void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Compute bounding volume."<<std::endl
				<< general_opt << "\n";
	exit( -1 ) ;
}
void missingParam ( std::string param ){
  std::cerr <<" Parameter: "<<param<<" is required.."<<std::endl;
  exit ( 1 );
}

using namespace std;
using namespace DGtal;
using namespace Z3i;


const unsigned int margin = 2 ;
const unsigned int N = 5*margin;

void gen_xy_cross( arma::Cube<arma::u8> &scene ) {
	for ( int x=1;x<scene.n_cols;x++)
		for ( int di = -1 ; di <= 1 ; di++ )
			for ( int dj = -1 ; dj <= 1 ; dj++ )
				scene( scene.n_rows/2+di, x, scene.n_slices/2+dj ) = 1 ;
	for ( int y=0;y<scene.n_rows;y++)
		for ( int di = -1 ; di <= 1 ; di++ )
			for ( int dj = -1 ; dj <= 1 ; dj++ )
				scene( y, scene.n_cols/2+di, scene.n_slices/2+dj ) = 1 ;
}

void gen_yz_cross( arma::Cube<arma::u8> &scene ) {
	for ( int z=3;z<scene.n_slices;z++)
		for ( int di = -1 ; di <= 1 ; di++ )
			for ( int dj = -1 ; dj <= 1 ; dj++ )
				scene( scene.n_rows/2+di, scene.n_cols/2+dj, z ) = 1 ;
	for ( unsigned int y=0;y<scene.n_rows;y++)
		for ( int di = -1 ; di <= 1 ; di++ )
			for ( int dj = -1 ; dj <= 1 ; dj++ )
				scene( y, scene.n_cols/2+di, scene.n_slices/2+dj ) = 1 ;
}

void gen_xz_cross( arma::Cube<arma::u8> &scene ) {
	for ( int x=1;x<scene.n_cols;x++)
		for ( int di = -1 ; di <= 1 ; di++ )
			for ( int dj = -1 ; dj <= 1 ; dj++ )
				scene( scene.n_rows/2+di, x, scene.n_slices/2+dj ) = 1 ;
	for ( int z=3;z<scene.n_slices;z++)
		for ( int di = -1 ; di <= 1 ; di++ )
			for ( int dj = -1 ; dj <= 1 ; dj++ )
				scene( scene.n_rows/2+di, scene.n_cols/2+dj, z ) = 1 ;
}

void gen_my_ob( arma::Cube<arma::u8> &scene ) {
	int x,y,z ;
	// draw left wall
	x=0 ;
	for ( y=2*margin ; y < scene.n_rows-2*margin ; y++ )
		for ( z = 3*margin ; z < scene.n_slices-5*margin-y ; z++ )
			scene(y,x,z) = 1 ;
	// draw right wall
	x=scene.n_cols-1;
	for ( y=2*margin ; y < scene.n_rows-margin ; y++ )
		for ( z = 3*margin ; z < scene.n_slices-margin ; z++ )
			scene(y,x,z) = 1 ;

	// draw floor
	y=2*margin ;
	for ( x=0 ; x < scene.n_cols ; x++ )
		for ( z = 3*margin ; z < scene.n_slices-3*margin ; z++ )
			scene(y,x,z) = 1 ;

	// draw back wall
	z=3*margin ;
	for ( x=0 ; x < scene.n_cols ; x++ )
		for ( y = 2*margin ; y < scene.n_rows-3*margin-x ; y++ )
			scene(y,x,z) = 1 ;

}

void printpoint( const Z2i::Point *begin, const Z2i::Point *end ) {
	const Z2i::Point *it ;
	for ( it = begin ; it != end ; it++ )
		std::cout<<"("<<it->at(0)<<","<<it->at(1)<<") ";
	std::cout<<std::endl;
}

int main( int narg , char **argv ) {
	// parse command line ----------------------------------------------
	po::options_description general_opt ( "Allowed options are: " );
	general_opt.add_options()
		( "help,h", "display this message." )
		( "figure,f", po::value<int>()->default_value(4), "Figure (toy problems are numerated from 0 to 4)." )
		( "axis,a", po::value<string>(),"Axis along which we compute the 2d convex hull (x|y|z). We may provide 1 to 3 value(s) according to the combination scheme." )
		( "combine,c", po::value<string>()->default_value("and"),"Combination scheme whenever at least 2 axis are provided (\"and\"|\"or\")" )
		( "problem,p", po::value<bool>()->default_value(false),"Display the toy problem (no computation)." ) ;

	bool parseOK = true ;
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(narg, argv, general_opt), vm);  
	} catch ( const std::exception& ex ) {
		parseOK = false ;
		std::cerr<< "Error checking program options: "<< ex.what()<< endl;
	}

	po::notify ( vm );
	if (!parseOK || vm.count ( "help" ) ||narg<=1 ) {
		std::cerr 	<< "Compute bounding volume"<<std::endl
						<< general_opt << "\n";
		return 0;
	}

	//Parse options
	bool computation = ! vm["problem"].as<bool>() ;
	std::string axis ;
	if ( computation && ! ( vm.count ( "axis" ) ) )
		missingParam ( "axis" );
	if ( vm.count ( "axis" ) ) axis = vm["axis"].as<std::string>();
	for ( int i=0;i<axis.size();i++)
		if ( axis[i] != 'x' && axis[i] != 'y' && axis[i] != 'z' ) errorAndHelp( general_opt ) ;
	

	std::string combination_scheme = vm["combine"].as<std::string>();
	if ( combination_scheme.compare( "and") != 0 && combination_scheme.compare( "or") != 0 ) errorAndHelp( general_opt ) ;
	
	int figure = vm["figure"].as<int>() ;
	if ( figure < 0 || figure > 4 ) errorAndHelp( general_opt ) ;

	QApplication application(narg,argv);
	
	Viewer3D viewer ;
	QString title = QString("2D convex hull of 3d objetcs - fig %1").arg(figure);
	if ( computation ) {
		title += QString(" - scan %1").arg(axis.c_str()) ;
		if ( axis.size() > 1 ) title += QString(" - %1").arg(combination_scheme.c_str());
	}
	viewer.setWindowTitle( title.toStdString().c_str() );
	viewer.show();



	arma::Cube<arma::u8> scene( 2*N, N, 4*N ) ;
	scene.fill(0) ;
	std::cout<<"Problem initialization..."<<std::endl;
	switch ( figure ) {
	case 0 :
		gen_xy_cross( scene ) ;
		break;
	case 1 :
		gen_yz_cross( scene ) ;
		break ;
	case 2 :
		gen_xz_cross( scene ) ;
		break ;
	case 3 :
		gen_xz_cross( scene ) ;
		gen_xy_cross( scene ) ;
		break ;
	case 4 :
		gen_my_ob( scene ) ;
		break ;
	}
	std::cout<<"Bounding boxes initialization..."<<std::endl;
	
	typedef Bounding<arma::u8> MyBounding ;
	
	MyBounding bounding( scene, ( combination_scheme.compare( "and" ) == 0 ? MyBounding::COMB_AND : MyBounding::COMB_OR ) ) ;
	const MyBounding::TBBoxes bboxes = bounding.bboxes() ;
	MyBounding::TBBoxConstIterator bb = bboxes.begin() ;
	std::cout<<"Bounding box of cc having color "<<(int)bb->first<<" is "<<bb->second.first<<"  --  "<<bb->second.second<<std::endl;

	arma::Cube< arma::u8 > *pVol = 0 ;
	
	if ( computation ) {
		register unsigned int x,y,z ;
		int encoded_axis = 0 ;
		for ( int i=0;i<axis.size();i++ ) {
			int hull = axis[i] == 'x' ? 0 : ( axis[i] == 'y'? 1 : 2 ) ;
			encoded_axis += 1<<hull;//pow( 2, hull );
		}
		pVol = bounding.convexHull2DAxis( encoded_axis, bb ) ;
	}
	std::cout<<"display"<<std::endl;
	Z3i::Domain domain ( Z3i::Point(0,0,0),Z3i::Point(scene.n_cols-1,scene.n_rows-1,scene.n_slices-1) ) ;
	viewer<<SetMode3D( domain.className(), "BoundingBox" ) ;
	viewer<<domain ;
	viewer<<Z3i::Domain( bb->second.first, bb->second.second ) ;
	{
		viewer << SetMode3D( bb->second.first.className(), "Both") ;
		register unsigned int x,y,z ;
		if ( !computation ) {
			for ( y = 0 ; y < scene.n_rows ; y++ )
				for ( x = 0 ; x < scene.n_cols ; x++ )
					for ( z = 0 ; z < scene.n_slices ; z++ )
						if ( scene( y,x,z) ) {
							viewer << Z3i::Point(x,y,z) ;
						}
		} else {
			for ( y = 0 ; y < (*pVol).n_rows ; y++ )
				for ( x = 0 ; x < (*pVol).n_cols ; x++ )
					for ( z = 0 ; z < (*pVol).n_slices ; z++ ) {
						if ( (*pVol)( y,x,z)>0) {
							viewer << Z3i::Point(x+bb->second.first.at(0),y+bb->second.first.at(1),z+bb->second.first.at(2)) ;
						}
					}
			std::cout<<std::endl;
		}
	}
	scene.reset();	
	viewer << Viewer3D::updateDisplay;
	if ( pVol ) {
		pVol->reset();
		delete pVol ;
		pVol = 0 ;
	}
	return application.exec();
}
