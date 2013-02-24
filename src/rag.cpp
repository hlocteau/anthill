#include <rag.hpp>

NodeData::NodeData( ) {
	_id = -1 ;
	_category = -1 ;
	_volume = 0 ;
	std::cout<<"Default constructor Node "<<(int)_id<<std::endl;
}
NodeData::~NodeData() {
	std::cout<<"Destructor Node "<<_id<<std::endl;
}
NodeData::NodeData( const NodeData & o ) {
	_id = o._id;
	_volume = o._volume ;
	_category = o._category ;
	std::cout<<"Copy constructor Node "<<_id<<std::endl;
}
NodeData & NodeData::operator = ( const NodeData &o ) {
	if ( this != &o ) {
		_id = o._id ;
		_volume = o._volume ;
		_category = o._category ;
		std::cout<<"copy operator Node "<<_id<<std::endl;
	}
	return *this ;
}

NodeData & NodeData::operator += ( const NodeData & other ) {
	_volume += other._volume ;
	return *this ;
}

EdgeData::EdgeData( ) {
	_source_id = -1 ;
	_target_id = -1 ;
	#ifdef STORAGE_VOXEL
	QList<DGtal::Z3i::Point> *null_ptr  = 0 ;
	_per_side[ 0 ] = boost::shared_ptr< QList<DGtal::Z3i::Point>  >(null_ptr) ;
	_per_side[ 1 ] = boost::shared_ptr< QList<DGtal::Z3i::Point>  >(null_ptr) ;
	#endif
	//std::cout<<"Default constructor Edge"<<std::endl;
}
EdgeData::~EdgeData() {
	//std::cout<<"before reset "<<_source_id<<" "<<_target_id<<"  adr "<<_per_side[0]<<std::endl;
	reset() ;
	//std::cout<<"Destructor Edge ";
	//std::cout<<std::endl;
}
EdgeData::EdgeData( const EdgeData & o ) {
	_source_id = o._source_id;
	_target_id = o._target_id ;
	#ifdef STORAGE_VOXEL
	_per_side[ 0 ] = o._per_side[ 0 ] ;
	_per_side[ 1 ] = o._per_side[ 1 ] ;
	//std::cout<<"Copy constructor Edge ("<<_source_id<<","<<_target_id <<")";
	//std::cout<<std::endl;
	#endif
}

EdgeData & EdgeData::operator = ( const EdgeData &o ) {
	if ( this != &o ) {
		reset();
		_source_id = o._source_id;
		_target_id = o._target_id ;
		#ifdef STORAGE_VOXEL
		_per_side[ 0 ] = o._per_side[ 0 ] ;
		_per_side[ 1 ] = o._per_side[ 1 ] ;
		//std::cout<<"copy operator Edge " ;
		std::cout<<std::endl;
		#endif
	}
	return *this ;
}

void EdgeData::reset( ) {
	_source_id = -1 ;
	_target_id = -1 ;
	#ifdef STORAGE_VOXEL
	//std::cout<<"line "<<__LINE__<<std::endl;
	_per_side[0].reset() ;
	_per_side[1].reset() ;
	//std::cout<<"line "<<__LINE__<<std::endl;
	#endif
}
#ifdef STORAGE_VOXEL
void EdgeData::append( uint id, DGtal::Z3i::Point v ) {
	boost::shared_ptr< QList< DGtal::Z3i::Point > > L = _per_side[ ( id == _source_id ? 0 : 1 ) ] ;
	uint size = L->size() ;
	uint position = 0 ;
	while ( position < size ) {
		if ( v <= L->at( position ) ) break ;
		position++ ;
	}
	if ( position != size ) {
		if ( v != L->at( position ) ) L->insert( position, v ) ;
	} else {
		L->append( v ) ;
	}
}
#endif

