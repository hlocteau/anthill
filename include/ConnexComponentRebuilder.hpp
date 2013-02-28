#ifndef CONNEX_COMPONENT_REBUILDER_HEADER
#define CONNEX_COMPONENT_REBUILDER_HEADER
#include <DGtal/helpers/StdDefs.h>
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

/**
 * \brief Rebuild an object from its skeleton and the corresponding depth map
 * 
 * Use the ReverseDistanceTransformation issued from DGtal but adding the management of a labeled skeleton.
 * As a voxel may be "rebuild" from several seeds belonging to the skeleton, distinct labels for those seeds
 * leads to conflicts that are first stored in the map _ill_per_voxel. This map is next proccessed to define
 * conflicts per set of labels, gathering all voxels that may be assigned to the same set of labels.
 * 
 * The post_processing() method deals with those ill defined voxels, assigning such a region to the biggest seed.
 * 
 * The tool rebuild.cpp takes advantage of the class ConnexComponentRebuilder.
 * Running tests can be done using the test parameter, e.g.:
 * \verbatim
	rebuildcc --test 6
   \endverbatim
 * It generates three 3D pgm files:
 *   - toy_dist.pgm3d : a distance map whom only voxels having a positive feature value are part of the skeleton;
 *   - toy_labels.pgm3d : the skeleton being labeled into 3 components;
 *   - toy_rebuild.pgm3D : the rebuild object (components).
 * 
 * \image html toy_dist.png "the filtered distance map"
 * \image html toy_labels.png "the labeled skeleton"
 * \image html toy_rebuild.png "the output"
 * 
 * See tool innerScene.cpp for an advanced usage.
 * 
 * \todo The content of innerScene.cpp should be part of this class as the bottleneck of DGtal's 
 * ReverseDistanceTransformation is memory management. Using tilling must be transparent for the user.
 * \tparam T type being used for the voxel in the input labeled image
 * \tparam U type being used for the distance map
 * \tparam V type being used for the voxel in the output labeled image (usually equals to T, but you do not care about labels in the output image)
 */
template <typename T, typename U, typename V> class ConnexComponentRebuilder {
public:
    typedef DGtal::ImageSelector< Domain, uint>::Type        Image ;
    typedef DGtal::ReverseDistanceTransformation< Image, 2 > RDT ;
    typedef RDT::OutputImage                                 OutImage ;
    typedef std::pair< DigitalSet*, Domain* >                LayerType ;
    typedef std::pair<Point,Point>                           VoxelsPair ;
    typedef struct _IllDefinedInstance {
        QList< V > _seeds ;
        /**
         * _idxVoxels indices of voxels that can be attached to any region enumerated in \a _seeds.
         *   the index values are those of the first region (i.e. _seeds.at(0))
         * \note if a voxel can be attached both to A, B and C, it does not appear in the instances (A,B), (B,C), (B,A) but only in (A,B,C).
         *   Thus, if it can be attached too to D, it does not appear in (A,B,C) but in (A,B,C,D).
         */
        QList< uint >     _idxVoxels ;
        std::string   seeds_as_string () const ;
        std::string  voxels_as_string () const ;
    } IllDefinedInstance ;
    typedef IllDefinedInstance* PtrIllDefinedInstance ;
    typedef struct _IllDefined {
        QList< PtrIllDefinedInstance > _instances ;
        QList< Point >                 _voxels ;
        std::string  voxels_as_string () const ;
        bool              first_child ( PtrIllDefinedInstance * inst ) ;
        bool               next_child ( const PtrIllDefinedInstance father, PtrIllDefinedInstance * child ) ;
    } IllDefined ;
    
    
    ConnexComponentRebuilder  ( const BillonTpl< T > &, QList< T > *ignoring = 0 ) ;
    ConnexComponentRebuilder  ( const QString & , QList< T > *ignoring = 0) ;
    ~ConnexComponentRebuilder ( ) ;
    
    bool                            setDepth ( BillonTpl< U > * ) ;
    bool                            setDepth ( const QString & ) ;
    bool                                 run ( bool storeOverlaping = false ) ;
    bool                                 run ( T label, V color, bool storeOverlaping = false ) ;
    
    const BillonTpl< V > &            result () const {
        return _result ;
    }
    
    void                     post_processing () ;
    
    QMap< V, uint32_t > &            volumes () { return _volumes ; }
    
    QMap< V, IllDefined > &       illDefined () { return _illDefined ; }
    
protected:
    void                                init ( const BillonTpl< T > &, QList< T > *ignoring) ;
    bool                           setBounds ( uint32_t selection ) ;
    void                          set_voxels ( const OutImage & img, const int * plane, const Point &seed, QList<Point> &crop, const Point &refPoint, U maxDist  ) ;
    void              explicit_missed_voxels ( V key ) const ;
    /**
     * \brief retrieve from the input set, the instance (father) being equal to elem (child) but the missing element
     * \param[in] inst : the input set
     * \param[in] seeds : the content of the child
     * \param[in] missing : the label of the region that is omitted in the father
     * \return the specific father of seeds
     */
    uint                     index_of_parent ( const QList < PtrIllDefinedInstance > & inst, const PtrIllDefinedInstance elem, uint missing, bool & ) ;
    /**
     * \copydoc index_of_parent(const QList<PtrIllDefinedInstance>&,const PtrIllDefinedInstance,uint,bool &)
     */
    uint                     index_of_parent ( const QList < PtrIllDefinedInstance > & inst, const QList<V > &elem, uint missing, bool & ) ;
    bool                        split_voxels ( PtrIllDefinedInstance father, const PtrIllDefinedInstance child ) ;
    bool                        split_voxels ( PtrIllDefinedInstance father, const QList< uint > &childVoxels ) ;
    void                  translate_idxVoxel ( const QList< uint > &idx_voxels, uint32_t ref, uint32_t newref, QList< uint > &idx_voxels_tr ) ;
    void               explicit_intersection ( const PtrIllDefinedInstance inst ) ;
    static bool           sortedListLessThan ( const PtrIllDefinedInstance &a, const PtrIllDefinedInstance &b ) ;
private:
    QMap< T, LayerType >           _layers ;
    BillonTpl< U >                 *_depth ;
    bool                           _allocated_depth ;
    QMap< T, Point >               _lower ;
    QMap< T, Point >               _upper ;
    QMap< T, uint32_t >            _volumes ;

    
    QMap< V, IllDefined >          _illDefined ;
    QMap< Point, QList< V > >      _ill_per_voxel ;
    T                              _n_labels ;
    uint32_t                       _n_rows ;
    uint32_t                       _n_cols ;
    uint32_t                       _n_slices ;
    QMap< T, QSet< T > >           _adjacency ;
    QMap< T, QList< VoxelsPair > > _locations ;
    BillonTpl< V >                 _result ;
    QString                        _depthfile ;
} ;
#include <ConnexComponentRebuilder.ih>
#endif
