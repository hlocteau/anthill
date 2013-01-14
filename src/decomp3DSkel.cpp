#include <def_billon.h>
#include <io/Pgm3dFactory.h>
#include <utils.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

#define step_percentage 5

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Selection (or separation when reject is specified) of skeleton point wrt their depth. When both percentage and value are specified, selection is done wrt the first criterion being valid."<<std::endl
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
		( "skel,s", po::value<std::string>(), "Input pgm filename." )
		( "depth,d", po::value<std::string>(), "Input pgm filename." )
		( "output,o", po::value<string>(),"Output pgm filename." )
		( "reject,r", po::value<string>(),"Output pgm filename for deletion (optional)." )
		( "percentage,p", po::value<int>()->default_value(100),"percetange of pixels to be removed (default is 100, keep nothing), integer in 0..100 ." )
		( "minvalue,m", po::value<int>()->default_value(numeric_limits<int>::max()),"value to be used when selection is obtained wrt value of depth (default, keep nothing), integer." );

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
	
	std::string rejectFileName ;
	if ( vm.count( "reject") ) rejectFileName = vm["reject"].as<std::string>();
	
	int percentage = vm["percentage"].as<int>() ;
	int minvalue = vm["minvalue"].as<int>() ;
	if ( percentage < 0 || percentage > 100 ) {
		errorAndHelp( general_opt ) ;
		return -1 ;
	}

	GrayCube *skel = io::Pgm3dFactory::read( QString( inputFileName.c_str() ) ) ;
	io::Pgm3dFactory::correct_encoding( skel, false ) ;
	GrayCube *dist = io::Pgm3dFactory::read( QString( depthFileName.c_str() ) ) ;
	io::Pgm3dFactory::correct_encoding( dist, true ) ;

	__GrayCube_type__ value ;
	ui32histogram_t histogram ;
	uint32_t sum_of_histogram = 0 ;
	
	uint 	depth = skel->n_slices,
			height = skel->n_rows,
			width = skel->n_cols ;
	__GrayCube_type__ threshold ;
	
	for ( uint z = 0 ; z < depth ; z++ )
		for ( uint y = 0 ; y < height ; y++ )
			for ( uint x = 0 ; x < width ; x++ ) {
				value = skel->at( x, y, z ) ;
				if ( value == 0 ) continue ;
				value = dist->at( x, y, z ) ;
				if ( histogram.find( value ) == histogram.end() ) histogram[value] = 0 ;
				histogram[value]++ ;
				sum_of_histogram++ ;
			}
	if ( minvalue < 0 ) minvalue = histogram.rbegin()->first ;

	uint32_t sum_of_histogram_begining = 0 ;
	std::map< __GrayCube_type__, uint32_t >::const_iterator it = histogram.begin() ;
	while ( it != histogram.end() && ( sum_of_histogram_begining * 100 ) < ( percentage * sum_of_histogram ) ) {
		if ( it->first >= minvalue ) break ;
		sum_of_histogram_begining += it->second ;
		std::cout<<it->first<<" : "<<it->second<<" \t "<<sum_of_histogram_begining<<" / "<<sum_of_histogram<<std::endl;
		it++ ;
	}
	if ( it == histogram.end() ) {
		std::cerr<<"Error : No selection have been done!"<<std::endl;
		delete skel ;
		delete dist ;
		return -1 ;
	}
	
	threshold = it->first ;
	
	
	it = histogram.begin() ;
	int perObserved = 0 ;
	sum_of_histogram_begining=0;
	__GrayCube_type__ lastValue ;
	while ( sum_of_histogram_begining != sum_of_histogram ) {
		while ( it != histogram.end() && ( sum_of_histogram_begining * 100 ) < ( (perObserved+step_percentage) * sum_of_histogram ) ) {
			sum_of_histogram_begining += it->second ;
			lastValue = it->first ;
			it++ ;
		}
		perObserved = ( ( sum_of_histogram_begining * 100 / step_percentage ) / sum_of_histogram ) * step_percentage ;
		std::cout<<lastValue<< " : "<<sum_of_histogram_begining<<" / "<<sum_of_histogram<<" ("<<perObserved<<" \%)"<<std::endl;
	}
	
	std::cerr<<"remove voxel having depth value smaller than "<<threshold<<std::endl;
	GrayCube *rejectImg = 0 ;
	if ( !rejectFileName.empty() ) {
		rejectImg = new GrayCube( width, height, depth ) ;
		rejectImg->fill( 0 ) ;
	}
	for ( uint z = 0 ; z < depth ; z++ )
		for ( uint y = 0 ; y < height ; y++ )
			for ( uint x = 0 ; x < width ; x++ ) {
				value = skel->at( x, y, z ) ;
				if ( value == 0 ) continue ;
				value = dist->at( x, y, z ) ;
				if ( value < threshold ) {
					skel->at( x, y, z ) = 0 ;
				} else if ( rejectImg != 0 ) {
					rejectImg->at( x, y, z ) = value ;
				}
			}
	io::IOPgm3d<qint32,false>::write( *skel, QString( outputFileName.c_str() ) ) ;
	delete skel ;
	delete dist ;
	if ( rejectImg != 0 ) {
		io::IOPgm3d<qint32,false>::write( *rejectImg, QString( rejectFileName.c_str() ) ) ;
		delete rejectImg ;
	}
	return 0 ;
}
