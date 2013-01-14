//#include <io/IOPgm3d.ih>
#include <io/Pgm3dFactory.h>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

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
		( "input,i", po::value<std::string>(), "Input pgm filename." )
		( "output,o", po::value<string>(),"Output pgm filename." )
		( "reverse,r", po::value<bool>()->default_value(true),"flag to be used for changing encoding (dfault is true)." )
		( "binary,b", po::value<bool>()->default_value(true),"flag to be used when output image's data have to be written in binary mode (default is true)." );

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
		std::cerr 	<< "Convert a pgm image (having BYTE/INT32/INT16/DOUBLE/FLOAT format) into pgm (INT16), either binary or ascii"<<std::endl
						<< general_opt << "\n";
		return 0;
	}

	//Parse options
	if ( ! ( vm.count ( "input" ) ) ) missingParam ( "input" );
	std::string inputFileName = vm["input"].as<std::string>();
	if ( ! ( vm.count ( "output" ) ) ) missingParam ( "output" );
	std::string outputFileName = vm["output"].as<std::string>();
	
	bool is_binary = vm["binary"].as<bool>() ;
	bool is_badlyencoded = vm["reverse"].as<bool>() ;
	
	QString input( inputFileName.c_str() ) ;
	QString output( outputFileName.c_str() ) ;
	Pgm3dFactory<int> factory ;
	BillonTpl<int> * skel = factory.read( input ) ;
	if ( is_badlyencoded )
		factory.correctEncoding( skel ) ;
	//*skel /= 16777216 ;
	std::cout<<"Sum on input "<< accu( *skel )<<std::endl;
	
	std::map<int,int> histogram ;
	for ( uint z=0;z<skel->n_slices; z++ )
		for ( uint y=0;y<skel->n_rows;y++)
			for ( uint x=0;x<skel->n_cols;x++) {
				int value = (*skel).at(x,y,z);
				if ( histogram.find( value ) == histogram.end() )
					histogram[ value ] = 1 ;
				else
					histogram[value]++ ;
			}
	for ( std::map<int,int>::const_iterator it=histogram.begin(); it != histogram.end();it++ )
		std::cout<<"Key : "<<it->first<<" Frequency : "<<it->second<<std::endl;
	
	
	std::cout<<"read image "<<skel->n_cols<<" x "<<skel->n_rows<<" x "<<skel->n_slices<<std::endl;
	
	if ( !is_binary )
		IOPgm3d< int,qint16, true >::write( *skel, output ) ;
	else
		IOPgm3d< int,qint16, false >::write( *skel, output ) ;
	delete skel ;

	return 0 ;
}
