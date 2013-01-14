#include <billon.h>
#include <io/Pgm3dFactory.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Reset depth voxel values according to the mask provided"<<std::endl
				<< general_opt << "\n";
}
void missingParam ( std::string param )
{
  /*trace.error()*/std::cerr <<" Parameter: "<<param<<" is required.."<<std::endl;
  exit ( 1 );
}



int main( int narg, char **argv ) {
	// parse command line ----------------------------------------------
	po::options_description general_opt ( "Allowed options are: " );
	general_opt.add_options()
		( "help,h", "display this message." )
		( "mask,m", po::value<std::string>(), "Input pgm filename." )
		( "depth,d", po::value<std::string>(), "Input pgm filename." )
		( "output,o", po::value<string>(),"Output pgm filename." );

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
	if ( ! ( vm.count ( "mask" ) ) ) missingParam ( "mask" );
	std::string inputFileName = vm["mask"].as<std::string>();
	if ( ! ( vm.count ( "depth" ) ) ) missingParam ( "depth" );
	std::string depthFileName = vm["depth"].as<std::string>();	
	if ( ! ( vm.count ( "output" ) ) ) missingParam ( "output" );
	std::string outputFileName = vm["output"].as<std::string>();

	Pgm3dFactory<int32_t> factory ;

	BillonTpl<int32_t> *skel = factory.read( QString( inputFileName.c_str() ) ) ;
	factory.correctEncoding( skel ) ;
	BillonTpl<int32_t> *dist = factory.read( QString( depthFileName.c_str() ) ) ;
	factory.correctEncoding( dist ) ;
	
	uint 	depth = dist->n_slices,
			width = dist->n_cols,
			height = dist->n_rows ;
	*( (arma::Cube<int32_t>*) dist) = *( (arma::Cube<int32_t>*) dist) % *( (arma::Cube<int32_t>*) skel) ;
	
	dist->setMinValue( dist->min() ) ;
	dist->setMaxValue( dist->max() ) ;
	
	/*
	for ( uint z = 0 ; z < depth ; z++ )
		for ( uint y = 0 ; y < height ; y++ )
			for ( uint x = 0 ; x < width ; x++ ) {
				if ( skel->at( y, x, z ) == 0 )
					dist->at( y, x, z ) = 0 ;
			}
	*/
	IOPgm3d< int32_t,qint32,false>::write( *dist, QString( outputFileName.c_str() ) ) ;
	
	return 0 ;
}
