/**
 * \file pgmpush.cpp
 * push multiple times a 2d image in order to get a 3d image
 */
#include <io/IOUtils.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace PgmPush {
	namespace po = boost::program_options;
	
typedef struct _TProgramArg {
	std::string  _inputFilePath ;
	std::string  _outputFilePath ;
	int          _repeat ;
	int          _border ;
} TProgramArg ;

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Push multiple times a 2d image in order to get a 3d image."<<std::endl
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
		( "input,i", po::value<std::string>(), "Input binary pgm filename (2d)." )
		( "output,o", po::value<std::string>(),"Output pgm filename (3d)." )
		( "repeat,r", po::value<int>()->default_value(1),"Number of duplicates (along z axis)" )
		( "border,b", po::value<int>()->default_value(0),"optional width of the boundary on the z axis");

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
	
	params._repeat = vm["repeat"].as<int>() ;
	params._border = vm["border"].as<int>() ;
	return true ;
}
} // end of namespace

using namespace PgmPush ;
int main( int narg, char **argv ) {
	TProgramArg params ;
	
	if ( ! process_arg( narg, argv, params ) ) return -1 ;
	
	Pgm3dFactory<arma::u8> factory ;
	BillonTpl<arma::u8> * flatImg = factory.read( params._inputFilePath.c_str() ) ;
	if ( flatImg->max() != 1 ) *flatImg /= flatImg->max() ;
	
	BillonTpl<arma::u8> * outImg = new BillonTpl< arma::u8 >( flatImg->n_rows, flatImg->n_cols, params._repeat + 2 * params._border ) ;
	BillonTpl<arma::u8>::const_iterator 	beginReadIter = flatImg->begin(),
											endReadIter   = flatImg->end(),
											iter ;
	BillonTpl<arma::u8>::iterator writeIter ;
	outImg->fill( 0 ) ;
	
	writeIter = outImg->begin() ;
	for ( int k = 0 ; k < params._border ; k++ )
		for ( int s = 0 ; s < flatImg->n_rows * flatImg->n_cols ; s++ )
			writeIter++ ;
	
	for ( int k = 0 ; k < params._repeat ; k++ ) {
		for ( iter = beginReadIter ; iter != endReadIter ; iter++, writeIter++ )
			*writeIter = 1-*iter ;
	}
	IOPgm3d< arma::u8,qint8,false>::write( *outImg, params._outputFilePath.c_str() ) ;
	
	delete flatImg ;
	delete outImg ;
	return 0 ;
}
