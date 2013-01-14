#include <io/Pgm3dFactory.h>
#include <connexcomponentextractor.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/range/irange.hpp>
#include <DGtal/helpers/StdDefs.h>

//#define CHECK_TOY_PROBLEM_OUTPUT

using namespace boost;
using DGtal::Z3i::Point ;

typedef Pgm3dFactory<char> 	CPgm3dFactory ;
typedef BillonTpl<char>		CImage ;
typedef Pgm3dFactory<int32_t> 	IPgm3dFactory ;
typedef int32_t				im_elem_sp_type ;
typedef BillonTpl< im_elem_sp_type > ISPImage ;
typedef ConnexComponentExtractor<CPgm3dFactory::value_type,int16_t> CCExtractor ;


typedef adjacency_list < vecS, vecS, undirectedS > graph_t;
typedef graph_traits < graph_t >::vertex_descriptor Vertex;
typedef graph_traits < graph_t >::edge_descriptor Edge;

typedef graph_traits < graph_t >::vertices_size_type Size;
typedef Size* Iiter;

  bool bNeedIncrement ;
template < typename TimeMap > class bfs_time_visitor:public default_bfs_visitor {
  typedef typename property_traits < TimeMap >::value_type T;
public:
  bfs_time_visitor(TimeMap tmap, T & t):m_timemap(tmap), m_time(t) { }
  template < typename Vertex, typename Graph >
    void discover_vertex(Vertex u, const Graph & g) const
  {
    std::cout<<"-> visiting vertex "<<vertex(u,g)<<" and set its discovery time "<<m_time<<std::endl;
    bNeedIncrement = true ;
    put(m_timemap, u, m_time);
  }
  template < typename Vertex, typename Graph >
    void finish_vertex(Vertex u, const Graph & g) const
  {
	if ( !bNeedIncrement ) m_time--;
	//std::cout<<"finish vertex"<<vertex(u,g)<<" at t="<<m_time<<std::endl;  
  }
  template < typename Edge, typename Graph >
  void gray_target( Edge e, const Graph &g ) const {
	//std::cout<<"gray target, vertex"<<target(e,g)<<" at t="<<m_time<<std::endl;  
  }
  template < typename Edge, typename Graph >
  void black_target( Edge e, const Graph &g ) const {
	//std::cout<<"black target, vertex"<<target(e,g)<<" at t="<<m_time<<std::endl;  
  }

  template < typename Vertex, typename Graph >
    void examine_vertex(Vertex u, const Graph & g) const
  {
	m_time++;  
	//std::cout<<"--> examination of vertex "<<vertex(u,g)<<" at t="<<m_time<<std::endl;  
	bNeedIncrement = false ;
  }

  TimeMap m_timemap;
  T & m_time;
};

#ifndef CHECK_TOY_PROBLEM_OUTPUT

int x_from_linear_coord( uint32_t v, int n_cols, int n_rows, int n_slices ) {
	return v % n_cols ;
}
int y_from_linear_coord( uint32_t v, int n_cols, int n_rows, int n_slices ) {
	return (v / n_cols) % n_rows ;
}
int z_from_linear_coord( uint32_t v, int n_cols, int n_rows, int n_slices ) {
	return (v / n_cols) / n_rows ;
}

/// \note it is unnecessary to provide the image's dimension each time!
iCoord3D from_linear_coord( uint32_t v, int n_cols, int n_rows, int n_slices ) {
	return iCoord3D( x_from_linear_coord(v,n_cols,n_rows,n_slices),y_from_linear_coord(v,n_cols,n_rows,n_slices),z_from_linear_coord(v,n_cols,n_rows,n_slices) ) ;
}

uint32_t linear_coord( uint32_t x, uint32_t y, uint32_t z, int n_cols, int n_rows, int n_slices ) {
	return ( z * n_rows + y ) * n_cols + x ;
} 

#define CHECK_VERTEX_DEGREE_3DIMAGE(node,graph)

#define CHECK_VERTEX_DEGREE_3DIMAGE2(node,graph) {\
	if ( degree( vertex( node, graph ),graph ) > 26 ) {\
		std::cerr<<"node "<<node<<" / voxel " ;\
		QList<uint32_t> keys = encodingVoxel.keys(node) ;\
		std::cerr<<x_from_linear_coord( keys[0], img.n_cols,img.n_rows,img.n_slices )<<",";\
		std::cerr<<y_from_linear_coord( keys[0], img.n_cols,img.n_rows,img.n_slices )<<",";\
		std::cerr<<z_from_linear_coord( keys[0], img.n_cols,img.n_rows,img.n_slices )<<std::endl;\
		typename graph_t::adjacency_iterator adj;\
		typename graph_t::adjacency_iterator adj_end;\
		boost::tie(adj,adj_end) = adjacent_vertices( node,graph );\
		keys.clear();\
		for ( ; adj != adj_end ; adj++ ) {\
			keys = encodingVoxel.keys(*adj) ;\
			std::cerr<<"adjacent node "<<*adj<<" / voxel " ;\
			std::cerr<<x_from_linear_coord( keys[0], img.n_cols,img.n_rows,img.n_slices )<<",";\
			std::cerr<<y_from_linear_coord( keys[0], img.n_cols,img.n_rows,img.n_slices )<<",";\
			std::cerr<<z_from_linear_coord( keys[0], img.n_cols,img.n_rows,img.n_slices )<<std::endl;\
		}\
		exit(-2);\
	}\
}

#define ADD_EDGE(source,target,graph) add_edge(source,target,graph);

#define ADD_EDGE2(source,target,graph) {\
	QList<uint32_t> keys = encodingVoxel.keys(source);\
	if ( keys.size() != 1 ) {\
		std::cerr<<"Error : retrieve "<<keys.size()<<" key(s) from value "<<source<<" @line "<<__LINE__<<std::endl;\
		exit(-3) ;\
	}\
	uint32_t source_key = keys[0];\
	keys.clear() ;\
	keys = encodingVoxel.keys(target);\
	if ( keys.size() != 1 ) {\
		std::cerr<<"Error : retrieve "<<keys.size()<<" key(s) from value "<<target<<" @line "<<__LINE__<<std::endl;\
		exit(-3) ;\
	}\
	uint32_t target_key = keys[0];\
	int source_x = x_from_linear_coord( source_key, img.n_cols, img.n_rows, img.n_slices );\
	int source_y = y_from_linear_coord( source_key, img.n_cols, img.n_rows, img.n_slices );\
	int source_z = z_from_linear_coord( source_key, img.n_cols, img.n_rows, img.n_slices );\
	int target_x = x_from_linear_coord( target_key, img.n_cols, img.n_rows, img.n_slices );\
	int target_y = y_from_linear_coord( target_key, img.n_cols, img.n_rows, img.n_slices );\
	int target_z = z_from_linear_coord( target_key, img.n_cols, img.n_rows, img.n_slices );\
	if ( abs(source_x-target_x)>1 || abs(source_y-target_y)>1 || abs(source_z-target_z)>1 ) {\
		std::cerr<<"Error : can not link node "<<source<<" to node "<<target<<" as theirs coordinates are respectively : ";\
		std::cerr<<source_x<<","<<source_y<<","<<source_z<<" and "<<target_x<<","<<target_y<<","<<target_z<<" @line "<<__LINE__<<std::endl;\
		exit(-3);\
	} else {\
		add_edge(source,target,graph);\
	}\
}


