#include <io/GatherFolderImg.hpp>
#include <connexcomponentextractor.h>
#include <Bounding.hpp>

#include <DGtal/helpers/StdDefs.h>
#include <io/IOPgm3d.h>
#include <DistanceTransform.hpp>
#include <io/AntHillFile.hpp>

namespace BuildScene {

typedef ConnexComponentExtractor<arma::u8,arma::u16>			CS_CCExtractor ;
typedef Bounding< CS_CCExtractor::value_type > 					MyBounding;
typedef GrayLevelHistogram< CS_CCExtractor::value_type > 		SHistogram ;
typedef DistanceTransform<arma::u8,arma::u32>					CIDistanceTransform ;
typedef GrayLevelHistogram< CIDistanceTransform::value_type > 	IHistogram ;

/**
 * \param [in] universe a bi-level image 
 * \param labels a labeled image of connected component
 * \param bbox the bounding box of a specific connected component of \a labels
 * \return a bi-level image (0 and 1) where a voxel \a v is set to 1 iff \a universe( v ) != 0 and \a labels( v ) != bbox.key()
 */
BillonTpl<arma::u8> *cropComplement( const arma::Cube<arma::u8> *universe, const BillonTpl<arma::u16> *labels, MyBounding::TBBoxConstIterator bbox ) {
	BillonTpl<arma::u8> *result = new BillonTpl<arma::u8>( universe->n_rows, universe->n_cols, universe->n_slices ) ;
	result->fill(0);
	result->setMinValue(0);
	result->setMinValue(1);
	uint z, x, y ;
	for ( z = 0 ; z < universe->n_slices ; z++ )
		for ( x = 0 ; x < universe->n_cols ; x++ )
			for ( y = 0 ; y < universe->n_rows ; y++ ) {
				if ( (*universe)(y,x,z) == 0 ) continue ;
				(*result)(y,x,z) = (*labels)(y+bbox->second.first.at(1),x+bbox->second.first.at(0),z+bbox->second.first.at(2) ) /*== 0*/ != bbox->first ? 1 : 0 ;
			}
	
	return result ;
}

/**
 * \def PROOF_COORD(a,b)
 * Computes a valid coordinate from b, i.e. a value in the interval 0:a-1
 **/

#define PROOF_COORD(a,b) (b<0?0:(b>=a?a-1:b))
} // end of namespace

using namespace BuildScene ;

