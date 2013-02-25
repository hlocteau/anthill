#ifndef REGION_ADJACENCY_GRAPH_HEADER
#define REGION_ADJACENCY_GRAPH_HEADER

#include <boost/property_map/property_map.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <QList>
#include <DGtal/helpers/StdDefs.h>

#include <billon.h>
#include <cast_integer.h>

#include <boost/shared_ptr.hpp>

class NodeData {
	public:
		uint _id ;
		uint _volume ;
		uint _category ;

		NodeData ( ) ;
		~NodeData( ) ;
		NodeData ( const NodeData & o ) ;
		NodeData & operator = ( const NodeData &o ) ;
		
		void setId       ( uint id ) {
			_id = id ;
		}
		void setVolume   ( uint volume ) {
			_volume = volume ;
		}
		void setCategory ( uint category ) {
			_category = category ;
		}
		
		uint id          (  ) const {
			return _id ;
		}
		uint volume      (  ) const {
			return _volume ;
		}
		uint category    (  ) const {
			return _category ;
		}
		
		NodeData & operator += ( const NodeData & o ) ;
} ;

#define STORAGE_VOXEL

class EdgeData {
	private:
		uint _source_id ;
		uint _target_id ;
		#ifdef STORAGE_VOXEL
		boost::shared_ptr< QList< DGtal::Z3i::Point > > _per_side[2] ;
		#endif
	public:
		EdgeData ( ) ;
		~EdgeData( ) ;
		EdgeData ( const EdgeData & o ) ;
		EdgeData & operator = ( const EdgeData &o ) ;
		
		void reset() ;
		
		void setSource   ( uint source_id ) {
			_source_id = source_id ;
			#ifdef STORAGE_VOXEL
			if ( ! _per_side[0] ) {
				_per_side[0] = boost::shared_ptr< QList<DGtal::Z3i::Point> >( new QList<DGtal::Z3i::Point> );
				_per_side[1] = boost::shared_ptr< QList<DGtal::Z3i::Point> >( new QList<DGtal::Z3i::Point> );
			}
			#ifdef DEV_RAG_METHODS
			std::cout<<"def ("<<_source_id<<","<<_target_id<<") adr "<<_per_side[0]<<" "<<_per_side[1]<<std::endl;
			#endif
			#endif
		}
		void setTarget   ( uint target_id ) {
			_target_id = target_id ;
			#ifdef STORAGE_VOXEL
			if ( ! _per_side[0] ) {
				_per_side[0] = boost::shared_ptr< QList<DGtal::Z3i::Point> >( new QList<DGtal::Z3i::Point> );
				_per_side[1] = boost::shared_ptr< QList<DGtal::Z3i::Point> >( new QList<DGtal::Z3i::Point> );
			}
			#ifdef DEV_RAG_METHODS
			std::cout<<"def ("<<_source_id<<","<<_target_id<<") adr "<<_per_side[0]<<" "<<_per_side[1]<<std::endl;
			#endif
			#endif
		}
		
		uint source_id   (  ) const {
			return _source_id ;
		}
		uint target_id   (  ) const {
			return _target_id ;
		}
		#ifdef STORAGE_VOXEL
		boost::shared_ptr< QList<DGtal::Z3i::Point> >  per_side    ( uint id ) const {
			if ( id == _source_id )
				return _per_side[0] ;
			else
				return _per_side[1] ;
		}
		void append( uint id, DGtal::Z3i::Point v ) ;
		#endif
		EdgeData & operator += ( const EdgeData & o ) ;
} ;
	
struct _NodeTag {
	typedef boost::vertex_property_tag kind ;
} ;
struct _EdgeTag {
	typedef boost::edge_property_tag kind ;
} ;
	
typedef boost::property< _NodeTag, NodeData > NodeProperty ;
typedef boost::property< _EdgeTag, EdgeData > EdgeProperty ;

typedef boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS, NodeProperty, EdgeProperty > GraphAdj;


template <typename T > GraphAdj * init_rag( const BillonTpl< T > &label, T th ) {
	uint max_value = cast_integer< T, uint>( label.max() );
	if ( max_value < th ) return 0 ;
	std::cout<<__FUNCTION__<<std::endl;
	GraphAdj * g = new GraphAdj( max_value+1 ) ;
	std::cout<<max_value+1<<" nodes have been inserted in the graph..."<<std::endl;
	boost::property_map< GraphAdj, _NodeTag >::type NodePropertyMap  = boost::get( _NodeTag(), *g ) ;
	boost::property_map< GraphAdj, _EdgeTag >::type EdgePropertyMap  = boost::get( _EdgeTag(), *g ) ;
	bool creation,
	     existing ;
	
	uint x,y,z,n ;
	uint n_slices = label.n_slices,
	     n_cols = label.n_cols,
	     n_rows = label.n_rows;
	T cur_value, adj_value ;
	for ( z = 0 ; z < n_slices ; z++ )
		for ( y = 0 ; y < n_rows ; y++ )
			for ( x = 0 ; x < n_cols ; x++ ) {
				cur_value = label( y,x,z ) ;
				if ( cur_value < th ) continue ;
				GraphAdj::vertex_descriptor vertex_cur = boost::vertex( cur_value, *g ) ;
				NodeData & data_cur = NodePropertyMap[ vertex_cur ];
				data_cur._id = cur_value  ;
				data_cur._volume++ ;
				for ( n = 0 ; n < 27 ; n++ )
					if ( ( y + (n/9-1)) >= 0 		&& ( y + (n/9-1)) < n_rows &&
						 ( x + ( (n%9)/3 -1 )) >= 0 && ( x + ( (n%9)/3 -1 ))<n_cols &&
						 ( z + ( n % 3 -1 )) >= 0 	&& ( z + ( n % 3 -1 ))<n_slices ) {
						adj_value = label( y + (n/9-1), x + ( (n%9)/3 -1 ), z + ( n % 3 -1 ) ) ;
						if ( adj_value && adj_value != cur_value ) {
							GraphAdj::vertex_descriptor vertex_adj = boost::vertex( adj_value, *g ) ;
							GraphAdj::edge_descriptor edge_adj ;
							boost::tie( edge_adj, existing ) = boost::edge( vertex_cur, vertex_adj, *g ) ;
							if ( !existing ) boost::tie(edge_adj,creation) = boost::add_edge( vertex_cur, vertex_adj, *g ) ;
							EdgeData & edge_data = EdgePropertyMap[ edge_adj ] ;
							if ( !existing && creation ) {
								#ifdef DEV_RAG_METHODS
								std::cout<<"really define edge "<<cast_integer<T,int>( cur_value ) <<" "<<cast_integer<T,int>( adj_value ) << std::endl;
								#endif
								edge_data.setSource( cur_value ) ;
								edge_data.setTarget( adj_value ) ;
							}
							#ifdef STORAGE_VOXEL
							edge_data.append( cur_value, DGtal::Z3i::Point(x,y,z) ) ;
							edge_data.append( adj_value, DGtal::Z3i::Point(x + ( (n%9)/3 -1 ),y + (n/9-1),z+ ( n % 3 -1 )) ) ;
							#endif
						}
					}
			}
	return g ;
}


void merge_nodes( GraphAdj::vertex_descriptor growing, GraphAdj::vertex_descriptor other, GraphAdj & g ) ;

#endif