#define GET_VERTEX_ID(x,y,z) ( encodingVoxel.contains(linear_coord(x,y,z,img.n_cols,img.n_rows,img.n_slices))?encodingVoxel[linear_coord(x,y,z,img.n_cols,img.n_rows,img.n_slices)]:encodingVoxel.size()+1)

#define CHECK_VERTEX_ID(x,y,z)

#define CHECK_VERTEX_ID2(x,y,z) {\
	uint32_t value = linear_coord(x,y,z,img.n_cols,img.n_rows,img.n_slices) ;\
	if ( !encodingVoxel.contains(value) ) {\
		std::cerr<<"Error : encoding of voxel("<<x<<","<<y<<","<<z<<") is "<<value<<" while no graph's vertex gets this encoding"<<std::endl;\
		std::cerr<<"Info  : img("<<x<<","<<y<<","<<z<<") is "<<(int)img(y,x,z)<<" @line "<<__LINE__<<std::endl;\
		exit(-3);\
	}\
}

/// without sampling
template< typename T >
graph_t* imageToGraph( const BillonTpl< T > &img, QMap<uint32_t,int32_t> &encodingVoxel, T label ) {
	graph_t* g ;
	int x,y,z;
	int32_t vertex_number=0 ;
	/// node management
	for ( z = 0 ; z < img.n_slices ; z++ )
		for ( y = 0 ; y < img.n_rows ; y++ )
			for ( x = 0 ; x < img.n_cols ; x++ )
				if ( img( y, x, z ) != 0 && ( label == -1 || img( y, x, z ) == label ) ) {
					encodingVoxel[ linear_coord(x,y,z,img.n_cols,img.n_rows,img.n_slices) ] = vertex_number ;
					vertex_number++ ;
				}
	assert(vertex_number==encodingVoxel.size());
	g = new graph_t( encodingVoxel.size() ) ;
	int32_t edge_number = 0 ;
	int32_t n,t ;
	/// edge management
	for ( z = 0 ; z < img.n_slices ; z++ )
		for ( y = 0 ; y < img.n_rows ; y++ )
			for ( x = 0 ; x < img.n_cols ; x++ ) {
				if ( img( y, x, z ) != 0 ) {
					if ( label == -1 || img( y, x, z ) == label ) {
						n = GET_VERTEX_ID(x,y,z);
						CHECK_VERTEX_ID(x,y,z);
						assert( n < vertex_number ) ;
						
						//                  (z)^
						//                     |                                             (z)^
						//     	               g---------h---------i                            |
						//     	             / |       / |       / |                            |
						//     	           d---------e---------f   |                            |
						//     	         / |   |   / |   |   / |   |                            |
						//     	       a---------b---------c   |   |                            |
						//     	       |   |   7-|---|-----|---|---9                            |
						//     	       |   | / | |   | / | |   | / |                            |
						//     	       |   4-----|--[5]----|---6   |                            4---------5
						//     	       | / |   | | / |   | | / |   |                          / |       / |
						//     	       1---------2---------3   |   |                        1---------2   |
						//     	       |   |   G-|---|---H-|---|---I-----------> (x)        |   |     |   |
						//     	       |   | /   |   | /   |   | /                          |   |     |   |
						//     	       |   D-----|---E-----|---F                            |  [D]----|---E---------> (x)
						//     	       | /       | /       | /                              | /       | /
						//     	       A---------B---------C                                A---------B
						//          /                                                     /
						//        v (y)                                                 v (y)
						//

						if ( y < img.n_rows-1 )
							if ( img( y+1, x  , z   ) != 0 	&& ( label == -1 || img( y+1, x  , z   ) == label )	)	{ /** edge DA - Right figure */
								t = GET_VERTEX_ID(x  ,y+1,z) ;
								CHECK_VERTEX_ID(x  ,y+1,z) ; edge_number++ ;
								CHECK_VERTEX_DEGREE_3DIMAGE( n,*g ) ;
								CHECK_VERTEX_DEGREE_3DIMAGE( t,*g ) ;
								ADD_EDGE(n,t,*g) ;
							}
						if ( y < img.n_rows-1 && x < img.n_cols-1 )
							if ( img( y+1, x+1, z   ) != 0 	&& ( label == -1 || img( y+1, x+1, z   ) == label )	)	{ /** edge DB - Right figure */
								t = GET_VERTEX_ID(x+1,y+1,z) ;
								CHECK_VERTEX_ID(x+1,y+1,z) ; edge_number++ ;
								CHECK_VERTEX_DEGREE_3DIMAGE( n,*g ) ;
								CHECK_VERTEX_DEGREE_3DIMAGE( t,*g ) ;
								ADD_EDGE( n, t,*g) ;
							}
						if ( y < img.n_rows-1 && x < img.n_cols-1 && z < img.n_slices-1 )
							if ( img( y+1, x+1, z+1 ) != 0 	&& ( label == -1 || img( y+1, x+1, z+1 ) == label )	)	{ /** edge D2 - Right figure */
								t = GET_VERTEX_ID(x+1,y+1,z+1) ;
								CHECK_VERTEX_ID( x+1,y+1,z+1 ) ; edge_number++ ;
								CHECK_VERTEX_DEGREE_3DIMAGE( n,*g ) ;
								CHECK_VERTEX_DEGREE_3DIMAGE( t,*g ) ;
								ADD_EDGE( n, t,*g) ;
							}
						if ( y < img.n_rows-1 && z < img.n_slices-1 )
							if ( img( y+1, x  , z+1 ) != 0 	&& ( label == -1 || img( y+1, x  , z+1 ) == label )	)	{/** edge D1 - Right figure */
								t = GET_VERTEX_ID(x  ,y+1,z+1);
								CHECK_VERTEX_ID( x  ,y+1,z+1 ) ; edge_number++ ;
								CHECK_VERTEX_DEGREE_3DIMAGE( n,*g ) ;
								CHECK_VERTEX_DEGREE_3DIMAGE( t,*g ) ;
								ADD_EDGE( n, t,*g) ;
							}
						if ( z < img.n_slices-1 )
							if ( img( y  , x  , z+1 ) != 0 	&& ( label == -1 || img( y  , x  , z+1 ) == label )	)	{/** edge D4 - Right figure */
								t = GET_VERTEX_ID( x  ,y  ,z+1) ;
								CHECK_VERTEX_ID( x  ,y  ,z+1 ) ; edge_number++ ;
								CHECK_VERTEX_DEGREE_3DIMAGE( n,*g ) ;
								CHECK_VERTEX_DEGREE_3DIMAGE( t,*g ) ;
								ADD_EDGE( n, t,*g) ;
							}
						if ( x < img.n_cols-1 && z < img.n_slices-1 )
							if ( img( y  , x+1, z+1 ) != 0 	&& ( label == -1 || img( y  , x+1, z+1 ) == label )	)	{/** edge D5 - Right figure */
								t = GET_VERTEX_ID( x+1,y  ,z+1) ;
								CHECK_VERTEX_ID( x+1,y  ,z+1 ) ; edge_number++ ;
								CHECK_VERTEX_DEGREE_3DIMAGE( n,*g ) ;
								CHECK_VERTEX_DEGREE_3DIMAGE( t,*g ) ;
								ADD_EDGE( n, t,*g) ;
							}
						if ( x < img.n_cols-1 )
							if ( img( y  , x+1, z   ) != 0 	&& ( label == -1 || img( y  , x+1, z   ) == label )	)	{/** edge DE - Right figure */
								t = GET_VERTEX_ID( x+1,y  ,z) ;
								CHECK_VERTEX_ID( x+1,y  ,z ) ; edge_number++ ;
								CHECK_VERTEX_DEGREE_3DIMAGE( n,*g ) ;
								CHECK_VERTEX_DEGREE_3DIMAGE( t,*g ) ;
								ADD_EDGE( n, t,*g) ;
							}
					} /** END if ( label != -1 || img( y, x, z ) == label ) */
				} /** END if ( img( y, x, z ) != 0 ) */
				if ( y < img.n_rows-1 && z < img.n_slices-1 && x < img.n_cols-1 ) {
					if ( img( y  , x  , z+1 ) != 0 	&& ( label == -1 || img( y  , x  , z+1 ) == label )	&&
						 img( y+1, x+1, z   ) != 0 	&& ( label == -1 || img( y+1, x+1, z   ) == label ) )	{/** edge 4B - Right figure */
						n = GET_VERTEX_ID( x  ,y  ,z+1) ;
						t = GET_VERTEX_ID( x+1,y+1,z) ;
						CHECK_VERTEX_ID( x  ,y  ,z+1 ) ;
						CHECK_VERTEX_ID( x+1,y+1,z ) ; edge_number++ ;
						CHECK_VERTEX_DEGREE_3DIMAGE( n,*g ) ;
						CHECK_VERTEX_DEGREE_3DIMAGE( t,*g ) ;
						ADD_EDGE( n, t,*g) ;
					}
					if ( img( y+1, x  , z   ) != 0 	&& ( label == -1 || img( y+1, x  , z   ) == label )	&&
						 img( y  , x+1, z+1 ) != 0 	&& ( label == -1 || img( y  , x+1, z+1 ) == label ) )	{/** edge A5 - Right figure */
						n = GET_VERTEX_ID( x  ,y+1,z) ;
						t = GET_VERTEX_ID( x+1,y  ,z+1) ;
						CHECK_VERTEX_ID( x  ,y+1,z ) ;
						CHECK_VERTEX_ID( x+1,y  ,z+1 ) ; edge_number++ ;
						CHECK_VERTEX_DEGREE_3DIMAGE( n,*g ) ;
						CHECK_VERTEX_DEGREE_3DIMAGE( t,*g ) ;
						ADD_EDGE( n, t,*g) ;
					}
					if ( img( y+1, x  , z+1 ) != 0 	&& ( label == -1 || img( y+1, x  , z+1 ) == label )	&&
						 img( y  , x+1, z   ) != 0 	&& ( label == -1 || img( y  , x+1, z   ) == label ) )	{/** edge 1E - Right figure */
						n = GET_VERTEX_ID( x  ,y+1,z+1) ;
						t = GET_VERTEX_ID( x+1,y  ,z) ;
						CHECK_VERTEX_ID( x  ,y+1,z+1 ) ;
						CHECK_VERTEX_ID( x+1,y  ,z ) ;
						edge_number++ ;
						CHECK_VERTEX_DEGREE_3DIMAGE( n,*g ) ;
						CHECK_VERTEX_DEGREE_3DIMAGE( t,*g ) ;
						ADD_EDGE( n, t,*g) ;
					}
				}
				if ( y < img.n_rows-1 && z < img.n_slices-1 )
					if ( img( y+1, x  , z   ) != 0 	&& ( label == -1 || img( y+1, x  , z   ) == label )	&&
						 img( y  , x  , z+1 ) != 0 	&& ( label == -1 || img( y  , x  , z+1 ) == label ) )	{/** edge A4 - Right figure */
						n = GET_VERTEX_ID( x  ,y+1,z) ;
						t = GET_VERTEX_ID( x  ,y  ,z+1) ;
						CHECK_VERTEX_ID( x  ,y+1,z ) ;
						CHECK_VERTEX_ID( x  ,y  ,z+1 ) ; edge_number++ ;
						CHECK_VERTEX_DEGREE_3DIMAGE( n,*g ) ;
						CHECK_VERTEX_DEGREE_3DIMAGE( t,*g ) ;
						ADD_EDGE( n, t,*g) ;
					}
				if ( x < img.n_cols-1 && z < img.n_slices-1 )
					if ( img( y  , x  , z+1 ) != 0 	&& ( label == -1 || img( y  , x  , z+1 ) == label )	&&
						 img( y  , x+1, z   ) != 0 	&& ( label == -1 || img( y  , x+1, z   ) == label ) )	{/** edge 4E - Right figure */
						n = GET_VERTEX_ID( x  ,y  ,z+1) ;
						t = GET_VERTEX_ID( x+1,y  ,z) ;
						CHECK_VERTEX_ID( x  ,y  ,z+1 ) ;
						CHECK_VERTEX_ID( x+1,y  ,z ) ; edge_number++ ;
						CHECK_VERTEX_DEGREE_3DIMAGE( n,*g ) ;
						CHECK_VERTEX_DEGREE_3DIMAGE( t,*g ) ;
						ADD_EDGE( n, t,*g) ;
					}

				if ( x < img.n_cols-1 && y < img.n_rows-1 )
					if ( img( y+1, x  , z   ) != 0 	&& ( label == -1 || img( y+1, x  , z   ) == label )	&&
						 img( y  , x+1, z   ) != 0 	&& ( label == -1 || img( y  , x+1, z   ) == label ) )	{/** edge AE - Right figure */
						n = GET_VERTEX_ID( x  ,y+1,z) ;
						t = GET_VERTEX_ID( x+1,y  ,z) ;
						CHECK_VERTEX_ID( x  ,y+1,z ) ;
						CHECK_VERTEX_ID( x+1,y  ,z ) ; edge_number++ ;
						CHECK_VERTEX_DEGREE_3DIMAGE( n,*g ) ;
						CHECK_VERTEX_DEGREE_3DIMAGE( t,*g ) ;
						ADD_EDGE( n, t,*g) ;
					}

			}
	std::cout<<"Create "<< edge_number<<" edges"<<std::endl ;
	{
		QList<uint32_t> keys = encodingVoxel.keys( 0 ) ;
		std::cout<<"Degree of source node for bfs : "<<out_degree(vertex( 0, *g), *g)<<" vertex 0, voxel "<< x_from_linear_coord(keys[0],img.n_cols,img.n_rows,img.n_slices)<<","<<y_from_linear_coord(keys[0],img.n_cols,img.n_rows,img.n_slices)<<","<<z_from_linear_coord(keys[0],img.n_cols,img.n_rows,img.n_slices) <<std::endl;
	}
	
	QMap< uint32_t, int32_t>::iterator Iter = encodingVoxel.begin() ;
	bool bError = false ;
	for ( ;Iter != encodingVoxel.end() ; Iter++ ) {
		if ( out_degree( vertex( Iter.value(), *g), *g) != 0 ) continue ;
		x=x_from_linear_coord( Iter.key(), img.n_cols,img.n_rows,img.n_slices) ;
		y=y_from_linear_coord( Iter.key(), img.n_cols,img.n_rows,img.n_slices) ;
		z=z_from_linear_coord( Iter.key(), img.n_cols,img.n_rows,img.n_slices) ;
		std::cout<<"vertex "<< Iter.value()<<" is mapped to voxel ("<<x<<","<<y<<","<<z<<") is disconnected"<<std::endl;
		for ( int zz=max(0,z-1);zz<=min((int)img.n_slices-1,z+1);zz++ )
			std::cout<<"z = "<<zz<<"\t\t";
		std::cout<<std::endl;
		for ( int yy=max(0,y-1);yy<=min((int)img.n_rows-1,y+1);yy++ ) {
			for ( int zz=max(0,z-1);zz<=min((int)img.n_slices-1,z+1);zz++ ) {
				for ( int xx=max(0,x-1);xx<=min((int)img.n_cols-1,x+1);xx++ )
					std::cout<<(int)img(yy,xx,zz)<<" ";
				std::cout<<"\t\t" ;
			}
			std::cout<<std::endl;
		}
		bError = true ;
	}
	if ( bError ) exit(-3);
	
	typename graph_t::adjacency_iterator v, vend;
	std::list< Vertex > vertices ; vertices.push_back( vertex( 0, *g) ) ;
	std::set< Vertex > forbidden ;
    for ( int iLoop=0;iLoop<0&&!vertices.empty();iLoop++ ) {
		forbidden.insert( vertices.front() ) ;
		/// display mini-cube
		if ( false){
			QList<uint32_t> keys = encodingVoxel.keys( vertices.front() ) ;
			x = x_from_linear_coord( keys[0], img.n_cols, img.n_rows, img.n_slices ) ;
			y = y_from_linear_coord( keys[0], img.n_cols, img.n_rows, img.n_slices ) ;
			z = z_from_linear_coord( keys[0], img.n_cols, img.n_rows, img.n_slices ) ;
			for ( int zz=max(0,z-1);zz<=min((int)img.n_slices-1,z+1);zz++ )
				std::cout<<"z = "<<zz<<"\t\t";
			std::cout<<std::endl;
			for ( int yy=max(0,y-1);yy<=min((int)img.n_rows-1,y+1);yy++ ) {
				for ( int zz=max(0,z-1);zz<=min((int)img.n_slices-1,z+1);zz++ ) {
					for ( int xx=max(0,x-1);xx<=min((int)img.n_cols-1,x+1);xx++ )
						std::cout<<(int)img(yy,xx,zz)<<" ";
					std::cout<<"\t\t" ;
				}
				std::cout<<std::endl;
			}
		}		
		for (tie(v,vend) = adjacent_vertices(vertices.front(), *g); v != vend; ++v) {
			QList<uint32_t> keys = encodingVoxel.keys( *v ) ;
			std::cout<<"from "<< vertices.front()<<" adj vertex "<<*v<<" voxel "<<x_from_linear_coord(keys[0],img.n_cols,img.n_rows,img.n_slices)<<","<<y_from_linear_coord(keys[0],img.n_cols,img.n_rows,img.n_slices)<<","<<z_from_linear_coord(keys[0],img.n_cols,img.n_rows,img.n_slices)<<std::endl;
			if ( forbidden.find( *v ) == forbidden.end() ) {
				vertices.push_back( *v ) ;
				forbidden.insert( vertices.front() ) ;
			}
		}
		vertices.pop_front() ;
	}
	return g ;
}