int main( int narg, char **argv ) {
	if ( narg == 1 ) return -1 ;
	if( !fs::exists( argv[1] ) ) return -1 ;
	
	GatherFolderImg *factory = new GatherFolderImg( argv[1] ) ;
	factory->load( narg==3?atoi(argv[2]):80 , true) ;
	const arma::Mat<arma::u8> & mask = factory->mask() ;
	arma::Cube<arma::u8> mask3d( mask.n_rows, mask.n_cols, 1 ) ;
	mask3d.slice(0) = mask ;
	
	fs::path folderpath = argv[1] ;
	if ( !fs::is_directory( folderpath ) )
		folderpath = folderpath.parent_path() ;
	if ( folderpath.empty() ) folderpath = ".";
	
	fs::path filepath = folderpath ;
	filepath /= ANTHILL_MASK_NAME ;
	IOPgm3d<arma::u8,qint8,false>::write( mask3d, filepath.c_str() ) ;
	mask3d.reset();
	
	CS_CCExtractor extractor;
	BillonTpl<arma::u16> *labels = extractor.run( factory->scene() ) ;
	delete factory ;
	
	// note : bounding boxes and volumes are computed by extractor...
	
	MyBounding bounding( *labels ) ;
	const MyBounding::TBBoxes & bboxes = bounding.bboxes() ;
	MyBounding::TBBoxConstIterator 	boxIter = bboxes.begin(),
									biggestBox = bboxes.begin(),
									lastBox = bboxes.end() ;
	int bigVol ;
	if ( biggestBox == lastBox ) {
		delete labels ;
		return -1 ;
	}
	bigVol =	( biggestBox->second.second.at(0)-biggestBox->second.first.at(0) )*
				( biggestBox->second.second.at(1)-biggestBox->second.first.at(1) )*
				( biggestBox->second.second.at(2)-biggestBox->second.first.at(2) );
	const QMap<arma::u16,int> &volumes = extractor.volumes() ;
	const QMap<arma::u16, std::pair<iCoord3D, iCoord3D> > & bounds = extractor.bounds3D();
	for ( ++boxIter ; boxIter != lastBox ; boxIter++ ) {
		int curVol =( boxIter->second.second.at(0)-boxIter->second.first.at(0) )*
					( boxIter->second.second.at(1)-boxIter->second.first.at(1) )*
					( boxIter->second.second.at(2)-boxIter->second.first.at(2) );
		if ( bigVol < curVol ) {
			bigVol = curVol ;
			biggestBox = boxIter ;
		}
	}
	
	std::cout<<"BoundingBox #"<<biggestBox->first<<" : "<<biggestBox->second.first<<" "<<biggestBox->second.second<<" volume "<<volumes[ biggestBox->first ]<<std::endl;
	
	// compute the bounding volume as a AND combination of the bounding 2D shapes on each axis
	arma::Cube<arma::u8> *result = bounding.convexHull2DAxis( 7, biggestBox ) ;

	BillonTpl<arma::u8> *inner = cropComplement( result, labels, biggestBox ) ;
	delete labels ;
	
	BillonTpl< CS_CCExtractor::value_type > *labelsInner = extractor.run( *inner ) ;
	SHistogram histInner( *labelsInner ) ;
	
	CIDistanceTransform *dtHull = new CIDistanceTransform( *result ) ;
	const arma::Cube< CIDistanceTransform::value_type > & imDtHull = dtHull->result() ;
	filepath = folderpath ;
	filepath /= ANTHILL_DT_HULL_NAME ;
	IOPgm3d<CIDistanceTransform::value_type,qint32,false>::write( imDtHull, filepath.c_str() ) ;
	
	CS_CCExtractor::value_type biggestInnerCCIdentifier = 1 ;
	for ( SHistogram::THistogram::iterator it = histInner._bin.begin() ; it != histInner._bin.end() ; it++ ) {
		if ( it->second > histInner._bin[ biggestInnerCCIdentifier ] ) biggestInnerCCIdentifier = it->first ;
	}
	delete dtHull ;
	
	// seems that we can keep only the biggest one
	{
		BillonTpl<arma::u8> imSelect ( labelsInner->n_rows, labelsInner->n_cols, labelsInner->n_slices ) ;
		imSelect.fill(0);
		imSelect.setMinValue(0);
		imSelect.setMaxValue(1);
		register int x,y,z,neighbor;
		CS_CCExtractor::value_type id_cc ;
		for ( z = 0 ; z < labelsInner->n_slices ; z++ )
			for ( x = 0 ; x < labelsInner->n_cols ; x++ )
				for ( y = 0 ; y < labelsInner->n_rows ; y++ )
					if ( biggestInnerCCIdentifier == (*labelsInner)( y,x,z) ) {
						for ( neighbor = 0 ; neighbor < 27 ; neighbor++ )
							if ( (*labelsInner)( PROOF_COORD((*labelsInner).n_rows,y + (neighbor/9-1)), PROOF_COORD((*labelsInner).n_cols,x + ( (neighbor%9)/3 -1 )), PROOF_COORD((*labelsInner).n_slices,z + ( neighbor % 3 -1 ) ) ) == 0 ) break ;
						if ( neighbor != 27 ) imSelect(y,x,z) = 1 ;
					}
		for ( z = 0 ; z < labelsInner->n_slices ; z++ )
			for ( x = 0 ; x < labelsInner->n_cols ; x++ )
				for ( y = 0 ; y < labelsInner->n_rows ; y++ )
					if ( biggestInnerCCIdentifier == (*labelsInner)( y,x,z) )
						imSelect(y,x,z) = 1 ;
		fs::path filepath = folderpath ;
		filepath /= ANTHILL_INIT_SCENE_NAME ;
		IOPgm3d<arma::u8,qint8,false>::write( imSelect, filepath.c_str() ) ;
		delete labelsInner ;
		delete result ;
		delete inner ;
	}
	return 1;
}
