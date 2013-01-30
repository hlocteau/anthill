#include <io/GatherFolderImg.hpp>
#include <connexcomponentextractor.h>
#include <Bounding.hpp>

#include <DGtal/helpers/StdDefs.h>
#include <io/IOPgm3d.h>
#include <DistanceTransform.hpp>

/**
template <typename IN, typename OUT>
BillonTpl<OUT> * BilloncastTo<IN,OUT>( const BillonTpl<IN> *src ) {
	BillonTpl<OUT> out = new BillonTpl<OUT>( src->n_rows, src->n_cols, src->n_slices ) ;
	int z,x,y;
	for ( z=0;z<src->n_slices;z++)
		for ( y=0;y<src->n_rows;y++ )
			for ( x=0;x<src->n_cols;x++ )
				(*out)(y,x,z) = (OUT) (*src)(y,x,z);
	return out ;
}
*/

typedef ConnexComponentExtractor<arma::u8,arma::u16>			CS_CCExtractor ;
typedef Bounding< CS_CCExtractor::value_type > 					MyBounding;
typedef GrayLevelHistogram< CS_CCExtractor::value_type > 		SHistogram ;
typedef DistanceTransform<arma::u8,arma::u32>					CIDistanceTransform ;
typedef GrayLevelHistogram< CIDistanceTransform::value_type > 	IHistogram ;

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
				(*result)(y,x,z) = (*labels)(y+bbox->second.first.at(1),x+bbox->second.first.at(0),z+bbox->second.first.at(2) ) == 0 ? 1 : 0 ;
			}
	
	return result ;
}

#define PROOF_COORD(a,b) (b<0?0:(b>=a?a-1:b))

