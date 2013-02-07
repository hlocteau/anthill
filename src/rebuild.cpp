/**
 * \brief reconstruct the connected component from the given skeleton and depth map
 */


//#include <util_pair.h>
#include <DistanceTransform.hpp>

#include <ConnexComponentRebuilder.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "boost/tuple/tuple.hpp"

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

std::pair< BillonTpl< arma::u8 >*, BillonTpl< arma::u16 >* > gen_toy_problem( ) {
	BillonTpl< arma::u8 >* labels = new BillonTpl< arma::u8 >( 20, 20, 20 ) ;
	labels->fill(0) ;

	arma::u8 x,y,z,n, n_rows = labels->n_rows,n_cols = labels->n_cols,n_slices=labels->n_slices,iExt ;
	/** step 1 : define the object */
	for ( z = 1 ; z < 9 ; z++ )
		for ( x = 10-z ; x <= 10+z ; x++ )
			for ( y = 10-z ; y <= 10+z ; y++ ) {
				(*labels)(y,x,2*z+0) = 1 ;
				(*labels)(y,x,2*z+1) = 1 ;
			}
	/** step 2 : define the distances */
	DistanceTransform< arma::u8, arma::u16 > dt( *labels ) ;
	BillonTpl< arma::u16 > *dist = new BillonTpl<arma::u16>( n_rows, n_cols, n_slices ) ;
	*( (arma::Cube<arma::u16> *) dist ) = dt.result() ;
	
	/** step 3 : define a skeleton */
	for ( z = 0 ; z < labels->n_slices ; z++ )
		for ( x = 0 ; x < labels->n_cols ; x++ )
			for ( y = 0 ; y < labels->n_rows ; y++ ) {
				if ( (*labels)(y,x,z) == 0 ) continue ;
				for ( n = 0 ; n < 27 ; n++ ) {
					if ( ( y + (n/9-1)) >= 0 		&& ( y + (n/9-1)) < n_rows &&
						 ( x + ( (n%9)/3 -1 )) >= 0 && ( x + ( (n%9)/3 -1 ))<n_cols &&
						 ( z + ( n % 3 -1 )) >= 0 	&& ( z + ( n % 3 -1 ))<n_slices && n != 9+4 ) {
						/// \todo analyze all "segments" passing through (x,y,z)
					}
				}
			}

	/** step 4 : define the classification's voxels */
	Point ext[5] ;
	ext[0] = Point(-1,-1,-1 ) ;
	for ( iExt = 1 ; iExt < 3 ; iExt++ )
		ext[iExt] = Point(-1,n_rows/2,-1 ) ;
	for ( iExt = 3 ; iExt < 5 ; iExt++ )
		ext[iExt] = Point(n_cols,n_rows/2,-1 ) ;
		
	for ( z = 0 ; z < labels->n_slices ; z++ )
		for ( x = 0 ; x < labels->n_cols ; x++ )
			for ( y = 0 ; y < labels->n_rows ; y++ ) {
				if ( (*labels)(y,x,z) == 0 ) continue ;
				if ( ext[0].at(0) < 0 ) ext[0]=Point(x,y,z) ;
				if ( z >= ext[1].at(2) && x >= ext[1].at(0) && y > ext[1].at(1) ) ext[1] = Point(x,y,z) ;
				if ( z >= ext[2].at(2) && x >= ext[2].at(0) && y < ext[2].at(1) ) ext[2] = Point(x,y,z) ;
				if ( z >= ext[3].at(2) && x <= ext[3].at(0) && y > ext[3].at(1) ) ext[3] = Point(x,y,z) ;
				if ( z >= ext[4].at(2) && x <= ext[4].at(0) && y < ext[4].at(1) ) ext[4] = Point(x,y,z) ;
			}
std::cerr<<"Classification voxels are : "<<std::endl
			<<"0 : "<<ext[0]<<std::endl
			<<"1 : "<<ext[1]<<std::endl
			<<"2 : "<<ext[2]<<std::endl
			<<"3 : "<<ext[3]<<std::endl
			<<"4 : "<<ext[4]<<std::endl;
	arma::u16 d[5] ;
	arma::u8 iClosest ;
	/** step 5 : define labels */
	for ( z = 0 ; z < labels->n_slices ; z++ )
		for ( x = 0 ; x < labels->n_cols ; x++ )
			for ( y = 0 ; y < labels->n_rows ; y++ ) {
				if ( (*labels)(y,x,z) == 0 ) continue ;
				iClosest = 0 ;
				for ( iExt = 0 ; iExt < 5 ; iExt++ ) {
					d[ iExt ] = ( ext[ iExt ]-Point(x,y,z) ).dot( ext[ iExt ]-Point(x,y,z) ) ;
					iClosest = ( d[ iExt ] < d[ iClosest ] ? iExt : iClosest ) ;
				}
				(*labels)(y,x,z) = iClosest + 1 ;
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
		( "id,i", po::value<int>()->default_value(-1),"id of the connected component to be rebuild (default, build everything)." )
		( "test,t", po::value<bool>()->default_value(false), "run test program.");

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
	if ( vm["test"].as<bool>() ) {
		BillonTpl< arma::u8 > *pLabels ;
		BillonTpl< arma::u16 > *pDist ;
		
		boost::tie( pLabels,pDist ) = gen_toy_problem() ;
		ConnexComponentRebuilder< arma::u8, arma::u16, arma::u8 > CCR( *pLabels ) ;
		CCR.setDepth( pDist ) ;
		CCR.run() ;
		IOPgm3d< arma::u8,qint8, false >::write( *pLabels, "toy_labels.pgm3d" ) ;
		IOPgm3d< arma::u16,qint16, false >::write( *pDist, "toy_dist.pgm3d" ) ;
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
		
		
		int idComponent = vm["id"].as<int>() ;

		ConnexComponentRebuilder< short, int32_t, char > CCR( QString( inputFileName.c_str() ) );
		CCR.setDepth( QString( depthFileName.c_str() ) ) ;
		trace.beginBlock("Reconstruction") ;
		if ( idComponent != -1 ) CCR.run( idComponent, (char)idComponent ) ;
		else CCR.run() ;
		trace.endBlock() ;
		IOPgm3d< char,qint8, false >::write( CCR.result(), QString( outputFileName.c_str() ) ) ;
	}
	return 0 ;
}
