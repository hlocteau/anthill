#ifndef SKELETON_TO_GRAPH_HEADER
#define SKELETON_TO_GRAPH_HEADER

#include <billon.h>
#include <coordinate.h>
#include <def_coordinate.h>
#include <QMap>
#include <boost/graph/adjacency_list.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <QList>

using namespace boost;

template <typename T> class SkeletonGraph {
public:

	typedef adjacency_list < vecS, vecS, undirectedS > 		graph_t;
	typedef graph_traits < graph_t >::vertex_descriptor 	Vertex;
	typedef graph_traits < graph_t >::edge_descriptor 		Edge;
	typedef graph_traits < graph_t >::vertices_size_type 	Size;
	typedef Size* 											Iiter;

	typedef QMap<uint32_t,int32_t>::ConstIterator			ConstVoxelIterator ;
	typedef std::pair< Vertex, Vertex >						VerticesPair ; 
	SkeletonGraph( const BillonTpl<T> &img, T label ) ;
	~SkeletonGraph ( ) ;
	bool 				isolated_vertex			( const BillonTpl<T> &img ) 			const ;
	
	bool				hide_edges				( QList< VerticesPair > & ) ;
	bool				restore_all_edges		( QList< VerticesPair > * pL = 0 ) ;

	inline
	const graph_t &		graph					( )										const {
		return *_pg ;
	}
	inline
	int32_t				number_of_vertices		( )										const {
		return _n_vertices ;
	}
	int32_t				number_of_edges			( )										const {
		return _n_edges ;
	}
	inline
	ConstVoxelIterator	encoding_begin			( )										const {
		return _decVoxel.begin() ;
	}
	inline
	ConstVoxelIterator	encoding_end			( )										const {
		return _decVoxel.end() ;
	}						
	
	inline iCoord3D		operator [] 			( int32_t v )							const {
		return from_linear_coord( _decVoxel.keys( v )[ 0 ] ) ;
	}
	int32_t				node					( const iCoord3D &pt )					const {
		return _decVoxel.find( linear_coord( pt ) ).value() ;
	}
	
	void				size					( int &rows, int &cols, int &slices ) 	const {
		rows = _n_rows ;
		cols = _n_cols ;
		slices = _n_slices ;
	}
	
	inline uint32_t 	linear_coord			( uint32_t x, uint32_t y, uint32_t z ) 	const ;
	inline uint32_t 	linear_coord			( const iCoord3D &pt ) 					const ;
	inline iCoord3D 	from_linear_coord		( uint32_t v ) 							const ;
	inline int 			x_from_linear_coord		( uint32_t v ) 							const ;
	inline int 			y_from_linear_coord		( uint32_t v ) 							const ;
	inline int 			z_from_linear_coord		( uint32_t v ) 							const ;
protected:
	void 		compute					( const BillonTpl<T> &img ) ;
private:
	int 					_n_cols ;
	int 					_n_rows ;
	int 					_n_slices ;
	T						_selectedLabel ;
	int32_t 				_n_vertices ;
	int32_t 				_n_edges ;
	QMap<uint32_t,int32_t> 	_decVoxel ;
	graph_t* 				_pg ;
	
	QList< VerticesPair >	_hidden_edges ;
} ;

#include <SkeletonGraph.ih>

#endif
