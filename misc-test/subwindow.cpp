#include <io/Pgm3dFactory.h>

int main( int narg, char **argv ) {
#if 0
	assert( narg >= 7 /*&& narg <=8*/ ) ;
#endif
	Billon * skel = io::Pgm3dFactory::read( QString("%1").arg( argv[1] ) ) ;
	assert( skel != 0 ) ;
	//std::cout<<"image's dimension is "<<skel->n_cols<<" x "<<skel->n_rows<<" x "<<skel->n_slices<<"   ("<<narg<<" arguments)"<<std::endl;


#if 0
	bool bToLittleEndian = false ;
	if ( narg == 8 )
		bToLittleEndian = ( strcmp( argv[7], "l" ) == 0 ) ;

	if ( bToLittleEndian ) {
		std::cout<<"Manage representation of number"<<std::endl;
		#if 0
		for ( uint z=0;z<skel->n_slices;z++)
			for ( uint y=0;y<skel->n_rows;y++)
				for ( uint x=0;x<skel->n_cols;x++) {
					uint32_t value = (uint32_t) ( (*skel).at(y,x,z) );
					(*skel).at(y,x,z) = qFromBigEndian< qint32 >( (const uchar*)( &value ) ) ;
				}
		#else		
		*skel *= -1 ;
		#endif
	}
#endif


	//std::cout<<"display for y in "<<ymin<<":"<<ymax<<" and x in "<<xmin<<":"<<xmax<<std::endl;

	std::map< uint32_t, uint32_t > histogram ;
	for ( uint y=0;y<skel->n_rows;y++) {
		for ( uint x=0;x<skel->n_cols;x++) {
			uint32_t value = ( (*skel).at(y,x,0) );
			if ( histogram.find( value ) == histogram.end() ) histogram[ value ] = 0 ;
			histogram[ value ] ++ ;
		}
	}
	std::cout<<"---"<<std::endl;
	for ( std::map< uint32_t, uint32_t >::const_iterator it = histogram.begin() ; it != histogram.end() ; it++ )
		std::cout<<it->first<<" : "<<it->second<<std::endl;
	std::cout<<"---"<<std::endl;

#if 0
	uint xmin = atoi( argv[2] ) ;
	uint ymin = atoi( argv[3] ) ;
	uint xmax = atoi( argv[4] ) ;
	uint ymax = atoi( argv[5] ) ;
	uint spacing = atoi( argv[ 6 ] ) ;
	
	assert( skel->n_slices == 1 ) ;
	assert( skel->n_rows >= ymax ) ;
	assert( skel->n_cols >= xmax ) ;

	for ( uint y=ymin;y<ymax;y++) {
		for ( uint x=xmin;x<xmax;x++) {
			uint32_t value = ( (*skel).at(y,x,0) );
			std::cout<<setw(spacing)<< value<<" " ;
		}
		std::cout<<std::endl;
	}
#endif
	delete skel ;
	return 0 ;

}
