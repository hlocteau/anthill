/**
 * filter skeleton wrt depth values observed on the depth map of the object and the bounding volume
 */
#include <io/IOPgm3d.h>
#include <io/Pgm3dFactory.h>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem ;

#define PROOF_COORD(a,b) (b<0?0:(b>=a?a-1:b))

int main( int narg, char **argv ) {
	fs::path inputFolderPath = argv[1] ;
	int maxLoop = -1 ;
	if ( narg == 3 ) maxLoop=atoi(argv[2]);
	/** input files */
	fs::path skelFilePath = inputFolderPath ;
	skelFilePath /= "anthillcontent.skeleucl.pgm3d" ;
	fs::path distToHullFilePath	= inputFolderPath ;
	distToHullFilePath /= "anthill.dthull.pgm3d" ;
	fs::path distToSkelFilePath = inputFolderPath ;
	distToSkelFilePath /= "anthillcontent.dist.pgm3d" ;
	
	/** output files */
	fs::path featAdjRemovedVoxelFilePath = inputFolderPath ;
	featAdjRemovedVoxelFilePath /= "featAdjRemovedVoxels.txt" ;
	fs::path innerskelFilePath = inputFolderPath ;
	innerskelFilePath /= "anthill.innerskel.pgm3d" ;
	
	Pgm3dFactory<arma::u8> factoryBin ;
	Pgm3dFactory<arma::u32> factoryInteger ;
	
	BillonTpl<arma::u8> *skel = factoryBin.read( QString( "%1").arg( skelFilePath.c_str() ) ) ;
	BillonTpl<arma::u32> *distToHull = factoryInteger.read( QString( "%1").arg(distToHullFilePath.c_str() ) ) ;
	//factoryInteger.correctEncoding( distToHull ) ;
	BillonTpl<arma::u32> *distToSkel = factoryInteger.read( QString( "%1").arg(distToSkelFilePath.c_str() ) ) ;
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
				mySkel(y,x,z) = 1 ; /// \note : adjacent voxels on the skeleton having same depth value wrt skel but larger value wrt hull have to be removed too
			}

	int neighbor ;
	/// remove adjacent voxels...
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
								/// we can remove this voxel as we delete its neighbor during the initialization (=> not in the current loop) AND we get the same distance to object
								break ;
							}
					if ( neighbor != 27 ) {
						mySkel( y,x,z ) = 0 ;
						if ( myskel_n_del_elem == skel_n_elem ) { std::cerr<<"Can not remove more elements than the inital number!"<<std::endl; z=mySkel.n_slices ; x = mySkel.n_cols;y=mySkel.n_rows;bUpdate=false;break;}
						myskel_n_del_elem ++ ;
						//bUpdate = true ;
					}
				}
		nLoop++ ;
		if ( nLoop == 1 ) std::cout<<skel_n_elem-myskel_n_del_elem<<" after the first loop"<<std::endl;
	} while ( bUpdate && nLoop != maxLoop ) ;
	std::cout	<<"Info : number of skeleton's voxels"<<std::endl
				<<"                    - input  : "<<skel_n_elem<<std::endl
				<<"                    - output : "<<skel_n_elem-myskel_n_del_elem<<" after "<<nLoop<<" loop(s)"<<std::endl;
	
	QFile file( featAdjRemovedVoxelFilePath.c_str() ) ;
	Z3i::DigitalSet toRemove( Z3i::Domain( Z3i::Point(0,0,0), Z3i::Point( mySkel.n_cols-1, mySkel.n_rows-1, mySkel.n_slices-1) ) ) ;
	if( file.open(QFile::WriteOnly) ) {
		QTextStream out(&file);
		for ( int iStep = 1 ; ; iStep++ ) {
			out<<"Step "<<iStep<<endl;
			for ( y = 0 ; y < mySkel.n_rows ; y++ )
				for ( x = 0 ; x < mySkel.n_cols ; x++ )
					for ( z = 0 ; z < mySkel.n_slices ; z++ ) {
						if ( mySkel( y,x,z )==0 ) continue ;
						for ( neighbor = 0 ; neighbor < 27 ; neighbor++ )
							if ( neighbor != (9+2+3) && 
										( y + (neighbor/9-1)) >= 0 			&& ( y + (neighbor/9-1)) < mySkel.n_rows &&
										( x + ( (neighbor%9)/3 -1 )) >= 0 	&& ( x + ( (neighbor%9)/3 -1 ))<mySkel.n_cols &&
										( z + ( neighbor % 3 -1 )) >= 0 	&& ( z + ( neighbor % 3 -1 ))<mySkel.n_slices )
								if ( 0==mySkel( PROOF_COORD(mySkel.n_rows,y + (neighbor/9-1)), PROOF_COORD(mySkel.n_cols,x + ( (neighbor%9)/3 -1 )), PROOF_COORD(mySkel.n_slices,z + ( neighbor % 3 -1 ) ) ) &&
									1==(*skel)( PROOF_COORD(mySkel.n_rows,y + (neighbor/9-1)), PROOF_COORD(mySkel.n_cols,x + ( (neighbor%9)/3 -1 )), PROOF_COORD(mySkel.n_slices,z + ( neighbor % 3 -1 ))) &&
									(*distToSkel)( y,x,z ) <= (*distToSkel)( PROOF_COORD(mySkel.n_rows,y + (neighbor/9-1)), PROOF_COORD(mySkel.n_cols,x + ( (neighbor%9)/3 -1 )), PROOF_COORD(mySkel.n_slices,z + ( neighbor % 3 -1 ) ) ) &&
									(*distToHull)( y,x,z ) <= (*distToHull)( PROOF_COORD(mySkel.n_rows,y + (neighbor/9-1)), PROOF_COORD(mySkel.n_cols,x + ( (neighbor%9)/3 -1 )), PROOF_COORD(mySkel.n_slices,z + ( neighbor % 3 -1 ) ) ) ) {
									out	<<x<<";"<<y<<";"<<z<<";"
										<< (*distToSkel)( y,x,z ) << ";"
										<<(*distToHull)( y,x,z ) <<";";
									out	<<(int)PROOF_COORD(mySkel.n_cols,x + ( (neighbor%9)/3 -1 ))<<";"<<(int)PROOF_COORD(mySkel.n_rows,y + (neighbor/9-1))<<";"<<(int)PROOF_COORD(mySkel.n_slices,z + ( neighbor % 3 -1 ) )<<";"
										<<(*distToSkel)( PROOF_COORD(mySkel.n_rows,y + (neighbor/9-1)), PROOF_COORD(mySkel.n_cols,x + ( (neighbor%9)/3 -1 )), PROOF_COORD(mySkel.n_slices,z + ( neighbor % 3 -1 ) ) )<<";"
										<<(*distToHull)( PROOF_COORD(mySkel.n_rows,y + (neighbor/9-1)), PROOF_COORD(mySkel.n_cols,x + ( (neighbor%9)/3 -1 )), PROOF_COORD(mySkel.n_slices,z + ( neighbor % 3 -1 ) ) )<<endl;
									toRemove.insertNew( Z3i::Point( x,y,z ) ) ;
								}
					}
			if ( toRemove.empty() ) break ;
			for ( Z3i::DigitalSet::ConstIterator pt = toRemove.begin() ; pt != toRemove.end() ; pt++ )
				mySkel( (*pt).at(1), (*pt).at(0), (*pt).at(2) ) = 0 ;
			toRemove.clear() ;
			IOPgm3d<arma::u8,qint8,false>::write( mySkel, QString( "%1" ).arg( innerskelFilePath.c_str() ) ) ;
		}
		file.close() ;
	} else {
		std::cerr << "Error : saving features of adjacent voxels being removed"<< std::endl;
	}
	delete skel ;
	delete distToHull ;
	delete distToSkel ;
	
	
	
	return 0 ;
}
