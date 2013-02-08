#include <io/Pgm3dFactory.h>
#include <connexcomponentextractor.h>
#include <GrayLevelHistogram.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

//#define DEBUG_ENCODING_IMAGE

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
		( "input,i", po::value<std::string>(), "Input binary pgm filename." )
		( "label,l", po::value<std::string>(), "Input label pgm filename." )
		( "output,o", po::value<string>(),"Output pgm filename." )
		( "top,t", po::value<int>()->default_value(-1),"rank when sorting connected components wrt their length" );

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
		std::cerr 	<< "Export labelled connected component"<<std::endl
						<< general_opt << "\n";
		return 0;
	}

	//Parse options
	if ( ! ( vm.count ( "input" ) ) && ! ( vm.count ( "label" ) )) missingParam ( "input or label" );
	std::string inputFileName ;
	if ( vm.count ( "input" ) ) inputFileName = vm["input"].as<std::string>();
	else inputFileName = vm["label"].as<std::string>();
	if ( ! ( vm.count ( "output" ) ) ) missingParam ( "output" );
	std::string outputFileName = vm["output"].as<std::string>();
	
	int top_size = vm["top"].as<int>() ;
	
	QString input( inputFileName.c_str() ) ;
	QString output( outputFileName.c_str() ) ;
	BillonTpl<arma::u32> *labels ;
	if ( vm.count ( "input" ) ) {
		Pgm3dFactory<arma::u8> factory ;
		BillonTpl<arma::u8> * skel = factory.read( input ) ;
		factory.correctEncoding( skel ) ;
		ConnexComponentExtractor<arma::u8,arma::u32> CCE ;
		labels = CCE.run( *skel ) ;
		delete skel ;
	} else {
		Pgm3dFactory<arma::u32> factory ;
		labels = factory.read( input ) ;
	}

	GrayLevelHistogram<arma::u32> histogram( *labels ) ;
	std::list< std::pair<arma::u32,int32_t> > sizeCC ;
	std::list< std::pair<arma::u32,int32_t> >::iterator pos ;
	for ( GrayLevelHistogram<arma::u32>::THistogram::const_iterator it=histogram._bin.begin(); it != histogram._bin.end();it++ ) {
		std::cout<<"Key : "<<it->first<<" Frequency : "<<it->second<<std::endl;

		pos = sizeCC.begin() ;
		while ( pos != sizeCC.end() ) {
			if ( pos->first < it->second ) break ;
			pos++ ;
		}
		sizeCC.insert( pos, std::pair<arma::u32,int32_t>( it->second, it->first ) ) ;
	}
	uint *NewMap = new uint [ /*histogram._bin.size()*/histogram._bin.rbegin()->first+1 ] ;
	uint iNewMap = 1 ;
	
	if ( top_size == -1 ) top_size = histogram._bin.size() ;
	/// sort connected components' identifier wrt their size

	/// keep only connected components having one of the top_size biggest area value

	arma::u32 last_distinct_value = sizeCC.front().first ;
	pos = sizeCC.begin() ;
	/// move pos to the first item to be removed
	while ( pos != sizeCC.end() ) {
		if ( pos->first != last_distinct_value ) {
			last_distinct_value = pos->first ;
			top_size-- ;
		}
		if ( top_size == 0 ) break ;
		NewMap[ pos->second ] = iNewMap ;
		std::cout<<pos->second<<" => "<<iNewMap<<std::endl;
		iNewMap++ ;
		pos++ ;
	}
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
	IOPgm3d< arma::u32,qint32, false >::write( *labels, output ) ;
	
	delete labels ;

	return 0 ;
}