bool avoid_adjcacent( QMap< uint32_t,int32_t> &samples, QMap< uint32_t,int32_t> &allsamples, graph_t *pGraph ) {
	typename graph_t::adjacency_iterator adj, adj_end;
	QList<uint32_t> keys ;
	bool bFound = false ;
	for ( QMap< uint32_t,int32_t>::iterator it = samples.begin() ; it != samples.end() ; it++ ) {
		for ( boost::tie(adj,adj_end) = adjacent_vertices( vertex( it.value(), *pGraph), *pGraph); adj != adj_end; adj++) {
			keys.clear() ;
			keys = allsamples.keys( *adj ) ;
			if ( samples.contains( keys[0] ) ) continue ; /// both of them are avoided
			std::cout<<"While "<<it.value()<<" is not treated, adjacent node "<<*adj<<" has been!"<<std::endl;
			bFound = true ;
		}
	}
	return bFound ;
}

bool reverse_avoid_adjcacent( QMap< uint32_t,int32_t> &samples, QMap< uint32_t,int32_t> &allsamples, graph_t *pGraph ) {
	typename graph_t::adjacency_iterator adj, adj_end;
	QList<uint32_t> keys ;
	bool bFound = false ;
	for ( QMap< uint32_t,int32_t>::iterator it = allsamples.begin() ; it != allsamples.end() ; it++ ) {
		if ( samples.contains( it.key() ) ) continue ;
		for ( boost::tie(adj,adj_end) = adjacent_vertices( vertex( it.value(), *pGraph), *pGraph); adj != adj_end; adj++) {
			keys.clear() ;
			keys = allsamples.keys( *adj ) ;
			if ( !samples.contains( keys[0] ) ) continue ; /// both of them are treated
			std::cout<<"While "<<it.value()<<" is treated, adjacent node "<<*adj<<" has not been!"<<std::endl;
			bFound = true ;
		}
	}
	return bFound ;
}


