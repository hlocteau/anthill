/**
 * \brief reconstruct the connected component from the given skeleton and depth map
 */

#include <bits/stl_pair.h>

template <typename T> const T & get_first( const std::pair< T, T > & value ) {
	return value.first ;
}
template <typename T> const T & get_second( const std::pair< T, T > & value ) {
	return value.second ;
}


#include <ConnexComponentRebuilder.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

//#define BUILD_INDIVIDUAL_COMP

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Rebuild labelled regions from a labelled skeleton and a depth map."<<std::endl
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
		( "skel,s", po::value<std::string>(), "Input colored skeleton pgm filename." )
		( "depth,d", po::value<std::string>(), "Input depth map pgm filename." )
		( "output,o", po::value<string>(),"Output pgm filename." )
		( "id,i", po::value<int>()->default_value(-1),"id of the connected component to be rebuild (default, build everything)." );

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
	if ( ! ( vm.count ( "skel" ) ) ) missingParam ( "skel" );
	std::string inputFileName = vm["skel"].as<std::string>();
	if ( ! ( vm.count ( "depth" ) ) ) missingParam ( "depth" );
	std::string depthFileName = vm["depth"].as<std::string>();	
	if ( ! ( vm.count ( "output" ) ) ) missingParam ( "output" );
	std::string outputFileName = vm["output"].as<std::string>();
	
	
	int idComponent = vm["id"].as<int>() ;

	ConnexComponentRebuilder< short, int32_t, char > CCR( QString( inputFileName.c_str() ) );
	CCR.setDepth( QString( depthFileName.c_str() ) ) ;
	trace.beginBlock("Reconstruction") ;
	if ( idComponent != -1 ) CCR.run( idComponent, (char)1 ) ;
	else CCR.run() ;
	trace.endBlock() ;
	IOPgm3d< char,qint8, false >::write( CCR.result(), QString( outputFileName.c_str() ) ) ;
	return 0 ;
}
