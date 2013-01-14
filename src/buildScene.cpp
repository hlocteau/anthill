#include <io/GatherFolderImg.hpp>
#include <connexcomponentextractor.h>
#include <Bounding.hpp>

#include <DGtal/helpers/StdDefs.h>
#include <QtGui/qapplication.h>
#include <DGtal/io/viewers/Viewer3D.h>


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

typedef ConnexComponentExtractor<char,short>				CS_CCExtractor ;
typedef Bounding< CS_CCExtractor::value_type > 				MyBounding;
typedef GrayLevelHistogram< CS_CCExtractor::value_type > 	SHistogram ;
typedef DistanceTransform<char,int32_t>						CIDistanceTransform ;
typedef GrayLevelHistogram< CIDistanceTransform::value_type > IHistogram ;

BillonTpl<char> *cropComplement( const arma::Cube<char> *universe, const BillonTpl<short> *labels, MyBounding::TBBoxConstIterator bbox ) {
	BillonTpl<char> *result = new BillonTpl<char>( universe->n_rows, universe->n_cols, universe->n_slices ) ;
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
	GatherFolderImg *factory = new GatherFolderImg( argv[1] ) ;
	factory->load( 80 ) ;
	const arma::Mat<char> & mask = factory->mask() ;
	arma::Cube<char> mask3d( mask.n_rows, mask.n_cols, 1 ) ;
	mask3d.slice(0) = mask ;
	IOPgm3d<char,qint8,false>::write( mask3d, QString("%1/anthill.mask.pgm3d").arg( argv[1] ) ) ;
	mask3d.reset();
	
	const BillonTpl< char > & scene = factory->scene() ;
	IOPgm3d<char,qint8,false>::write( scene, QString("%1/anthill.scene.pgm3d").arg( argv[1] ) ) ;
	CS_CCExtractor extractor;
	BillonTpl<short> *labels = extractor.run( scene ) ;
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
	const QMap<short,int> &volumes = extractor.volumes() ;
	const QMap<short, std::pair<iCoord3D, iCoord3D> > & bounds = extractor.bounds3D();
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
	
	arma::Cube<char> *result = bounding.convexHull2DAxis( 7, biggestBox ) ;
	IOPgm3d<char,qint8,false>::write( *result, QString("%1/anthillqhull.pgm3d").arg( argv[1] ) ) ;
	BillonTpl<char> *inner = cropComplement( result, labels, biggestBox ) ;
	IOPgm3d<char,qint8,false>::write( *inner, QString("%1/anthillallcontent.pgm3d").arg( argv[1] ) ) ;
	delete labels ;
	
	BillonTpl< CS_CCExtractor::value_type > *labelsInner = extractor.run( *inner ) ;
	SHistogram histInner( *labelsInner ) ;
	
	CIDistanceTransform *dtHull = new CIDistanceTransform( *result ) ;
	const arma::Cube< CIDistanceTransform::value_type > & imDtHull = dtHull->result() ;
	IOPgm3d<CIDistanceTransform::value_type,qint32,false>::write( imDtHull, QString("%1/anthill.dthull.pgm3d").arg( argv[1] ) ) ;
	
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
		BillonTpl<char> imSelect ( labelsInner->n_rows, labelsInner->n_cols, labelsInner->n_slices ) ;
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
		IOPgm3d<char,qint16,false>::write( imSelect, QString("%1/anthillcontent_v3d.pgm3d").arg( argv[1] ),QString("P3D") ) ;
		for ( z = 0 ; z < labelsInner->n_slices ; z++ )
			for ( x = 0 ; x < labelsInner->n_cols ; x++ )
				for ( y = 0 ; y < labelsInner->n_rows ; y++ )
					if ( biggestInnerCCIdentifier == (*labelsInner)( y,x,z) )
						imSelect(y,x,z) = 1 ;
		IOPgm3d<char,qint8,false>::write( imSelect, QString("%1/anthillcontent.pgm3d").arg( argv[1] ) ) ;
		delete labelsInner ;
		delete result ;
		delete inner ;
		
		
		/// compute distance transform on imSelect
		CIDistanceTransform dtObj( imSelect ) ;
		IHistogram depthObj( dtObj.result() ) ;
		QFile fHist( QString("%1/depthObjHist.txt").arg(argv[1])) ;
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
	
	
	
	
	#if 0
	QApplication application(narg,argv);
	Viewer3D viewer ;
	QString title = QString("convex hull of 3d objetcs - %1").arg(argv[1]);
	viewer.setWindowTitle( title.toStdString().c_str() );
	viewer.show();
	Z3i::Domain domain ( biggestBox->second.first,biggestBox->second.second ) ;
	viewer<<SetMode3D( domain.className(), "BoundingBox" ) ;
	viewer<<domain ;
	int x,y,z;
	int n_rows = (*result).n_rows,
		n_cols = (*result).n_cols,
		n_slices = (*result).n_slices ;

	/*
	viewer<<CustomColors3D( Color(255,255,0,220), Color(255,255,0,220) ) ;
	for ( y = biggestBox->second.first.at(1) ; y <=biggestBox->second.second.at(1) ; y++ )
		for ( x = biggestBox->second.first.at(0) ; x <= biggestBox->second.second.at(0) ; x++ )
			for ( z = biggestBox->second.first.at(2) ; z <= biggestBox->second.second.at(2) ; z++ )
				if ( (*labels)( y,x,z ) == id ) {
					viewer << Z3i::Point(x,y,z) ;
				}
	*/
	viewer<<CustomColors3D( Color(255,255,255,10), Color(255,255,255,10) ) ;
	
	arma::Cube<char> *toDisplay = inner ;
	uint64_t nVoxels = 0 ;
	for ( y = 0 ; y < n_rows ; y++ )
		for ( x = 0 ; x < n_cols ; x++ )
			for ( z = 0 ; z < n_slices ; z++ )
				if ( (*toDisplay)( y,x,z)>0) {
					if (toDisplay->at(y,max(0,x-1),z) ==0 || toDisplay->at(y, min(n_cols-1,x+1),z) ==0 || 
						toDisplay->at(max(0,y-1),x,z) ==0 || toDisplay->at(min(n_rows-1,y+1),x,z) ==0 || 
						toDisplay->at(y,x,max(0,z-1)) ==0 || toDisplay->at(y,x,min(n_slices-1,z+1)) ==0|| ( x==0||y==0||z==0||x==n_cols-1||y==n_rows-1||z==n_slices-1) ) {
						nVoxels++ ;
						viewer << Z3i::Point(x+biggestBox->second.first.at(0),y+biggestBox->second.first.at(1),z+biggestBox->second.first.at(2)) ;
					}
				}
	std::cout<<"Display "<<nVoxels<<" voxel(s)"<<std::endl;
	/*
	viewer<<CustomColors3D( Color(255,0,0), Color(255,0,0) ) ;
	const arma::Mat<char> &mask = factory.mask() ;
	for ( x = 0 ; x < scene.n_cols ; x++ ) {
		for ( y = 0;y<scene.n_rows ; y++ )
			if ( mask(y,x)==(char)1 ) break ;
		for ( z = 0 ; z < scene.n_slices ; z++ )
			viewer << Z3i::Point(x,y,z) ;
	}
	*/
	viewer << Viewer3D::updateDisplay;
	toDisplay=0;
	#endif
	
	
	return 1;//application.exec();
}