bool connectivity_map( QMap< uint32_t,int32_t> &samples, QMap< uint32_t,int32_t> &allsamples, const ISPImage & spimg, const BillonTpl< CCExtractor::value_type > * labels, CCExtractor::value_type theLabel, const graph_t *pGraph, const char *setName ) {
	bool bFound = false ;
	for ( QMap< uint32_t,int32_t>::iterator it = samples.begin() ; it != samples.end() ; it++ ) {
		int x = x_from_linear_coord( it.key(),spimg.n_cols, spimg.n_rows, spimg.n_slices ) ;
		int y = y_from_linear_coord( it.key(),spimg.n_cols, spimg.n_rows, spimg.n_slices ) ;
		int z = z_from_linear_coord( it.key(),spimg.n_cols, spimg.n_rows, spimg.n_slices ) ;
		int num_adj_vertices = degree( vertex(it.value(),*pGraph), *pGraph );
		int num_adj_voxels = 0 ;
		for ( int yy=max(0,y-1);yy<=min(y+1,(int)spimg.n_rows-1);yy++) {
			for ( int zz=max(0,z-1);zz<=min(z+1,(int)spimg.n_slices-1);zz++) {
				for ( int xx=max(0,x-1);xx<=min(x+1,(int)spimg.n_cols-1);xx++) {
					if ((*labels)(yy,xx,zz) == theLabel && ( xx!=x||yy!=y||zz!=z) ) num_adj_voxels++ ;
				}
			}
		}
		if ( num_adj_voxels != num_adj_vertices ) {
			std::cout<<setName<<" DEBUGING WARNING : WE GET HIM/ONE OF THEM"<<std::endl;
			bFound = true ; 
			std::cout<<setName<<" vertex "<<it.value()<<" == voxel "<<x<<" , "<<y<<" , "<<z<<"   degree "<<num_adj_vertices<<std::endl;
			typename graph_t::adjacency_iterator adj, adj_end;
			for ( boost::tie(adj,adj_end) = adjacent_vertices( vertex( it.value(), *pGraph), *pGraph); adj != adj_end; adj++) {
				QList<uint32_t> keys = allsamples.keys( *adj ) ;
				std::cout<<"\t\trecover adj vertex "<<*adj<<" voxel "<<x_from_linear_coord(keys[0],spimg.n_cols,spimg.n_rows,spimg.n_slices)<<","<<y_from_linear_coord(keys[0],spimg.n_cols,spimg.n_rows,spimg.n_slices)<<","<<z_from_linear_coord(keys[0],spimg.n_cols,spimg.n_rows,spimg.n_slices)<<std::endl;
				
				if ( samples.size() != allsamples.size() ) {
					keys.clear() ;
					keys = samples.keys( *adj ) ;
					if ( keys.empty() ) {
						std::cout<<"While source "<<it.value()<<" is connected to target "<<*adj<<" we have ";
						std::cout<<"source is "<<( samples.contains( it.value() )?"not treated":"treated" )<<" and ";
						std::cout<<"target is not treated"<<std::endl;
					}
				}
				
			}
			/// display label image
			for ( int zz=max(0,z-1);zz<=min(z+1,(int)spimg.n_slices-1);zz++)
				std::cout<<"z = "<<zz<<"\t\t" ;
			std::cout<<std::endl;
			for ( int yy=max(0,y-1);yy<=min(y+1,(int)spimg.n_rows-1);yy++) {
				for ( int zz=max(0,z-1);zz<=min(z+1,(int)spimg.n_slices-1);zz++) {
					for ( int xx=max(0,x-1);xx<=min(x+1,(int)spimg.n_cols-1);xx++) {
						std::cout<<(int)((*labels)(yy,xx,zz) == theLabel )<<"|";
					}
					std::cout<<"\t\t" ;
				}
				std::cout<<std::endl;
			}
		}
	}
	return bFound ;
}

