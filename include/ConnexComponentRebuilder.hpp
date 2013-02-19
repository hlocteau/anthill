#ifndef CONNEX_COMPONENT_REBUILDER_HEADER
#define CONNEX_COMPONENT_REBUILDER_HEADER

#include <DGtal/images/ImageContainerBySTLVector.h>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/geometry/volumes/distance/ReverseDistanceTransformation.h>
#include <io/Pgm3dFactory.h>
#include <def_coordinate.h>
#include <coordinate.h>
#include <util_pair.h>
using DGtal::Z3i::DigitalSet ;
using DGtal::Z3i::Point ;
using DGtal::Z3i::Domain ;

template <typename T, typename U, typename V> class ConnexComponentRebuilder {
public:
	typedef DGtal::ImageSelector< Domain, uint>::Type 			Image ;
	typedef DGtal::ReverseDistanceTransformation< Image, 2 > 	RDT ;
	typedef RDT::OutputImage									OutImage ;
	typedef std::pair< DigitalSet*, Domain* >					LayerType ;
	typedef std::pair<Point,Point>								VoxelsPair ;
	typename typedef struct _IllRebuild {
		QList< V > _seeds ;
		QList< Point >     _voxels ;
	} IllRebuild ;

	
	
	ConnexComponentRebuilder			( const BillonTpl< T > &, QList< T > *ignoring = 0 ) ;
	ConnexComponentRebuilder			( const QString & , QList< T > *ignoring = 0) ;
	~ConnexComponentRebuilder			( ) ;
	
	bool 					setDepth 	( BillonTpl< U > * ) ;
	bool 					setDepth 	( const QString & ) ;
	bool 					run			( bool storeOverlaping = false ) ;
	bool 					run			( uint32_t label, V color, bool storeOverlaping = false ) ;
	
	const BillonTpl< V > &	result		( ) 						const {
		return _result ;
	}
	
	QMap< uint32_t, uint32_t > &volumes() { return _volumes ; }
	QMap< uint32_t,
	       QMap< uint32_t, QList< Point > > > & sharedVoxels() { return _sharedVoxels ; }
	
protected:
	void					init 		( const BillonTpl< T > &, QList< T > *ignoring) ;
	bool					setBounds	( uint32_t selection ) ;
	void                    set_voxels  ( const OutImage & img, const int * plane, const Point &seed, QList<Point> &crop, const Point &refPoint, U maxDist  ) ;
private:
	QMap< uint32_t, LayerType > 			_layers ;
	BillonTpl< U >							*_depth ;
	bool									_allocated_depth ;
	QMap< uint32_t, Point >					_lower ;
	QMap< uint32_t, Point >					_upper ;
	QMap< uint32_t, uint32_t >              _volumes ;
	QMap< uint32_t,
	       QMap< uint32_t, QList<Point> > > _sharedVoxels ;
	uint32_t								_n_labels ;
	uint32_t								_n_rows ;
	uint32_t								_n_cols ;
	uint32_t								_n_slices ;
	QMap< uint32_t, QSet< uint32_t > >		_adjacency ;
	QMap< uint32_t, QList< VoxelsPair > >	_locations ;
	BillonTpl< V >							_result ;
	QString									_depthfile ;
} ;

#include <ConnexComponentRebuilder.ih>
#endif
