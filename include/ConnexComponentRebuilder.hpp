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
	typedef struct _IllDefinedInstance {
		QList< V > _seeds ;
		/// \def _idxVoxels indices of voxels that can be attached to any region enumerated in _seeds
		/// the index values are those of the first region (i.e. _seeds.at(0))
		/// \note if a voxel can be attached both to A, B and C, it does not appear in the instances (A,B), (B,C), (B,A) but only in (A,B,C).
		///       Thus, if it can be attached too to D, it does not appear in (A,B,C) but in (A,B,C,D).
		QList< uint >     _idxVoxels ;
		std::string  seeds_as_string( ) const ;
		std::string  voxels_as_string( ) const ;
	} IllDefinedInstance ;
	typedef IllDefinedInstance* PtrIllDefinedInstance ;
	typedef struct _IllDefined {
		QList< PtrIllDefinedInstance > _instances ;
		QList< Point >     _voxels ;
		std::string  voxels_as_string( ) const ;
		bool first_child( PtrIllDefinedInstance * inst ) ;
		bool next_child( const PtrIllDefinedInstance father, PtrIllDefinedInstance * child ) ;
	} IllDefined ;
	
	
	ConnexComponentRebuilder			( const BillonTpl< T > &, QList< T > *ignoring = 0 ) ;
	ConnexComponentRebuilder			( const QString & , QList< T > *ignoring = 0) ;
	~ConnexComponentRebuilder			( ) ;
	
	bool 					setDepth 	( BillonTpl< U > * ) ;
	bool 					setDepth 	( const QString & ) ;
	bool 					run			( bool storeOverlaping = false ) ;
	bool 					run			( T label, V color, bool storeOverlaping = false ) ;
	
	const BillonTpl< V > &	result		( ) 						const {
		return _result ;
	}
	
	void                    post_processing  ( ) ;
	
	QMap< V, uint32_t > &volumes() { return _volumes ; }
	
	QMap< V, IllDefined > &              illDefined() { return _illDefined ; }
	
protected:
	void					init 		     ( const BillonTpl< T > &, QList< T > *ignoring) ;
	bool					setBounds	     ( uint32_t selection ) ;
	void                    set_voxels       ( const OutImage & img, const int * plane, const Point &seed, QList<Point> &crop, const Point &refPoint, U maxDist  ) ;
	void              explicit_missed_voxels ( V key ) const ;
	/**
	 * \brief retrieve from the input set, the instance (father) being equal to elem (child) but the missing element
	 * \param[in] inst : the input set
	 * \param[in] seeds : the content of the child
	 * \param[in] missing : the label of the region that is omitted in the father
	 * \return the specific father of seeds
	 */
	uint                  index_of_parent    ( const QList < PtrIllDefinedInstance > & inst, const PtrIllDefinedInstance elem, uint missing, bool & ) ;
	/**
	 * \brief retrieve from the input set, the instance (father) being equal to elem (child) but the missing element
	 * \param[in] inst : the input set
	 * \param[in] elem : the child
	 * \param[in] missing : the label of the region that is omitted in the father
	 * \return the specific father of elem
	 */
	uint                  index_of_parent    ( const QList < PtrIllDefinedInstance > & inst, const QList<V > &elem, uint missing, bool & ) ;
	bool                  split_voxels       ( PtrIllDefinedInstance father, const PtrIllDefinedInstance child ) ;
	bool                  split_voxels       ( PtrIllDefinedInstance father, const QList< uint > &childVoxels ) ;
	void                  translate_idxVoxel ( const QList< uint > &idx_voxels, uint32_t ref, uint32_t newref, QList< uint > &idx_voxels_tr ) ;
	void              explicit_intersection  ( const PtrIllDefinedInstance inst ) ;
	static bool           sortedListLessThan ( const PtrIllDefinedInstance &a, const PtrIllDefinedInstance &b ) ;
private:
	QMap< T, LayerType > 			_layers ;
	BillonTpl< U >							*_depth ;
	bool									_allocated_depth ;
	QMap< T, Point >					_lower ;
	QMap< T, Point >					_upper ;
	QMap< T, uint32_t >              _volumes ;

	
	QMap< V, IllDefined >            _illDefined ;
	QMap< Point, QList< V > >      _ill_per_voxel ;
	T								_n_labels ;
	uint32_t								_n_rows ;
	uint32_t								_n_cols ;
	uint32_t								_n_slices ;
	QMap< T, QSet< T > >		_adjacency ;
	QMap< T, QList< VoxelsPair > >	_locations ;
	BillonTpl< V >							_result ;
	QString									_depthfile ;
} ;

#include <ConnexComponentRebuilder.ih>
#endif
