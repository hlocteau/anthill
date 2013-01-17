#include <def_billon.h>
#include <io/Pgm3dFactory.h>
#include <utils.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Crop skeleton located in the space delimited by the two input given volume."<<std::endl
				<< general_opt << "\n";
}
void missingParam ( std::string param )
{
  /*trace.error()*/std::cerr <<" Parameter: "<<param<<" is required.."<<std::endl;
  exit ( 1 );
}


void boundingXYDomain( const GrayCube &img, arma::Mat< int > &minmap, arma::Mat<int> &maxmap ) {
	uint 	width = img.n_cols,
			height = img.n_rows,
			depth = img.n_slices ;
	for ( uint x=0 ; x < width ; x++ )
		for ( uint y=0; y<height ; y++ ) {
			minmap(x,y) = depth;
			maxmap(x,y) = -1 ;
			for ( uint z = 0 ; z < depth ; z++ )
				if ( img.at(x,y,z) != 0 ) {
					minmap(x,y) = z ;
					break ;
				}
			if ( minmap(x,y) != depth ) {
				for ( uint z = 0 ; z < depth ; z++ )
					if ( img.at(x,y,depth-1-z) != 0 ) {
						maxmap(x,y) = z ;
						break ;
					}
			}
		}
}

int main( int narg, char **argv ) {
	
	// parse command line ----------------------------------------------
	po::options_description general_opt ( "Allowed options are: " );
	general_opt.add_options()
		( "help,h", "display this message." )
		( "first,f", po::value<std::string>(), "Input pgm filename (volume 1)." )
		( "second,s", po::value<std::string>(), "Input pgm filename (volume 2)." )
		( "skel,k", po::value<std::string>(), "Input pgm filename (full skeleton)." )
		( "output,o", po::value<string>(),"Output pgm filename (crop skeleton)." );

	bool parseOK = true ;
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(narg, argv, general_opt), vm);  
	} catch ( const std::exception& ex ) {
		parseOK = false ;
		std::cerr<< "Error checking program options: "<< ex.what()<< endl;
		return -1 ;
	}

	po::notify ( vm );
	if (!parseOK || vm.count ( "help" ) ||narg<=1 ) {
		errorAndHelp( general_opt ) ;
		return -1 ;
	}

	//Parse options
	if ( ! ( vm.count ( "first" ) ) ) missingParam ( "first" );
	std::string volfile_a = vm["first"].as<std::string>();
	if ( ! ( vm.count ( "second" ) ) ) missingParam ( "second" );
	std::string volfile_b = vm["second"].as<std::string>();

	if ( ! ( vm.count ( "skel" ) ) ) missingParam ( "skel" );
	std::string skeletonFileName = vm["skel"].as<std::string>();	

	if ( ! ( vm.count ( "output" ) ) ) missingParam ( "output" );
	std::string outputFileName = vm["output"].as<std::string>();
	
	
	GrayCube *p_img_a = io::Pgm3dFactory::read( QString( volfile_a.c_str() ) ) ;
	GrayCube *p_img_b = io::Pgm3dFactory::read( QString( volfile_b.c_str() ) ) ;
	
	uint 	width = p_img_a->n_cols,
			height = p_img_a->n_rows,
			depth = p_img_a->n_slices ;
	
	GrayCube bounds( width, height, 4 ) ;
	
	boundingXYDomain( *p_img_a, bounds.slice(0), bounds.slice(1) ) ;
	boundingXYDomain( *p_img_b, bounds.slice(2), bounds.slice(3) ) ;

	delete p_img_a ;
	delete p_img_b ;

	GrayCube *skel = io::Pgm3dFactory::read( QString( skeletonFileName.c_str() ) ) ;
	GrayCube cropSkel( width, height, depth ) ;
	cropSkel.fill(0) ;

	for ( uint x = 0 ; x < width ; x++ )
		for ( uint y = 0 ; y < height ; y++ ) {
			if ( bounds.slice(0)(x,y) < depth && bounds.slice(2)(x,y) < depth ) {
				/// both objects have a point (x,y)
				for ( uint z = min( bounds.slice(1)(x,y), bounds.slice(3)(x,y) ) ; z < max(bounds.slice(0)(x,y), bounds.slice(2)(x,y) ) ; z++ )
					if ( (*skel)( x, y, z ) > 0 )
						cropSkel(x,y,z ) = 1 ;
			} else if ( bounds.slice(0)(x,y) == depth && bounds.slice(2)(x,y) == depth ) {
				/// none of the objects have a point (x,y)
				for ( uint z = min( max(max(bounds.slice(1))), max(max(bounds.slice(3))) ) ; z < max( min(min(bounds.slice(0))), min(min(bounds.slice(2))) ) ; z++ )
					if ( (*skel)( x, y, z ) > 0 )
						cropSkel(x,y,z ) = 1 ;
			} else if ( bounds.slice(0)(x,y) < depth ) {
				/// object a is the only object having a point (x,y)
				if ( bounds.slice(0)(x,y) < max(max(bounds.slice(3))) ) {
					for ( uint z = bounds.slice(1)(x,y) ; z < max(max(bounds.slice(3))) ; z++ )
						if ( (*skel)( x, y, z ) > 0 )
							cropSkel(x,y,z ) = 1 ;
				} else {
					for ( uint z = min(min(bounds.slice(2))) ; z < bounds.slice(0)(x,y) ; z++ )
						if ( (*skel)( x, y, z ) > 0 )
							cropSkel(x,y,z ) = 1 ;			
				}
			} else {
				/// meaning bounds.slice(2)(x,y) < depth
				if ( bounds.slice(2)(x,y) < max(max(bounds.slice(1))) ) {
					for ( uint z = bounds.slice(3)(x,y) ; z < max(max(bounds.slice(1))) ; z++ )
						if ( (*skel)( x, y, z ) > 0 )
							cropSkel(x,y,z ) = 1 ;
				} else {
					for ( uint z = min(min(bounds.slice(0))) ; z < bounds.slice(2)(x,y) ; z++ )
						if ( (*skel)( x, y, z ) > 0 )
							cropSkel(x,y,z ) = 1 ;			
				}
			}
		}
	delete skel ;
	io::IOPgm3d< qint32,false>::write( cropSkel, QString( outputFileName.c_str() ) ) ;
	return 0 ;
}