#endif

void init_toyproblem(graph_t ** pGraph) {
	(*pGraph) = new graph_t ;
	add_edge( 0,2, **pGraph ) ;
	add_edge( 1,1, **pGraph ) ;
	add_edge( 1,3, **pGraph ) ;
	add_edge( 1,4, **pGraph ) ;
	add_edge( 2,1, **pGraph ) ;
	add_edge( 2,3, **pGraph ) ;
	add_edge( 2,4, **pGraph ) ;
	add_edge( 3,1, **pGraph ) ;
	add_edge( 3,4, **pGraph ) ;
	add_edge( 4,0, **pGraph ) ;
	add_edge( 4,1, **pGraph ) ;
}
void solution_toy_problem( graph_t * pGraph, std::vector < int32_t > &dtime ) {
	for ( int inode=0;inode<num_vertices(*pGraph);inode++ ) {
		std::cout<<"Vertex "<<vertex( inode, *pGraph )<<" has distance "<<dtime[ inode ]<<std::endl;
	}
	std::cout<<std::endl;
}

//#define DEBUG_CC_EXTRACTION
//#define DEBUG_USE_V3D_CC_EXPORT

template <typename T, typename U>
std::string adjacent_voxels( const iCoord3D &v, const BillonTpl< T > & imT, const BillonTpl< U > & imU ) {
	std::ostringstream value ;
	register int x,y,z ;
	for ( z = std::max((int)0,v.z-1) ; z <= std::min( (int) imT.n_slices-1, v.z+1 ) ; z++ )
		value<<"z="<<z<<"\t\t\t" ;
	value << " for z, arround x,y="<<v.x<<","<<v.y<<"\n" ;
	int nc = 0 ;
	for ( y = std::max((int)0,v.y-1) ; y <= std::min( (int) imT.n_rows-1, v.y+1 ) ; y++ ) {
		for ( z = std::max((int)0,v.z-1) ; z <= std::min( (int) imT.n_slices-1, v.z+1 ) ; z++ ) {
			for ( x = std::max((int)0,v.x-1) ; x <= std::min( (int) imT.n_cols-1, v.x+1 ) ; x++ ) {
				value << setw(3)<<(int32_t)imT( y,x,z )<<"|"<<setw(3)<<(int32_t)imU( y,x,z )<<" " ;
				nc++ ;
			}
			value << "\t\t" ;
		}
		value << "\n" ;
	}
	value << nc << "\n" ;
	return value.str() ;
}