EdgeData & EdgeData::operator += ( const EdgeData & other ) {
	uint tr[] = { 0, 1 } ;
	if ( _source_id == other._source_id && _target_id == other._target_id ) {
	} else if ( _source_id == other._target_id && _target_id == other._source_id ) {
		tr[0] = 1 ;
		tr[1] = 0 ;
	} else {
		assert( ( _source_id == other._source_id && _target_id == other._target_id ) ||
		        ( _source_id == other._target_id && _target_id == other._source_id ) ) ;
	}
	
	for ( uint s = 0 ; s < 2 ; s++ ) {
		uint pos_this = 0,
		     pos_other = 0,
		     size_other = other._per_side[ tr[s] ]->size() ;
		while ( pos_other != size_other && pos_this != _per_side[ s ]->size() ) {
			if ( _per_side[ s ]->at( pos_this ) == other._per_side[ tr[s] ]->at( pos_other ) ) {
				pos_this++ ;
				pos_other++ ;
			} else if ( _per_side[ s ]->at( pos_this ) > other._per_side[ tr[s] ]->at( pos_other ) ) {
				_per_side[ s ]->insert( pos_this, other._per_side[ tr[s] ]->at( pos_other ) ) ;
				pos_other++ ;
			} else {
				pos_this++ ;
			}
		}
		while ( pos_other != size_other ) {
			_per_side[ s ]->append( other._per_side[ tr[s] ]->at( pos_other ) ) ;
			pos_other++ ;
		}
	}
	
	return *this ;
}

void merge_nodes( GraphAdj::vertex_descriptor growing, GraphAdj::vertex_descriptor other, GraphAdj & g ) {
	std::cout<<__FUNCTION__<<" vertex "<<other<<" attached to "<<growing<<std::endl;
	boost::property_map< GraphAdj, _NodeTag >::type node_map  = boost::get( _NodeTag(), g ) ;
	boost::property_map< GraphAdj, _EdgeTag >::type edge_map  = boost::get( _EdgeTag(), g ) ;
	
	NodeData & nd = node_map[ growing ] ;
			
	GraphAdj::in_edge_iterator in_edge, in_edge_end ;
	GraphAdj::edge_descriptor e ;
	GraphAdj::vertex_descriptor opposite ;
	bool existing,creation ;
	boost::tie( in_edge, in_edge_end ) = boost::in_edges(other, g) ;
	for ( ; in_edge != in_edge_end ; in_edge++ ) {
		std::cout<<"\tinfo edge "<<boost::source( *in_edge, g )<<" "<<boost::target( *in_edge, g )<<" in loop"<<std::endl;
		assert( boost::target( *in_edge, g ) == other ) ;
		opposite = boost::source( *in_edge, g ) ;
		EdgeData & ed = edge_map[ *in_edge ] ;
		
		if ( opposite == growing ) {
			std::cout<<"\t\tinfo edge link specific pair of vertices"<<std::endl;
			ed.reset() ;
			continue ;
		}
				
		/// case 1 : there is no edge (growing,opposite)
		/// case 2 : there is already an edge (growing,opposite)
		
		boost::tie( e, existing ) = boost::edge( opposite, growing, g ) ;
		if ( !existing ) {
			std::cout<<"\t\tinfo edge has to be created"<<std::endl;
			boost::tie( e, creation ) = boost::add_edge( opposite, growing, g ) ;
			assert( boost::source(e,g) == opposite ) ;
			assert( boost::target(e,g) == growing ) ;
			if ( ed.target_id() == other )
				ed.setTarget( growing ) ;
			else
				ed.setSource( growing ) ;
			edge_map[ e ] = ed ;
		} else {
			std::cout<<"\t\tinfo edge has to be updated"<<std::endl;
			assert( boost::source(e,g) == opposite ) ;
			assert( boost::target(e,g) == growing ) ;
			std::cerr<<"\t\t\tinfo : ed "<<ed.source_id()<<" - "<<ed.target_id()<<std::endl;
			EdgeData & edg = edge_map[ e ] ;
			std::cerr<<"\t\t\tinfo : edg "<<edg.source_id()<<" - "<<edg.target_id()<<std::endl;
			if ( ed.target_id() == other ) ed.setTarget( growing ) ;
			if ( ed.source_id() == other ) ed.setSource( growing ) ;
			std::cerr<<"\t\t\tinfo : ed "<<ed.source_id()<<" - "<<ed.target_id()<<std::endl;
			edg += ed ;
		}
		ed.reset() ;
	}
	NodeData & ndo = node_map[ other ] ;
	nd += ndo ;
	
	boost::tie( e, existing ) = boost::edge( other, growing, g ) ;
	assert( existing ) ;
	boost::remove_edge( e, g ) ;
	
	boost::clear_vertex( other, g ) ;
	boost::remove_vertex( other, g ) ;
}
