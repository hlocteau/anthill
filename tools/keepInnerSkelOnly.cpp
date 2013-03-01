/**
 * \file
 * filter skeleton wrt depth values observed on the depth map of the object and the bounding volume in order to remove branches in concavities.
 */
#include <io/IOPgm3d.h>
#include <io/Pgm3dFactory.h>
#include <io/AntHillFile.hpp>
#include <boost/filesystem.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace InnerSkel {

namespace fs = boost::filesystem ;
namespace po=boost::program_options ;
/**
 * \def PROOF_COORD(a,b)
 * Computes a valid coordinate from b, i.e. a value in the interval 0:a-1
 **/
#define PROOF_COORD(a,b) (b<0?0:(b>=a?a-1:b))

typedef struct _TProgramArg {
	fs::path _inputFolderPath ;
	int      _maxLoop ;
} TProgramArg ;

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Computes a filter skeleton wrt depth values observed on the depth map of the object and the bounding volume in order to remove branches in concavities."<<std::endl
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
		( "input,i", po::value<std::string>(), "Input folder." )
		( "loop,l", po::value<int>()->default_value(-1),"maximum number of iterations (-1 : infinite)." );

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
	if ( ! ( vm.count ( "input" ) ) )   return missingParam ( "input" );
	params._inputFolderPath = vm["input"].as<std::string>();
	params._maxLoop = vm["loop"].as<int>() ;
	return true ;
}
} // end of namespace

using namespace InnerSkel ;