template <typename T>
std::string adjacent_voxels( const iCoord3D &v, const BillonTpl< T > & im ) {
	std::ostringstream value ;
	register int x,y,z ;
	for ( z = std::max((int)0,v.z-1) ; z <= std::min( (int) im.n_slices-1, v.z+1 ) ; z++ )
		value<<"z="<<z<<"\t\t\t" ;
	value << " for z, arround x,y="<<v.x<<","<<v.y<<"\n" ;
	int nc = 0 ;
	for ( y = std::max((int)0,v.y-1) ; y <= std::min( (int) im.n_rows-1, v.y+1 ) ; y++ ) {
		for ( z = std::max((int)0,v.z-1) ; z <= std::min( (int) im.n_slices-1, v.z+1 ) ; z++ ) {
			for ( x = std::max((int)0,v.x-1) ; x <= std::min( (int) im.n_cols-1, v.x+1 ) ; x++ ) {
				value << setw(3)<<(int32_t)im( y,x,z )<<" " ;
				nc++ ;
			}
			value << "\t\t" ;
		}
		value << "\n" ;
	}
	value << nc << "\n" ;
	return value.str() ;
}
template <typename T, typename U >
bool exist_mapping( const BillonTpl< T > &labels, const BillonTpl< U > &lblV3D, int x, int y,int z ) {
	register int zz, yy, xx ;
	bool is_mapping = true ;
	for ( zz=std::max((int)0,z-1);is_mapping&&zz<=std::min((int)labels.n_slices-1,z+1);zz++ )
		for ( yy=std::max((int)0,y-1);is_mapping&&yy<=std::min((int)labels.n_rows-1,y+1);yy++ )
			for ( xx=std::max((int)0,x-1);is_mapping&&xx<=std::min((int)labels.n_cols-1,x+1);xx++ )
				is_mapping = ( labels(yy,xx,zz) ==labels(y,x,z) ? lblV3D(yy,xx,zz)==lblV3D(y,x,z) : lblV3D(yy,xx,zz)==0 ) ;
	if ( is_mapping ) return is_mapping ;
	iCoord3D voi(x,y,z) ;
	assert( voi.x ==x && voi.y==y && voi.z==z);
	std::cerr<<"labels|lblV3D|img "<<adjacent_voxels( voi, labels, lblV3D ) ;
	return is_mapping ;
}

const int n_voi = 7 ;
iCoord3D dbg_voi[] = { iCoord3D(75,65,18), iCoord3D(130,32,44), iCoord3D(171,306,7), iCoord3D(198,371,92), iCoord3D(199,371,77), iCoord3D(230,57,10), iCoord3D(282,157,25) } ;

template <typename T> CImage * filter_imlabel( BillonTpl< T> &im, T selectedLabel ) {
	CImage *out = new CImage( im.n_rows, im.n_cols, im.n_slices ) ;
	out->fill(0) ;
	out->setMinValue(0);
	out->setMaxValue(1);
	register int x,y,z ;
	for ( z = 0 ; z < im.n_slices ; z++ )
		for ( x = 0 ; x < im.n_cols ; x++ )
			for ( y = 0 ; y < im.n_rows ; y++ )
				if ( im(y,x,z) == selectedLabel )
					(*out)(y,x,z) = 1 ;
	return out ;
}

std::pair< BillonTpl< CCExtractor::value_type > *,CCExtractor::value_type > own_extraction( const char *inputFileName, bool addboundary ) {
	CPgm3dFactory factory ;
	CImage *skel = factory.read( QString( "%1").arg(inputFileName) ) ;
	factory.correctEncoding( skel ) ;
	if ( addboundary ) {
		CImage *skelwithboundary =new CImage ( skel->n_rows+2,skel->n_cols+2,skel->n_slices+2) ;
		skelwithboundary->fill(0);
		register int x,y,z;
		for ( z=0;z<skel->n_slices;z++)
			for (x=0;x<skel->n_cols;x++)
				for ( y=0;y<skel->n_rows;y++ )
					(*skelwithboundary)(y+1,x+1,z+1)=(*skel)(y,x,z);
		skelwithboundary->setMinValue( skel->minValue() ) ;
		skelwithboundary->setMaxValue( skel->maxValue() ) ;
		delete skel ;
		skel = skelwithboundary ;
	}
	
	/// ensure we get a connected component
	CCExtractor extractor ;
	BillonTpl< CCExtractor::value_type > *labels = extractor.run( *skel ) ;
	const CCExtractor::TMapVolume &volumes = extractor.volumes() ;
	CCExtractor::TMapVolume::ConstIterator mainCCIter = volumes.constBegin() ;
	const CCExtractor::TMap3DBounds &bboxes = extractor.bounds3D();
	CCExtractor::TMap3DBounds::ConstIterator bboxIter = bboxes.constBegin();
	std::cout<<"Label : Volume ; x0 ; y0; z0 "<<std::endl;
	for ( CCExtractor::TMapVolume::ConstIterator lblIter = volumes.constBegin() ; lblIter != volumes.constEnd() ; lblIter++ ) {
		std::cout<<setw(6)<<(int)lblIter.key()<<":"<<setw(7)<<lblIter.value()<<";"<<setw(4)<<bboxIter.value().first.x<<";"<<setw(4)<<bboxIter.value().first.y<<";"<<setw(4)<<bboxIter.value().first.z<<std::endl;
		assert( lblIter.key() == bboxIter.key() ) ;
		if ( mainCCIter.value() < lblIter.value() )
			mainCCIter = lblIter ;
		bboxIter++ ;
	}
	delete skel ;
	return std::pair< BillonTpl< CCExtractor::value_type > *, CCExtractor::value_type >( labels, mainCCIter.key() ) ;
}

std::pair< ISPImage*, im_elem_sp_type> v3d_extraction( const char *inputFileName ) {
	IPgm3dFactory ifactory ;
	ISPImage * lblV3D = ifactory.read( QString("%1").arg(inputFileName) ) ;

	iCoord3D *lower = new iCoord3D[ lblV3D->maxValue()+1 ] ;
	iCoord3D *upper = new iCoord3D[ lblV3D->maxValue()+1 ] ;
	uint32_t *volume = new uint32_t [ lblV3D->maxValue()+1 ] ;
	uint32_t mainCCIndex = 1 ;
	uint32_t value ;
	for ( value = 1 ; value <= lblV3D->maxValue() ; value++ ) {
		lower[ value ] = iCoord3D( lblV3D->n_cols, lblV3D->n_rows, lblV3D->n_slices ) ;
		upper[ value ] = iCoord3D( 0, 0, 0 ) ;
		volume[ value ] = 0 ;
	}
	register int x,y,z ;
	for ( z=0;z<lblV3D->n_slices;z++)
		for (x=0;x<lblV3D->n_cols;x++)
			for ( y=0;y<lblV3D->n_rows;y++ ) {
				value = (*lblV3D)(y,x,z) ;
				if ( value == 0 ) continue ;
				lower[ value ].x = std::min( lower[ value ].x, x ) ;
				lower[ value ].y = std::min( lower[ value ].y, y ) ;
				lower[ value ].z = std::min( lower[ value ].z, z ) ;

				upper[ value ].x = std::max( upper[ value ].x, x ) ;
				upper[ value ].y = std::max( upper[ value ].y, y ) ;
				upper[ value ].z = std::max( upper[ value ].z, z ) ;

				volume[ value ] ++ ;
				if ( volume[value] > volume[mainCCIndex] ) mainCCIndex = value ;
			}
	/// print informations
	for ( value = 1 ; value <= lblV3D->maxValue() ; value++ ) {
		std::cout<<setw(6)<<(int)value<<":"<<setw(7)<<volume[value]<<";"<<setw(4)<<lower[value].x<<";"<<setw(4)<<lower[value].y<<";"<<setw(4)<<lower[value].z<<";"<<setw(4)<<upper[value].x<<";"<<setw(4)<<upper[value].y<<";"<<setw(4)<<upper[value].z<<std::endl;
	}
	delete [] lower ;
	delete [] volume ;
	return std::pair< ISPImage*, im_elem_sp_type > ( lblV3D, mainCCIndex ) ;
}

