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
		( "input,i", po::value<std::string>(), "Input pgm filename." )
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
	if ( ! ( vm.count ( "input" ) ) ) missingParam ( "input" );
	std::string inputFileName = vm["input"].as<std::string>();
	if ( ! ( vm.count ( "output" ) ) ) missingParam ( "output" );
	std::string outputFileName = vm["output"].as<std::string>();
	
	int top_size = vm["top"].as<int>() ;
	
	QString input( inputFileName.c_str() ) ;
	QString output( outputFileName.c_str() ) ;
	Pgm3dFactory<char> factory ;
	BillonTpl<char> * skel = factory.read( input ) ;
	factory.correctEncoding( skel ) ;
	ConnexComponentExtractor<char,int32_t> CCE ;
	BillonTpl<int32_t> *labels = CCE.run( *skel ) ;

	if ( top_size != -1 ) {
		GrayLevelHistogram<int32_t> histogram( *labels ) ;
		std::list< std::pair<uint32_t,int32_t> > sizeCC ;
		std::list< std::pair<uint32_t,int32_t> >::iterator pos ;
		for ( GrayLevelHistogram<int32_t>::THistogram::const_iterator it=histogram._bin.begin(); it != histogram._bin.end();it++ ) {
			std::cout<<"Key : "<<it->first<<" Frequency : "<<it->second<<std::endl;

			pos = sizeCC.begin() ;
			while ( pos != sizeCC.end() ) {
				if ( pos->first < it->second ) break ;
				pos++ ;
			}
			sizeCC.insert( pos, std::pair<uint32_t,int32_t>( it->second, it->first ) ) ;
		}
		uint *NewMap = new uint [ histogram._bin.size()+1 ] ;
		uint iNewMap = 1 ;
		/// sort connected components' identifier wrt their size

		/// keep only connected components having one of the top_size biggest area value

		uint32_t last_distinct_value = sizeCC.front().first ;
		pos = sizeCC.begin() ;
		/// move pos to the first item to be removed
		while ( pos != sizeCC.end() ) {
			if ( pos->first != last_distinct_value ) {
				last_distinct_value = pos->first ;
				top_size-- ;
			}
			if ( top_size == 0 ) break ;
			NewMap[ pos->second ] = iNewMap ;
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
	}
	else
	{
		/// nothing as we keep all connected components
	}
	IOPgm3d< int32_t,qint32, false >::write( *labels, output ) ;
	
	#ifdef DEBUG_ENCODING_IMAGE
	GrayCube * imcheck = io::Pgm3dFactory::read( output ) ;
	ui32histogram_t hWrite, hRead ;
	GrayLevelHistogram( *labels, hWrite ) ;
	GrayLevelHistogram( *imcheck, hRead ) ;

	if ( accu(*labels != *imcheck) > 0 ) {
		std::cerr<<"ERROR[encoding] : saving/loading an image does not preserve the values"<<std::endl;
		std::cerr<<"Histogram of the exported image "<<std::endl;
		for ( ui32histogram_t::const_iterator bin = hWrite.begin() ; bin != hWrite.end() ; bin++ )
			std::cerr<<bin->first<<" : "<<bin->second<<std::endl;
		std::cerr<<std::endl;
		std::cerr<<"Histogram of the imported image "<<std::endl;
		for ( ui32histogram_t::const_iterator bin = hRead.begin() ; bin != hRead.end() ; bin++ )
			std::cerr<<bin->first<<" : "<<bin->second<<std::endl;
		std::cerr<<std::endl;

	}
	std::cerr<<"Histograms' size are "<< hWrite.size()<<" vs "<<hRead.size()<<std::endl;
	#endif
	
	delete labels ;
	delete skel ;

	return 0 ;
}