int main( int narg, char **argv ) {
	if( !fs::exists( argv[1] ) ) return -1 ;
	
	GatherFolderImg *factory = new GatherFolderImg( argv[1] ) ;
	factory->load( 80 ) ;
	const arma::Mat<arma::u8> & mask = factory->mask() ;
	arma::Cube<arma::u8> mask3d( mask.n_rows, mask.n_cols, 1 ) ;
	mask3d.slice(0) = mask ;
	
	fs::path folderpath = argv[1] ;
	if ( !fs::is_directory( folderpath ) )
		folderpath = folderpath.parent_path() ;
	
	IOPgm3d<arma::u8,qint8,false>::write( mask3d, QString("%1/anthill.mask.pgm3d").arg( folderpath.c_str() ) ) ;
	mask3d.reset();
	
	const BillonTpl< arma::u8 > & scene = factory->scene() ;
	IOPgm3d<arma::u8,qint8,false>::write( scene, QString("%1/anthill.scene.pgm3d").arg( folderpath.c_str() ) ) ; /// mask has been applied, may be useful even while providing directly a "scene" file
	CS_CCExtractor extractor;
	BillonTpl<arma::u16> *labels = extractor.run( scene ) ;
	IOPgm3d<arma::u16,qint16,false>::write( *labels, QString("%1/anthill.labelscene.pgm3d").arg( folderpath.c_str() ) ) ;
	delete factory ;
	
	/// note : bounding boxes and volumes are computed by extractor...
	
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
				( biggestBox->second.second.at(0)-biggestBox->second.first.at(0) )+
				( biggestBox->second.second.at(1)-biggestBox->second.first.at(1) )*
				( biggestBox->second.second.at(1)-biggestBox->second.first.at(1) )+
				( biggestBox->second.second.at(2)-biggestBox->second.first.at(2) )*
				( biggestBox->second.second.at(2)-biggestBox->second.first.at(2) );
	const QMap<arma::u16,int> &volumes = extractor.volumes() ;
	const QMap<arma::u16, std::pair<iCoord3D, iCoord3D> > & bounds = extractor.bounds3D();
	for ( ++boxIter ; boxIter != lastBox ; boxIter++ ) {
		int curVol =( boxIter->second.second.at(0)-boxIter->second.first.at(0) )*
					( boxIter->second.second.at(0)-boxIter->second.first.at(0) )+
					( boxIter->second.second.at(1)-boxIter->second.first.at(1) )*
					( boxIter->second.second.at(1)-boxIter->second.first.at(1) )+
					( boxIter->second.second.at(2)-boxIter->second.first.at(2) )*
					( boxIter->second.second.at(2)-boxIter->second.first.at(2) );
		if ( bigVol < curVol ) {
			bigVol = curVol ;
			biggestBox = boxIter ;
		}
		//std::cerr<<boxIter->first<<" "<<boxIter->second.first<<" to "<<boxIter->second.second<<" volume "<<volumes[ boxIter->first ]<<std::endl;
		
	}
	
	/*
	short id ;
	std::cout<<"Draw component ";
	std::cin>>id;
	biggestBox = bboxes.find( id ) ;
	*/
	std::cout<<"BoundingBox "<<biggestBox->second.first<<" "<<biggestBox->second.second<<" volume "<<volumes[ biggestBox->first ]<<std::endl;
	
	arma::Cube<arma::u8> *result = bounding.convexHull2DAxis( 7, biggestBox ) ;
	IOPgm3d<arma::u8,qint8,false>::write( *result, QString("%1/anthillqhull.pgm3d").arg( folderpath.c_str() ) ) ;
	BillonTpl<arma::u8> *inner = cropComplement( result, labels, biggestBox ) ;
	IOPgm3d<arma::u8,qint8,false>::write( *inner, QString("%1/anthillallcontent.pgm3d").arg( folderpath.c_str() ) ) ;
	delete labels ;
	
	BillonTpl< CS_CCExtractor::value_type > *labelsInner = extractor.run( *inner ) ;
	IOPgm3d<arma::u16,qint16,false>::write( *labelsInner, QString("%1/anthilllabelinner.pgm3d").arg( folderpath.c_str() ) ) ; /// to make it viewable (redefine labels wrt volumes)
	SHistogram histInner( *labelsInner ) ;
	
	CIDistanceTransform *dtHull = new CIDistanceTransform( *result ) ;
	const arma::Cube< CIDistanceTransform::value_type > & imDtHull = dtHull->result() ;
	IOPgm3d<CIDistanceTransform::value_type,qint32,false>::write( imDtHull, QString("%1/anthill.dthull.pgm3d").arg( folderpath.c_str() ) ) ;
	
	std::map< CS_CCExtractor::value_type, CIDistanceTransform::value_type > distToHull ;
	std::set< CS_CCExtractor::value_type > touchingHull ;
	/// for each component of inner
	/// determine its min-max values on dtHull
	{
		register int x,y,z,neighbor ;
		CS_CCExtractor::value_type id_cc ;
		CIDistanceTransform::value_type value ;
		for ( z = 0 ; z < imDtHull.n_slices ; z++ )
			for ( x = 0 ; x < imDtHull.n_cols ; x++ )
				for ( y = 0 ; y < imDtHull.n_rows ; y++ ) {
					id_cc = (*labelsInner)( y,x,z) ;
					if ( id_cc == 0 ) continue ;
					value = imDtHull(y,x,z) ;
					if ( touchingHull.find( id_cc ) == touchingHull.end() ) {
						/// insert id_cc into touchingHull whenever we can find an adjacent voxel out of the hull
						for ( neighbor = 0 ; neighbor < 27 ; neighbor++ )
							if ( (*result)( PROOF_COORD(imDtHull.n_rows,y + (neighbor/9-1)), PROOF_COORD(imDtHull.n_cols,x + ( (neighbor%9)/3 -1 )), PROOF_COORD(imDtHull.n_slices,z + ( neighbor % 3 -1 ) ) ) == 0 ) break ;
						if ( neighbor != 27 ) touchingHull.insert( id_cc ) ;
					}
					if ( distToHull.find( id_cc ) == distToHull.end() )
						distToHull[ id_cc ] = value ;
					else {
						if ( distToHull[ id_cc ] < value ) distToHull[ id_cc ] = value ;
					}
				}
	}
	
	CS_CCExtractor::value_type biggestInnerCCIdentifier = 1 ;
	QFile fHist( "/tmp/histcomplement.txt") ;
	if( !fHist.open(QFile::WriteOnly) ) {
		std::cerr << "Error : saving size of conn comp on the complement of the scene"<< std::endl;
		return -1;
	}
	QTextStream out(&fHist);
	for ( SHistogram::THistogram::iterator it = histInner._bin.begin() ; it != histInner._bin.end() ; it++ ) {
		out<<QString("%1 : %2 : %3 : %4").arg((int)it->first).arg((long unsigned int)it->second).arg( distToHull[ (int)it->first ] ).arg( touchingHull.find(it->first) != touchingHull.end() ? "touch":"disconnected")<<endl;
		if ( it->second > histInner._bin[ biggestInnerCCIdentifier ] ) biggestInnerCCIdentifier = it->first ;
	}
	fHist.close() ;
	delete dtHull ;

	{
		/// Selection
		/// component not being adjacent to hull / volume  >= 250
		BillonTpl< short > imSelect( result->n_rows, result->n_cols, result->n_slices ) ;
		imSelect.fill(0);
		std::map< CS_CCExtractor::value_type, short > tblEncoding ;
		register int x,y,z;
		CS_CCExtractor::value_type id_cc ;
		for ( z = 0 ; z < labelsInner->n_slices ; z++ )
			for ( x = 0 ; x < labelsInner->n_cols ; x++ )
				for ( y = 0 ; y < labelsInner->n_rows ; y++ ) {
					id_cc = (*labelsInner)( y,x,z) ;
					if ( id_cc == 0 ) continue ;
					if ( histInner._bin[ id_cc ] < 250 ) continue ;
					if ( touchingHull.find( id_cc ) != touchingHull.end() ) continue ;
					if ( tblEncoding.find( id_cc ) == tblEncoding.end() )
						tblEncoding[ id_cc ] = tblEncoding.size()-1 ;
					imSelect(y,x,z) = tblEncoding[ id_cc ] ;
				}
		imSelect.setMinValue(0);
		imSelect.setMaxValue( tblEncoding.size() );
		IOPgm3d<short,qint16,false>::write( imSelect, QString("/tmp/selection_v3d.pgm3d"),QString("P3D") ) ;
	}
	//IOPgm3d<short,qint16,false>::write( *labelsInner, QString("/tmp/scene.pgm3d") ) ;
	
	/// seems that we can keep only the biggest one
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
		IOPgm3d<arma::u8,qint16,false>::write( imSelect, QString("%1/anthillcontent_v3d.pgm3d").arg( folderpath.c_str() ),QString("P3D") ) ;
		for ( z = 0 ; z < labelsInner->n_slices ; z++ )
			for ( x = 0 ; x < labelsInner->n_cols ; x++ )
				for ( y = 0 ; y < labelsInner->n_rows ; y++ )
					if ( biggestInnerCCIdentifier == (*labelsInner)( y,x,z) )
						imSelect(y,x,z) = 1 ;
		IOPgm3d<arma::u8,qint8,false>::write( imSelect, QString("%1/anthillcontent.pgm3d").arg( folderpath.c_str() ) ) ;
		delete labelsInner ;
		delete result ;
		delete inner ;
		
		
		/// compute distance transform on imSelect
		CIDistanceTransform dtObj( imSelect ) ;
		IHistogram depthObj( dtObj.result() ) ;
		QFile fHist( QString("%1/depthObjHist.txt").arg(folderpath.c_str())) ;
		if( !fHist.open(QFile::WriteOnly) ) {
			std::cerr << "Error : saving size of conn comp on the complement of the scene"<< std::endl;
			return -1;
		}
		QTextStream out(&fHist);
		for ( IHistogram::THistogram::iterator it = depthObj._bin.begin() ; it != depthObj._bin.end() ; it++ ) {
			out<<QString("%1 : %2").arg((int)it->first).arg((long unsigned int)it->second)<<endl;
		}
		fHist.close() ;
	}	
	return 1;
}