void antipodal_voxels_path( const graph_t &g, const std::vector < int32_t > &dtime, 
							const QMap< uint32_t, int32_t > &encodingVoxel, int n_cols, int n_rows, int n_slices, 
							QList<iCoord3D> &path ) {
	uint32_t farestVoxel = 1 ;
	for ( uint32_t idx = 0 ; idx != dtime.size() ; idx++  )
		if ( dtime[ idx ] > dtime[ farestVoxel ] )
			farestVoxel = idx ;
	path.append( from_linear_coord( encodingVoxel.keys(farestVoxel)[0], n_cols, n_rows, n_slices ) ) ;
	
	graph_t::adjacency_iterator adj,adj_end ;
	/// from this voxel, backpropagation
	while ( dtime[ farestVoxel ] != 1 ) {
		boost::tie( adj,adj_end ) = adjacent_vertices( vertex( farestVoxel, g ), g ) ;
		for ( ; adj != adj_end ; adj++ ) {
			if ( dtime[ *adj ] < dtime[ farestVoxel ] ) {
				path.append( from_linear_coord( encodingVoxel.keys(*adj)[0], n_cols, n_rows, n_slices ) ) ;
				farestVoxel = *adj;
				break ;
			}
		}
		assert ( adj != adj_end ) ;
	}
}
template <typename T>
bool is_junction( const graph_t &g, const QMap< uint32_t, int32_t > &encodingVoxel, const iCoord3D &pt, const BillonTpl<T> &img ) {
	uint32_t value = linear_coord(pt.x,pt.y,pt.z,img.n_cols,img.n_rows,img.n_slices) ;
	int32_t node = encodingVoxel[ value ] ;
	
	if ( degree( node, g ) <= 2 ) return false ;
	
	graph_t::adjacency_iterator adj_begin, adj_na,adj_nb,adj_end,
								adj_nc_begin, adj_nc_end,
								adj_nd_begin, adj_nd_end,
								nb,nc,nd,
								adj_nb_begin, adj_nb_end, n ;
	std::set<int32_t> adjB, adjC,share ;
	boost::tie(adj_begin,adj_end ) = adjacent_vertices( vertex( node, g ), g ) ;
	for ( nb = adj_begin ; nb != adj_end ; nb++ ) {
		nc = nb ;
		nc++ ;
		boost::tie(adj_nb_begin,adj_nb_end ) = adjacent_vertices( vertex( *nb, g ), g ) ;
		for ( n = adj_nb_begin;n!=adj_nb_end;n++ )
			adjB.insert( *n ) ;
		adjB.erase( node ) ;
		adjC.insert( *nb ) ;
		for ( ; nc != adj_end ; nc++ ) {
			boost::tie(adj_nc_begin,adj_nc_end ) = adjacent_vertices( vertex( *nc, g ), g ) ;
			for ( n = adj_nc_begin;n!=adj_nc_end;n++ )
				adjC.insert( *n ) ;
			adjC.erase( node ) ;
			adjC.insert( *nc ) ;
			set_intersection( adjB.begin(), adjB.end(), adjC.begin(), adjC.end(), inserter( share, share.begin() )) ;
			if ( share.empty() ) {
				nd = nc ;
				nd++ ;
				if ( nd != adj_end ) {
					for ( ; nd != adj_end ; nd++ ) {
						if ( adjB.find( *nd ) == adjB.end() && adjC.find( *nd ) == adjC.end() ) break ;
					}
					if ( nd != adj_end ) {
						BillonTpl<char> junction(3,3,3) ;
						junction.fill(0);
						junction(1,1,1) = 1 ;
						for ( n = adj_begin ; n != adj_end ; n++ ) {
							uint32_t ea = encodingVoxel.keys( *n )[ 0 ] ;
							iCoord3D pta = from_linear_coord( ea, img.n_cols, img.n_rows, img.n_slices ) ;
							junction( pta.y - pt.y +1 , pta.x-pt.x + 1, pta.z-pt.z+1) = 1 ;
						}
						std::cerr << adjacent_voxels( iCoord3D(1,1,1), junction ) << std::endl ;
						return true ;
					}
				}
			}
			share.clear() ;
			adjC.clear() ;
		}
		adjB.clear() ;
	}
	

	return false ;
}

