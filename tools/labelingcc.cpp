#include <io/IOUtils.h>
#include <connexcomponentextractor.h>
#include <GrayLevelHistogram.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;
typedef struct _TProgramArg {
	std::string  _inputFilePath ;
	std::string  _labelFilePath ;
	std::string  _outputFilePath ;
	int          _top_size ;
} TProgramArg ;

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "(Re)define labels of connected components based on their size. The bigger, the smallest identifier is been assigned."<<std::endl
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
		( "label,l", po::value<std::string>(), "Input label pgm filename." )
		( "output,o", po::value<string>(),"Output pgm filename." )
		( "top,t", po::value<int>()->default_value(-1),"rank when sorting connected components wrt their size. (-1 : keep all)" );

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
	if ( ! ( vm.count ( "input" ) ) && ! ( vm.count ( "label" ) )) missingParam ( "input or label" );
	std::string inputFileName ;
	if ( vm.count ( "input" ) ) params._inputFilePath = vm["input"].as<std::string>();
	else params._labelFilePath = vm["label"].as<std::string>();
	if ( ! ( vm.count ( "output" ) ) ) missingParam ( "output" );
	params._outputFilePath = vm["output"].as<std::string>();
	
	params._top_size = vm["top"].as<int>() ;
	return true ;
}

/**
 * (Re)define labels of connected components based on their size. The bigger, the smallest identifier is been assigned.
 * \todo define the core of this program as a method of IOUtils.h
 */
int main( int narg, char **argv ) {
	TProgramArg params ;
	
	if ( ! process_arg( narg, argv, params ) ) return -1 ;
	
	BillonTpl<arma::u32> *labels ;
	if ( ! params._inputFilePath.empty() ) {
		Pgm3dFactory<arma::u8> factory ;
		BillonTpl<arma::u8> * skel = factory.read( params._inputFilePath.c_str() ) ;
		factory.correctEncoding( skel ) ;
		ConnexComponentExtractor<arma::u8,arma::u32> CCE ;
		labels = CCE.run( *skel ) ;
		delete skel ;
	} else {
		Pgm3dFactory<arma::u32> factory ;
		labels = factory.read(  params._labelFilePath.c_str() ) ;
	}

	GrayLevelHistogram<arma::u32> histogram( *labels ) ;
	std::list< std::pair<arma::u32,int32_t> > sizeCC ;
	std::list< std::pair<arma::u32,int32_t> >::iterator pos ;
	for ( GrayLevelHistogram<arma::u32>::THistogram::const_iterator it=histogram._bin.begin(); it != histogram._bin.end();it++ ) {
		pos = sizeCC.begin() ;
		// sort connected components' identifier wrt their size
		while ( pos != sizeCC.end() ) {
			if ( pos->first < it->second ) break ;
			pos++ ;
		}
		sizeCC.insert( pos, std::pair<arma::u32,int32_t>( it->second, it->first ) ) ;
	}
	uint *NewMap = new uint [ histogram._bin.rbegin()->first+1 ] ;
	uint iNewMap = 1 ;
	
	if ( params._top_size == -1 ) params._top_size = histogram._bin.size() ;

	arma::u32 last_distinct_value = sizeCC.front().first ;
	pos = sizeCC.begin() ;
	// move pos to the first item to be removed
	while ( pos != sizeCC.end() ) {
		if ( pos->first != last_distinct_value ) {
			last_distinct_value = pos->first ;
			params._top_size-- ;
		}
		if ( params._top_size == 0 ) break ;
		NewMap[ pos->second ] = iNewMap ;
		iNewMap++ ;
		pos++ ;
	}
	// keep only connected components having one of the top_size biggest area value
	while ( pos != sizeCC.end() ) {
		NewMap[ pos->second ] = 0 ;
		pos++ ;
	}

	for ( uint z = 0 ; z < labels->n_slices ; z++ )
		for ( uint y = 0 ; y < labels->n_rows ; y++ )
			for ( uint x = 0 ; x < labels->n_cols ; x++ )
				if ( (*labels)(y,x,z) != 0 )
					(*labels)(y,x,z) = NewMap[ (*labels)(y,x,z) ] ;
	labels->setMaxValue( iNewMap );
	delete [] NewMap ;
	save_minspace( *labels, params._outputFilePath.c_str() ) ;
	delete labels ;

	return 0 ;
}
