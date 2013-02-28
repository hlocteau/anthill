/**
 * \file
 * reconstruct the connected component from the given skeleton and depth map
 */


//#include <util_pair.h>
#include <DistanceTransform.hpp>

#include <ConnexComponentRebuilder.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "boost/tuple/tuple.hpp"

namespace po = boost::program_options;

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Rebuild labelled regions from a labelled skeleton and a depth map."<<std::endl
				<< general_opt << "\n";
}
void missingParam ( std::string param )
{
  std::cerr <<" Parameter: "<<param<<" is required.."<<std::endl;
  exit ( 1 );
}

std::pair< BillonTpl< arma::u8 >*, BillonTpl< arma::u16 >* > gen_toy_problem( uint cfg ) {
	assert( cfg <= 12 ) ;
	arma::s8 configs[] = { 1,0,0,  0,1,0,  0,0,1,  0,1,-1,  1,0,1,  -1,1,0,  0,1,1,  -1,0,1,  1,1,0,  -1,1,1,  1,-1,1,  1,1,-1,  1,1,1 } ;
	BillonTpl< arma::u8 >* labels = new BillonTpl< arma::u8 >( 40, 40, 40 ) ;
	labels->fill(0) ;
	arma::u8 x,y,z,n, n_rows = labels->n_rows,n_cols = labels->n_cols,n_slices=labels->n_slices,iExt ;
	BillonTpl< arma::u16 > *dist = new BillonTpl<arma::u16>( n_rows, n_cols, n_slices ) ;
	dist->fill(0) ;
	
	Point pBegin(10,10,10), pEnd(10+configs[cfg*3+0]*18,10+configs[cfg*3+1]*18,10+configs[cfg*3+2]*18) ;
	for (uint iCoord = 0 ; iCoord < 3 ; iCoord++ )
		if ( pEnd.at(iCoord) < 0 ) { pBegin.at(iCoord) += 18 ; pEnd.at(iCoord) += 18 ; }
	
	std::cerr<<"[ info ] : draw line "<<pBegin<<" to "<<pEnd<<std::endl;
	
	for (int idx=0;idx<=18;idx++ ) {
		(*dist)( pBegin.at(1)+idx*configs[cfg*3+1], pBegin.at(0)+idx*configs[cfg*3+0],pBegin.at(2)+idx*configs[cfg*3+2] ) = (idx < 4 ? idx+2 : (idx < 15 ? 6 : 20-idx ) ) ;
		(*labels)( pBegin.at(1)+idx*configs[cfg*3+1], pBegin.at(0)+idx*configs[cfg*3+0],pBegin.at(2)+idx*configs[cfg*3+2] ) = (idx < 4 ? 1 : (idx < 15 ? 2 : 3 ) ) ;
	}
	return std::pair< BillonTpl< arma::u8 >*, BillonTpl< arma::u16 >* > ( labels, dist ) ;
}

int main( int narg, char **argv ) {

	// parse command line ----------------------------------------------
	po::options_description general_opt ( "Allowed options are: " );
	general_opt.add_options()
		( "help,h", "display this message." )
		( "skel,s", po::value<std::string>(), "Input colored skeleton pgm filename." )
		( "depth,d", po::value<std::string>(), "Input depth map pgm filename." )
		( "output,o", po::value<string>(),"Output pgm filename." )
		( "selection", po::value< std::string >()->multitoken(), "rebuild only specific id.")
		( "ignore",po::value< std::string >()->multitoken(), "rebuild ignoring specific id.")
		( "test,t", po::value<int>()->default_value(0), "run test program.");

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
	if ( vm["test"].as<int>() ) {
		BillonTpl< arma::u8 > *pLabels ;
		BillonTpl< arma::u16 > *pDist ;
		
		boost::tie( pLabels,pDist ) = gen_toy_problem( vm["test"].as<int>()-1 ) ;
		IOPgm3d< arma::u8,qint8, false >::write( *pLabels, "toy_labels.pgm3d" ) ;
		IOPgm3d< arma::u16,qint16, false >::write( *pDist, "toy_dist.pgm3d" ) ;

		ConnexComponentRebuilder< arma::u8, arma::u16, arma::u8 > CCR( *pLabels ) ;
		CCR.setDepth( pDist ) ;
		CCR.run() ;
		IOPgm3d< arma::u8,qint8, false >::write( CCR.result(), "toy_rebuild.pgm3d" ) ;
		delete pDist ;
		delete pLabels ;
	} else {
		//Parse options
		if ( ! ( vm.count ( "skel" ) ) ) missingParam ( "skel" );
		std::string inputFileName = vm["skel"].as<std::string>();
		if ( ! ( vm.count ( "depth" ) ) ) missingParam ( "depth" );
		std::string depthFileName = vm["depth"].as<std::string>();	
		if ( ! ( vm.count ( "output" ) ) ) missingParam ( "output" );
		std::string outputFileName = vm["output"].as<std::string>();
		
		QList< arma::u32 > Labels;
		if ( vm.count("selection") ) {
			QStringList selectedLabels = QString( "%1").arg( vm["selection"].as< std::string >().c_str() ).split( " ", QString::SkipEmptyParts) ;
			while ( !selectedLabels.isEmpty() ) {
				int sep_interval = selectedLabels.at( 0 ).indexOf( ':') ;
				if ( sep_interval == -1 )
					Labels.append( (arma::u32)selectedLabels.takeAt(0).toInt() ) ;
				else {
					QStringList interval = selectedLabels.takeAt(0).split( ":" ) ;
					int interval_value = interval.takeAt(0).toInt() ;
					int interval_end = interval.takeAt(0).toInt() ;
					for ( ; interval_value <= interval_end ; interval_value++ )
						Labels.append( (arma::u32) interval_value ) ;
				}
			}
			qSort( Labels.begin(), Labels.end(), qLess<arma::u32>() ) ;
		}
		QList< arma::u32 > noLabels ;
		if ( vm.count("ignore") ) {
			QStringList selectedLabels = QString( "%1").arg( vm["ignore"].as< std::string >().c_str() ).split( " ", QString::SkipEmptyParts) ;
			while ( !selectedLabels.isEmpty() ) {
				int sep_interval = selectedLabels.at( 0 ).indexOf( ':') ;
				if ( sep_interval == -1 )
					noLabels.append( (arma::u32)selectedLabels.takeAt(0).toInt() ) ;
				else {
					QStringList interval = selectedLabels.takeAt(0).split( ":" ) ;
					int interval_value = interval.takeAt(0).toInt() ;
					int interval_end = interval.takeAt(0).toInt() ;
					for ( ; interval_value <= interval_end ; interval_value++ )
						noLabels.append( (arma::u32) interval_value ) ;
				}
			}
			qSort( noLabels.begin(), noLabels.end(), qLess<arma::u32>() ) ;
		}

		ConnexComponentRebuilder< arma::u32, int32_t, arma::u32 > CCR( QString( inputFileName.c_str() ), &noLabels );
		CCR.setDepth( QString( depthFileName.c_str() ) ) ;
		trace.beginBlock("Reconstruction") ;
		if ( !Labels.isEmpty() )
			for ( uint i=0;i<Labels.size();i++) {
				std::cerr<<"step "<<(int)i<<" / "<<Labels.size()<<" (cc # "<<(int)Labels.at(i)<<")"<<std::endl;
				CCR.run( Labels.at(i),Labels.at(i) ) ;
			}
		else CCR.run() ;
		trace.endBlock() ;
		
		save_minspace( CCR.result(), QString( outputFileName.c_str() ) ) ;		
	}
	return 0 ;
}