int main( int narg, char **argv ) {
	graph_t 				*pGraph =0;
	uint32_t 				source = 0 ;
	QMap<uint32_t,int32_t> 	encoding_vertices ;

#ifdef CHECK_TOY_PROBLEM_OUTPUT
	init_toyproblem(&pGraph) ;
#else
	const char *inputFileName = argv[1] ;
	const char *outputFileName = argv[2] ;
	const char *depthFileName = argv[3] ;
	
	BillonTpl< CCExtractor::value_type > 	*pOwnLabel=0;
	CCExtractor::value_type 				idOwn ;
	ISPImage								*pV3DLabel=0;
	im_elem_sp_type							idV3D ;
	
#ifdef DEBUG_CC_EXTRACTION
	{
		boost::tie(pOwnLabel,idOwn) = own_extraction( inputFileName, true ) ;
		boost::tie(pV3DLabel,idV3D) = v3d_extraction( "/tmp/labelWithV3D.pgm3d" ) ;
		std::cerr<<"Comparison between anthouse.labels ("<<pOwnLabel->n_rows<<" x "<<pOwnLabel->n_cols<<" x "<<pOwnLabel->n_slices<<") and v3d.labels ("
														<<pV3DLabel->n_rows<<" x "<<pV3DLabel->n_cols<<" x "<<pV3DLabel->n_slices<<")"<<std::endl;
		register int z, y, x ;
		for ( z=0;z<pOwnLabel->n_slices;z++)
			for (x=0;x<pOwnLabel->n_cols;x++)
				for ( y=0;y<pOwnLabel->n_rows;y++ ) {
					if ( (*pOwnLabel)(y,x,z) )
						if ( !exist_mapping( *pOwnLabel, *pV3DLabel,x,y,z ) ) {
							std::cerr<<"anthouse.cc "<<(*pOwnLabel)(y,x,z)<<" vs v3d.cc "<<(*pV3DLabel)(y,x,z)<<std::endl;
						}
				}
		std::cerr<<"== debug == "<<__FILE__<<" @ line "<<__LINE__<<std::endl;
		for ( int i_voi = 0 ; i_voi < n_voi ; i_voi++ )
			std::cerr<<adjacent_voxels( dbg_voi[ i_voi ], *pOwnLabel ) ;
		
		delete pOwnLabel ;
		delete pV3DLabel ;
		exit(8);
	}
#else
	CImage *img = 0;
#ifdef DEBUG_USE_V3D_CC_EXPORT
	boost::tie(pV3DLabel,idV3D) = v3d_extraction( "/tmp/labelWithV3D.pgm3d" ) ;
	std::cerr<<"[V3D] labels "<<pV3DLabel->n_rows<<" x "<<pV3DLabel->n_cols<<" x "<<pV3DLabel->n_slices<<" validity "<< !have_touching_conn_comp( *pV3DLabel ) <<std::endl;
	img = filter_imlabel( *pV3DLabel,idV3D ) ;
	std::cerr<<"[V3D] Select component "<<idV3D<<std::endl;
	delete pV3DLabel ;
#else
	boost::tie(pOwnLabel,idOwn) = own_extraction( inputFileName, false ) ;
	std::cerr<<"[OWN] labels "<<pOwnLabel->n_rows<<" x "<<pOwnLabel->n_cols<<" x "<<pOwnLabel->n_slices<<" validity "<< !have_touching_conn_comp( *pOwnLabel )<<std::endl;
	img = filter_imlabel( *pOwnLabel,idOwn ) ;
	std::cerr<<"[OWN] Select component "<<idOwn<<std::endl;
	delete pOwnLabel ;
#endif
	/// initializing the graph
	pGraph = imageToGraph( *img, encoding_vertices, (char)1 );

	ISPImage spimg( img->n_rows, img->n_cols, img->n_slices ) ;
	spimg.fill(0) ;

	std::cout<<encoding_vertices.begin().key()<<" "<<encoding_vertices.begin().value()<<std::endl
			 <<y_from_linear_coord(encoding_vertices.begin().key(),spimg.n_cols, spimg.n_rows, spimg.n_slices)<<","<<x_from_linear_coord(encoding_vertices.begin().key(),spimg.n_cols, spimg.n_rows, spimg.n_slices)<<","<<z_from_linear_coord(encoding_vertices.begin().key(),spimg.n_cols, spimg.n_rows, spimg.n_slices)<<std::endl;
	std::cout<<num_vertices(*pGraph)<<" vertices / "<<encoding_vertices.size()<<" encoded voxels"<<std::endl;

	delete img ;
#endif
#endif
	/// computing shortest path from source voxel
	std::vector < int32_t > dtime(num_vertices(*pGraph));
	int32_t time = 1;
	bfs_time_visitor < int32_t * >vis(&dtime[0], time);
	breadth_first_search(*pGraph, vertex( source, *pGraph), visitor(vis));
  
#ifdef CHECK_TOY_PROBLEM_OUTPUT
	solution_toy_problem(pGraph,dtime) ;
#else
#ifndef DEBUG_CC_EXTRACTION
	/// exporting result
	QFile file("/tmp/minpath.log") ;
	if( !file.open(QFile::WriteOnly) ) {
		std::cerr << "Error : can not write log file"<< std::endl;
		return -1;
	}
	QTextStream out(&file) ;
	out<<"Command line : "<<argv[0]<<" "<<argv[1]<<" "<<argv[2]<<endl;
	QMap< uint32_t,int32_t> missing ;
	for ( QMap<uint32_t,int32_t>::iterator it = encoding_vertices.begin() ; it != encoding_vertices.end() ; it++ ) {
		spimg( y_from_linear_coord( it.key(),spimg.n_cols, spimg.n_rows, spimg.n_slices ) , x_from_linear_coord( it.key(),spimg.n_cols, spimg.n_rows, spimg.n_slices ), z_from_linear_coord( it.key(),spimg.n_cols, spimg.n_rows, spimg.n_slices ) ) = dtime[ it.value() ] ;
		out<<y_from_linear_coord( it.key(),spimg.n_cols, spimg.n_rows, spimg.n_slices )<<" , "<<x_from_linear_coord( it.key(),spimg.n_cols, spimg.n_rows, spimg.n_slices )<<" , "<<z_from_linear_coord( it.key(),spimg.n_cols, spimg.n_rows, spimg.n_slices )<<" : "<<dtime[ it.value() ]<<endl;
		if ( dtime[ it.value() ] == 0 ) {
			missing[ it.key() ] = it.value() ;
			spimg( y_from_linear_coord( it.key(),spimg.n_cols, spimg.n_rows, spimg.n_slices ) , x_from_linear_coord( it.key(),spimg.n_cols, spimg.n_rows, spimg.n_slices ), z_from_linear_coord( it.key(),spimg.n_cols, spimg.n_rows, spimg.n_slices ) ) = time+1 ;
 		}
	}
	QList<iCoord3D> path ;
	antipodal_voxels_path( *pGraph, dtime, encoding_vertices, spimg.n_cols, spimg.n_rows, spimg.n_slices, path ) ;
 	spimg.setMinValue( 0 ) ;
 	spimg.setMaxValue( time+1 ) ;
	IOPgm3d< im_elem_sp_type, qint32,false>::write( spimg, QString("%1").arg( outputFileName ) ) ;
	spimg.fill( 0 ) ;
	for ( int step = 0 ; step < path.size() ; step++ )
		spimg( path.at(step).y,path.at(step).x,path.at(step).z) =1;//time-step;
 	spimg.setMinValue( 0 ) ;
 	spimg.setMaxValue( 1/*time+1*/ ) ;

	IOPgm3d< im_elem_sp_type, qint32,false>::write( spimg, QString("/tmp/geodesic.pgm3d") ) ;

	std::cout<<"Number of elements being zero : "<<missing.size()<<std::endl;
	{
		IPgm3dFactory ifactory ;
		ISPImage *depthmap = ifactory.read( QString("%1").arg(depthFileName) ) ;
		ifactory.correctEncoding( depthmap ) ;
		spimg.fill(0) ;
		int nComp = 1 ;
		out <<"thickness along the longest path (geodesic)"<<endl;
		for ( int step = 0 ; step < path.size() ; step++ ) {
			bool junction = is_junction( *pGraph, encoding_vertices, path.at(step), *depthmap ) ;
			if ( junction ) nComp++ ;
			out << (*depthmap)( path.at(step).y,path.at(step).x,path.at(step).z) << " "<<junction <<  endl ;
			spimg( path.at(step).y,path.at(step).x,path.at(step).z) = nComp ;
		}
		spimg.setMaxValue( nComp+1 ) ;
		IOPgm3d< im_elem_sp_type, qint32,false>::write( spimg, QString("/tmp/geodesic.comp.pgm3d") ) ;
		delete depthmap ;
	}
	file.close();
#ifdef EXPENSIVE_TESTING
	if ( !missing.empty()) {
		connectivity_map( missing, encoding_vertices, spimg, labels,  mainCCIter.key(), pGraph, "missing") ;
	}
	connectivity_map( encoding_vertices, encoding_vertices, spimg, labels,  mainCCIter.key(), pGraph, "all") ;
	
	avoid_adjcacent( missing, encoding_vertices, pGraph ) ;
	reverse_avoid_adjcacent( missing, encoding_vertices, pGraph ) ;
#endif
#endif
#endif
	delete pGraph ;
	
	return 0 ;
}
