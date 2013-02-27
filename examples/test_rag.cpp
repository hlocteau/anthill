#include <rag.hpp>
#include <io/IOPgm3d.h>

template <typename T > void redraw( BillonTpl<T> &img, T old, T cur ) {
	BillonTpl< arma::u8>::iterator  iterWriteEnd = img.end(),
	                                iterWrite =img.begin() ;
	for ( ; iterWrite != iterWriteEnd ; iterWrite++ )
		if ( *iterWrite == old )
			*iterWrite = cur ;

}

template <typename T > void draw( BillonTpl<T> &img, GraphAdj &g ) {
	boost::property_map< GraphAdj, _NodeTag >::type node_map  = boost::get( _NodeTag(), g ) ;
	boost::property_map< GraphAdj, _EdgeTag >::type edge_map  = boost::get( _EdgeTag(), g ) ;

	GraphAdj::edge_iterator eb,ee ;
	boost::tie( eb,ee ) = boost::edges( g ) ;
	
	arma::u8 number_of_cc = boost::num_vertices(g) ;
	
	for ( ; eb != ee ; eb++ ) {
		EdgeData &data = edge_map[ *eb ] ;
		
		boost::shared_ptr< QList< DGtal::Z3i::Point > > L = data.per_side( data.source_id() ) ;
		number_of_cc++ ;
		for ( uint v = 0 ; v < L->size() ; v++ )
			(img)( L->at(v).at(1),
			       L->at(v).at(0),
			       L->at(v).at(2) ) = number_of_cc ;
		number_of_cc++ ;
		L = data.per_side( data.target_id() ) ;
		for ( uint v = 0 ; v < L->size() ; v++ )
			(img)( L->at(v).at(1),
			       L->at(v).at(0),
			       L->at(v).at(2) ) = number_of_cc ;		
	}
}
template <typename T> void
reset_img( BillonTpl<T> *img ) {
	arma::Cube<T> t = (*( (arma::Cube<arma::u8>*)img ) ) ;
	t ( arma::span(3,7), arma::span(4,8), arma::span(10,30) ).fill( (T)1 );
	t ( arma::span(6,22), arma::span(7,20), arma::span(25,50) ).fill( (T)2 );
	t ( arma::span(15,30), arma::span(6,30), arma::span(45,55) ).fill( (T)3 );
	t ( arma::span(8,50), arma::span(25,35), arma::span(40,50) ).fill( (T)4 );
	t ( arma::span(20,40), arma::span(20,40), arma::span(35,45) ).fill( (T)5 );
	typename BillonTpl< T >::iterator iterRead = t.begin(),
	                                  iterReadEnd = t.end(),
	                                  iterWrite =img->begin() ;
	for ( ; iterRead != iterReadEnd ; iterRead++, iterWrite++ )
		*iterWrite = * iterRead ;
}


void print( GraphAdj & g ) {
	boost::property_map< GraphAdj, _NodeTag >::type node_map  = boost::get( _NodeTag(), g ) ;
	boost::property_map< GraphAdj, _EdgeTag >::type edge_map  = boost::get( _EdgeTag(), g ) ;
	GraphAdj::edge_iterator eb,ee ;
	boost::tie( eb,ee ) = boost::edges( g ) ;
	for ( ; eb != ee ; eb++ ) {
		EdgeData &data = edge_map[ *eb ] ;
		std::cout<< data.source_id()<< " "<< data.target_id()<<" "
		         <<boost::source(*eb,g )<<" "<<boost::target(*eb,g )<<"    "
		         <<data.per_side( data.source_id() )->size()<<" "<< data.per_side( data.target_id() )->size()<< std::endl;
	}

	GraphAdj::vertex_iterator vb,ve ;
	boost::tie( vb,ve ) = boost::vertices( g ) ;
	for ( ; vb != ve ; vb++ ) {
		NodeData & data = node_map[ *vb ] ;
		if ( data.id() != -1 )
			std::cout<< "vertex descriptor "<<*vb<<"  property "<<data.id()<< " "<< data.volume()<<std::endl;
	}
}

int main( int narg, char **argv ) {

	BillonTpl< arma::u8> *img = new BillonTpl< arma::u8>(60,60,60) ;
	img->fill(0);
	
	reset_img( img ) ;
	GraphAdj *g = init_rag( *img, (arma::u8)1 ) ;
	
	IOPgm3d< arma::u8, qint8, false >::write( *img, "test_rag1.pgm3d" ) ;

	draw<arma::u8>( *img, *g ) ;
	print( *g ) ;
	IOPgm3d< arma::u8, qint8, false >::write( *img, "test_rag2.pgm3d" ) ;
	
	merge_nodes( atoi( argv[1]),atoi( argv[2]), *g ) ;
	reset_img( img ) ;
	redraw( *img, (arma::u8)atoi( argv[2]),(arma::u8)atoi( argv[1]) ) ;
	print( *g ) ;
	draw<arma::u8>( *img, *g ) ;
	IOPgm3d< arma::u8, qint8, false >::write( *img, "test_rag3.pgm3d" ) ;
	
	delete img ;
	delete g ;

	return 0 ;
}
