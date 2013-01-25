#include <io/Pgm3dFactory.h>
#include <../test/connexcomponentextractortest.h>
#include <def_coordinate.h>
#include <coordinate.h>

typedef Pgm3dFactory<char> 	CPgm3dFactory ;
typedef BillonTpl<char>		CImage ;
int main( int narg, char **argv ) {
	
	const char *inputFileName = argv[1] ;
	
	Billon img ; /// ... input image
	{
		CPgm3dFactory factory ;
		CImage *skel = factory.read( QString( "%1").arg(inputFileName) ) ;
		int N = std::max( (int)skel->n_rows, (int)skel->n_cols) +2 ;
		factory.correctEncoding( skel ) ;
		/// set img=skel 
		img = Billon( N, N, skel->n_slices+2 ) ; /// add white boundary
		img.fill(0);
		register int x,y,z;
		for ( z = 0 ; z < skel->n_slices ; z++ )
			for ( x = 0 ; x < skel->n_cols ; x++ )
				for ( y = 0 ; y < skel->n_rows ; y++ ) {
					img(y+1,x+1,z+1) = (*skel)(y,x,z) ;
				}
		delete skel ;
	}
	std::cout<<"Input image's size is "<<img.n_cols<<" x "<<img.n_rows<<" x " <<img.n_slices<<std::endl;
	Billon *res = ConnexComponentExtractor::extractConnexComponents( img, 0, 0 ) ;
	IOPgm3d< __billon_type__, qint32, false>::write( *res, QString("/tmp/labelWithV3D.pgm3d") ) ;
	
	iCoord3D *lower = new iCoord3D[ res->maxValue()+1 ] ;
	uint32_t *volume = new uint32_t[ res->maxValue()+1 ] ;
	{
		register int lbl ;
		for ( lbl=1;lbl<=res->maxValue();lbl++ ) {
			lower[lbl]=iCoord3D( img.n_cols,img.n_rows,img.n_slices);
			volume[lbl]=0;
		}
		register int x,y,z;
		for ( z = 0 ; z < res->n_slices ; z++ )
			for ( x = 0 ; x < res->n_cols ; x++ )
				for ( y = 0 ; y < res->n_rows ; y++ ) {
					lbl = (*res)(y,x,z) ;
					if ( lbl == 0 ) continue ;
					if ( x < lower[lbl].x ) lower[lbl].x=x;
					if ( y < lower[lbl].y ) lower[lbl].y=y;
					if ( z < lower[lbl].z ) lower[lbl].z=z;
					volume[lbl]++;
				}
	}
	for ( int lbl=1;lbl<=res->maxValue() ;lbl++ )
		std::cout<<setw(4)<<lbl<<":"<<setw(6)<<volume[lbl]<<";"<<setw(4)<<lower[lbl].x<<";"<<setw(4)<<lower[lbl].y<<";"<<setw(4)<<lower[lbl].z<<std::endl;
	
	delete [] volume ;
	delete [] lower ;
	delete res ;
	return 0 ;
}