int main( int narg, char **argv ) {
	TProgramArg params ;
	if ( !process_arg( narg, argv, params ) ) return -1 ;
	
	/* input files */
	fs::path skelFilePath = params._inputFolderPath ;
	skelFilePath /= ANTHILL_SKEL_NAME ;
	fs::path distToHullFilePath	= params._inputFolderPath ;
	distToHullFilePath /= ANTHILL_DT_HULL_NAME ;
	fs::path distToSkelFilePath = params._inputFolderPath ;
	distToSkelFilePath /= ANTHILL_DT_CONTENT_NAME ;
	
	if ( ! fs::exists( skelFilePath ) ||
	     ! fs::exists( distToHullFilePath ) ||
	     ! fs::exists( distToSkelFilePath ) ) {
		std::cerr<<"[ Error ] : expecting to find the following file : "<<std::endl
		         <<"            "<<skelFilePath.string()<<std::endl
		         <<"            "<<distToHullFilePath.string()<<std::endl
		         <<"            "<<distToSkelFilePath.string()<<std::endl 
		         <<"            abort program"<<std::endl;
		return -2 ;
	}
	
	/* output file */
	fs::path innerskelFilePath = params._inputFolderPath ;
	innerskelFilePath /= ANTHILL_INNER_SKEL_NAME ;
	
	Pgm3dFactory<arma::u8> factoryBin ;
	Pgm3dFactory<arma::u32> factoryInteger ;
	
	BillonTpl<arma::u8> *skel = factoryBin.read( skelFilePath.c_str() ) ;
	BillonTpl<arma::u32> *distToHull = factoryInteger.read( distToHullFilePath.c_str() ) ;
	BillonTpl<arma::u32> *distToSkel = factoryInteger.read( distToSkelFilePath.c_str() ) ;
	factoryInteger.correctEncoding( distToSkel ) ;
	
	std::cout	<<"Info : size of the input skeleton's image    "<<skel->n_rows<< " x "<<skel->n_cols<<" x " << skel->n_slices<<std::endl
				<<"       size of the distance hull's image     "<<distToHull->n_rows<< " x "<<distToHull->n_cols<<" x " << distToHull->n_slices<<std::endl
				<<"       size of the distance skeleton's image "<<distToSkel->n_rows<< " x "<<distToSkel->n_cols<<" x " << distToSkel->n_slices<<std::endl;
	
	
	arma::Cube<arma::u8> mySkel( skel->n_rows, skel->n_cols, skel->n_slices ) ;
	mySkel.fill( 0 ) ;
	
	int x,y,z ;
	uint32_t 	skel_n_elem = 0,
				myskel_n_del_elem = 0 ;
	for ( y = 0 ; y < mySkel.n_rows ; y++ )
		for ( x = 0 ; x < mySkel.n_cols ; x++ )
			for ( z = 0 ; z < mySkel.n_slices ; z++ ) {
				if ( ! (*skel)( y,x,z ) ) continue ;
				skel_n_elem++ ;
				if ( (*distToHull)( y,x,z ) == (*distToSkel)( y,x,z ) ) {
					myskel_n_del_elem++ ;
					continue ;
				}
				mySkel(y,x,z) = 1 ; // adjacent voxels on the skeleton having same depth value wrt skel but larger value wrt hull have to be removed too
			}

	int neighbor ;
	// remove adjacent voxels...
	bool bUpdate ;
	int nLoop = 0 ;
	do {
		bUpdate = false ;
		for ( y = 0 ; y < mySkel.n_rows ; y++ )
			for ( x = 0 ; x < mySkel.n_cols ; x++ )
				for ( z = 0 ; z < mySkel.n_slices ; z++ ) {
					if ( ! mySkel( y,x,z ) ) continue ;
					for ( neighbor = 0 ; neighbor < 27 ; neighbor++ )
						if ( neighbor != (9+2+3) && 
								( y + (neighbor/9-1)) >= 0 			&& ( y + (neighbor/9-1)) < mySkel.n_rows &&
								( x + ( (neighbor%9)/3 -1 )) >= 0 	&& ( x + ( (neighbor%9)/3 -1 ))<mySkel.n_cols &&
								( z + ( neighbor % 3 -1 )) >= 0 	&& ( z + ( neighbor % 3 -1 ))<mySkel.n_slices )
							if ( (*distToHull)( PROOF_COORD(mySkel.n_rows,y + (neighbor/9-1)), PROOF_COORD(mySkel.n_cols,x + ( (neighbor%9)/3 -1 )), PROOF_COORD(mySkel.n_slices,z + ( neighbor % 3 -1 ) ) ) == (*distToSkel)( PROOF_COORD(mySkel.n_rows,y + (neighbor/9-1)), PROOF_COORD(mySkel.n_cols,x + ( (neighbor%9)/3 -1 )), PROOF_COORD(mySkel.n_slices,z + ( neighbor % 3 -1 ) ) ) &&
								(*distToSkel)(y,x,z) == (*distToSkel)( PROOF_COORD(mySkel.n_rows,y + (neighbor/9-1)), PROOF_COORD(mySkel.n_cols,x + ( (neighbor%9)/3 -1 )), PROOF_COORD(mySkel.n_slices,z + ( neighbor % 3 -1 ) ) ) ) {
								// we can remove this voxel as we delete its neighbor during the initialization (=> not in the current loop) AND we get the same distance to object
								break ;
							}
					if ( neighbor != 27 ) {
						mySkel( y,x,z ) = 0 ;
						myskel_n_del_elem ++ ;
						bUpdate = true ;
					}
				}
		nLoop++ ;
		if ( nLoop == 1 ) std::cout<<skel_n_elem-myskel_n_del_elem<<" after the first loop"<<std::endl;
	} while ( bUpdate && nLoop != params._maxLoop ) ;
	std::cout	<<"Info : number of skeleton's voxels"<<std::endl
				<<"                    - input  : "<<skel_n_elem<<std::endl
				<<"                    - output : "<<skel_n_elem-myskel_n_del_elem<<" after "<<nLoop<<" loop(s)"<<std::endl;
	
	IOPgm3d<arma::u8,qint8,false>::write( mySkel, innerskelFilePath.c_str() ) ;
	delete skel ;
	delete distToHull ;
	delete distToSkel ;
	return 0 ;
}
