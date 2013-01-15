#include <io/Pgm3dFactory.h>
#include <connexcomponentextractor.h>
#include <boost/graph/breadth_first_search.hpp>
#include <SkeletonGraph.hpp>
#include <boost/range/irange.hpp>
#include <DGtal/helpers/StdDefs.h>

//#define CHECK_TOY_PROBLEM_OUTPUT


using DGtal::Z3i::Point ;

typedef Pgm3dFactory<char> 				CPgm3dFactory ;
typedef BillonTpl<char>					CImage ;
typedef SkeletonGraph<char>				CSkeletonGraph ;
typedef Pgm3dFactory<int32_t> 			IPgm3dFactory ;
typedef int32_t							im_elem_sp_type ;
typedef BillonTpl< im_elem_sp_type > 	ISPImage ;
typedef ConnexComponentExtractor<CPgm3dFactory::value_type,int16_t> CCExtractor ;



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

#if 0
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
#endif

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

const int n_voi = 0 ;
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

template <typename T>
void antipodal_voxels_path( const SkeletonGraph<T> &SG, const std::vector < int32_t > &dtime, QList<iCoord3D> &path ) {
	uint32_t farestVoxel = 1 ;
	for ( uint32_t idx = 0 ; idx != dtime.size() ; idx++  )
		if ( dtime[ idx ] > dtime[ farestVoxel ] )
			farestVoxel = idx ;
	path.append( SG[ farestVoxel ] ) ;
	
	typename SkeletonGraph<T>::graph_t::adjacency_iterator adj,adj_end ;
	const typename SkeletonGraph<T>::graph_t g = SG.graph() ;
	/// from this voxel, backpropagation
	while ( dtime[ farestVoxel ] != 1 ) {
		boost::tie( adj,adj_end ) = adjacent_vertices( vertex( farestVoxel, g ), g ) ;
		for ( ; adj != adj_end ; adj++ ) {
			if ( dtime[ *adj ] < dtime[ farestVoxel ] ) {
				path.append( SG[*adj] ) ;
				farestVoxel = *adj;
				break ;
			}
		}
		assert ( adj != adj_end ) ;
	}
}
template <typename T> bool is_junction( const SkeletonGraph<T> & SG, const iCoord3D &pt ) {
	int32_t node = SG.node( pt ) ;
	const typename SkeletonGraph<T>::graph_t & g = SG.graph() ;
	if ( degree( node, g ) <= 2 ) return false ;
	
	typename SkeletonGraph<T>::graph_t::adjacency_iterator adj_begin, adj_na,adj_nb,adj_end,
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
							iCoord3D pta = SG[ *n ] ;
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

#ifdef CHECK_TOY_PROBLEM_OUTPUT
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

int main( int narg, char **argv ) {
	graph_t 				*pGraph =0;
	init_toyproblem(&pGraph) ;
	
	/// computing shortest path from source voxel
	std::vector < int32_t > dtime(num_vertices(*pGraph));
	int32_t time = 1;
	bfs_time_visitor < int32_t * >vis(&dtime[0], time);
	breadth_first_search(*pGraph, vertex( source, *pGraph), visitor(vis));
	solution_toy_problem(pGraph,dtime) ;
	
	delete pGraph ;
	return 0 ;
}
#else

template <typename T> seg_geodesic( const SkeletonGraph<T> &SG, int32_t source, const QString &serie ) {
	std::vector < int32_t > dtime(SG.number_of_vertices());
	int32_t time = 1;
	bfs_time_visitor < int32_t * >vis(&dtime[0], time);
	breadth_first_search( g, vertex( source, g), visitor(vis));

	if ( save_travels ) {
		spimg.fill( 0 ) ;
		for ( SkeletonGraph<T>::ConstVoxelIterator it = SG.encoding_begin() ; it != SG.encoding_end() ; it++ ) {
			if ( dtime[ it.value() ] == 0 ) continue ;
			iCoord3D pt = SG.from_linear_coord( it.key() ) ;
			spimg( pt.y , pt.x, pt.z ) = dtime[ it.value() ] ;
		}
		spimg.setMinValue( 0 ) ;
		spimg.setMaxValue( time+1 ) ;
		IOPgm3d< im_elem_sp_type, qint32,false>::write( spimg, QString("%1").arg( outputFileName ) ) ;
	}
	QList<iCoord3D> path ;
	antipodal_voxels_path( SG, dtime, path ) ;
	if ( save_longest_travel ) {
		spimg.fill( 0 ) ;
		for ( int step = 0 ; step < path.size() ; step++ )
			spimg( path.at(step).y,path.at(step).x,path.at(step).z) =1;
		spimg.setMinValue( 0 ) ;
		spimg.setMaxValue( 1 ) ;
		IOPgm3d< im_elem_sp_type, qint32,false>::write( spimg, QString("/tmp/geodesic.pgm3d") ) ;
	}

	{
		IPgm3dFactory ifactory ;
		ISPImage *depthmap = ifactory.read( QString("%1").arg(depthFileName) ) ;
		ifactory.correctEncoding( depthmap ) ;
		spimg.fill(0) ;
		int nComp = 1 ;
		out <<"thickness along the longest path (geodesic)"<<endl;
		for ( int step = 0 ; step < path.size() ; step++ ) {
			bool junction = is_junction( SG,path.at(step) ) ;
			if ( junction ) nComp++ ;
			out << (*depthmap)( path.at(step).y,path.at(step).x,path.at(step).z) << " "<<junction <<  endl ;
			spimg( path.at(step).y,path.at(step).x,path.at(step).z) = nComp ;
		}
		spimg.setMaxValue( nComp+1 ) ;
		IOPgm3d< im_elem_sp_type, qint32,false>::write( spimg, QString("/tmp/geodesic.comp.pgm3d") ) ;
		delete depthmap ;
	}
	/// have to remove all edges between the distinct adjacent connected components
}

int main( int narg, char **argv ) {
	const char *inputFileName = argv[1] ;
	const char *outputFileName = argv[2] ;
	const char *depthFileName = argv[3] ;
	
	BillonTpl< CCExtractor::value_type > 	*pOwnLabel=0;
	CCExtractor::value_type 				idOwn ;
	boost::tie(pOwnLabel,idOwn) = own_extraction( inputFileName, true ) ;
	CImage *img = filter_imlabel( *pOwnLabel,idOwn ) ;
	delete pOwnLabel ;

	/// initializing the graph
	CSkeletonGraph SG( *img, (char)1 );
	const CSkeletonGraph::graph_t & g = SG.graph() ; 


	ISPImage spimg( img->n_rows, img->n_cols, img->n_slices ) ;
	spimg.fill(0) ;

	/// aplly source selection...
	uint32_t 				source = 0 ;
	std::cout<<SG.encoding_begin().key()<<" "<<SG.encoding_begin().value()<<" = "<<source<<std::endl
			 <<SG[ source ].y<<","<<SG[ source ].x<<","<<SG[ source ].z<<std::endl;
	std::cout<<SG.number_of_vertices()<<std::endl;

	delete img ;

	/// computing shortest path from source voxel

  
	/// exporting result
	QFile file("/tmp/minpath.log") ;
	if( !file.open(QFile::WriteOnly) ) {
		std::cerr << "Error : can not write log file"<< std::endl;
		return -1;
	}
	QTextStream out(&file) ;
	out<<"Command line : "<<argv[0]<<" "<<argv[1]<<" "<<argv[2]<<endl;
	file.close();
#ifdef EXPENSIVE_TESTING
	if ( !missing.empty()) {
		connectivity_map( missing, encoding_vertices, spimg, labels,  mainCCIter.key(), pGraph, "missing") ;
	}
	connectivity_map( encoding_vertices, encoding_vertices, spimg, labels,  mainCCIter.key(), pGraph, "all") ;
	
	avoid_adjcacent( missing, encoding_vertices, pGraph ) ;
	reverse_avoid_adjcacent( missing, encoding_vertices, pGraph ) ;
#endif
	
	return 0 ;
}
#endif
