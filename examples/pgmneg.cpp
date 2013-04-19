/**
 * \file pgmneg.cpp
 * Negative of the input 2d/3d image (no matter the convention : 0 and 1 or 0 and 255).
 */
#include <io/IOUtils.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace PgmNegative {
	namespace po = boost::program_options;
	
typedef struct _TProgramArg {
	std::string  _inputFilePath ;
	std::string  _outputFilePath ;
	int          _repeat ;
	int          _border ;
} TProgramArg ;

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Negative of the input 2d/3d image (no matter the convention : 0 and 1 or 0 and 255)."<<std::endl
				<< general_opt << "\n";
}

bool missingParam ( std::string param ) {
	std::cerr <<" Parameter: "<<param<<" is required.."<<std::endl;
	return false ;
}

bool process_arg( int narg, char **argv, TProgramArg &params ) {
	po::options_description general_opt ( "Allowed options are: " );
	general_opt.add_options()
		( "help,h", "display this message." )
		( "input,i", po::value<std::string>(), "Input binary pgm filename." )
		( "output,o", po::value<std::string>(),"Output pgm filename." );

	bool parseOK = true ;
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(narg, argv, general_opt), vm);  
	} catch ( const std::exception& ex ) {
		parseOK = false ;
		std::cerr<< "Error checking program options: "<< ex.what()<< endl;
		return false ;
	}

	po::notify ( vm );
	if (!parseOK || vm.count ( "help" ) ||narg<=1 ) {
		errorAndHelp( general_opt ) ;
		return false ;
	}

	//Parse options
	if ( ! ( vm.count ( "input" ) ) ) missingParam ( "input" );
	std::string inputFileName ;
	params._inputFilePath = vm["input"].as<std::string>();
	if ( ! ( vm.count ( "output" ) ) ) missingParam ( "output" );
	params._outputFilePath = vm["output"].as<std::string>();
	return true ;
}
} // end of namespace

using namespace PgmNegative ;
int main( int narg, char **argv ) {
	TProgramArg params ;
	
	if ( ! process_arg( narg, argv, params ) ) return -1 ;
	
	Pgm3dFactory<arma::u8> factory ;
	BillonTpl<arma::u8> * ioImg = factory.read( params._inputFilePath.c_str() ) ;
	
	arma::u8 max_value = ioImg->max() ;
	
	BillonTpl<arma::u8>::iterator 	beginRWIter = ioImg->begin(),
									endRWIter   = ioImg->end(),
									iter ;
	for ( iter = beginRWIter ; iter != endRWIter ; iter++ )
		*iter = max_value-*iter ;

	IOPgm3d< arma::u8,qint8,false>::write( *ioImg, params._outputFilePath.c_str() ) ;
	
	delete ioImg ;
	return 0 ;
}
