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

std::pair< BillonTpl< arma::u8 >*, BillonTpl< arma::u16 >* > gen_toy_problemB( uint cfg ) {
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

std::pair< BillonTpl< arma::u8 >*, BillonTpl< arma::u16 >* > gen_toy_problemA( ) {
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
	IOPgm3d< arma::u8,qint8, false >::write( *labels, "toy_scene.pgm3d" ) ;
	
	
	/** step 2 : define the distances */
	DistanceTransform< arma::u8, arma::u16 > dt( *labels ) ;
	BillonTpl< arma::u16 > *dist = new BillonTpl<arma::u16>( n_rows, n_cols, n_slices ) ;
	*( (arma::Cube<arma::u16> *) dist ) = dt.result() ;
	
	arma::u8 xx,yy,zz, of ;
	bool on_skeleton ;
	/** step 3 : define a skeleton */
	QString cmd = QString("medialaxis toy_scene.pgm3d 3 /tmp/skel2_tmp_m && threshold /tmp/skel2_tmp_m 1 /tmp/skel2_tmp_m1 && skeleucl toy_scene.pgm3d 26 /tmp/skel2_tmp_m1 /tmp/skel2_tmp_s") ;
	std::system( cmd.toStdString().c_str() ) ;
	delete labels ;
	{
		Pgm3dFactory< arma::u8 > factory ;
		labels = factory.read( "/tmp/skel2_tmp_s" ) ;
		if ( labels->max() != 0 )
			*labels /= labels->max() ;
		labels->setMaxValue(1);
		IOPgm3d< arma::u8, qint8,false >::write( *labels, "toy_skel.pgm3d");
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
				if ( z >= ext[1].at(2) && (x > ext[1].at(0) || (x == ext[1].at(0) && y > ext[1].at(1) ) ) ) ext[1] = Point(x,y,z) ;
				if ( z >= ext[2].at(2) && (x > ext[2].at(0) || (x == ext[2].at(0) && y < ext[2].at(1) ) ) ) ext[2] = Point(x,y,z) ;
				if ( z >= ext[3].at(2) && (x < ext[3].at(0) || (x == ext[3].at(0) && y > ext[3].at(1) ) ) ) ext[3] = Point(x,y,z) ;
				if ( z >= ext[4].at(2) && (x < ext[4].at(0) || (x == ext[4].at(0) && y < ext[4].at(1) ) ) ) ext[4] = Point(x,y,z) ;
			}
std::cerr<<"Classification voxels are : "<<std::endl
			<<"0 : "<<ext[0]<<std::endl
			<<"1 : "<<ext[1]<<std::endl
			<<"2 : "<<ext[2]<<std::endl
			<<"3 : "<<ext[3]<<std::endl
			<<"4 : "<<ext[4]<<std::endl;
	labels->setMaxValue(6);
	arma::u16 d[5] ;
	arma::u8 iClosest ;
std::cin>>	iClosest;
	
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
		( "selection", po::value< std::string >()->multitoken(), "rebuild only specific id.")
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
		
		if ( vm["test"].as<int>() == 1 )
			boost::tie( pLabels,pDist ) = gen_toy_problemA() ;
		else
			boost::tie( pLabels,pDist ) = gen_toy_problemB( vm["test"].as<int>()-2 ) ;
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
		

		ConnexComponentRebuilder< arma::u32, int32_t, arma::u32 > CCR( QString( inputFileName.c_str() ) );
		CCR.setDepth( QString( depthFileName.c_str() ) ) ;
		trace.beginBlock("Reconstruction") ;
		if ( !Labels.isEmpty() )
			for ( uint i=0;i<Labels.size();i++) {
				std::cerr<<"step "<<(int)i<<" / "<<Labels.size()<<" (cc # "<<(int)Labels.at(i)<<")"<<std::endl;
				CCR.run( Labels.at(i),Labels.at(i) ) ;
			}
		else CCR.run() ;
		trace.endBlock() ;
		
		if ( (int)CCR.result().max() < (int)std::numeric_limits<unsigned int>::max() )
			IOPgm3d< arma::u32, qint8, false >::write( CCR.result(), QString( outputFileName.c_str() ) ) ;
		else if ( (int)CCR.result().max() < (int)std::numeric_limits<unsigned short>::max() )
			IOPgm3d< arma::u32, qint16, false >::write( CCR.result(), QString( outputFileName.c_str() ) ) ;
		else
			IOPgm3d< arma::u32, qint32, false >::write( CCR.result(), QString( outputFileName.c_str() ) ) ;
		
	}
	return 0 ;
}
