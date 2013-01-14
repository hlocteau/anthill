/// behaviour of v3D/src/connexcomponentextractor.cpp
/// it seems we gather voxels not belonging to the same connected component in the antHouse/include/connexcomponentextractor.ih

#include <connexcomponentextractor.h>
#include <Pgm3dFactory.h>
#include <image3d.h>
#include <connexcomponent.h>
#include <regressivelementlinearly.h>
typedef Pgm3dFactory<char> 	CPgm3dFactory ;
typedef BillonTpl<char>		CImage ;

class Foo : public ConnexComponentExtractor {
public:
	Foo( ) {};
	void job( arma::icube &input, Image3D &output ) {
		extractConnexComponent( input, output ) ;
	}
} ;

int main( int narg, char **argv ) {
	
	const char *inputFileName = argv[1] ;
	
	arma::icube img ; /// ... input image
	{
		CPgm3dFactory factory ;
		CImage *skel = factory.read( QString( "%1").arg(inputFileName) ) ;
		int N = std::max( (int)skel->n_rows, (int)skel->n_cols) +2 ;
		factory.correctEncoding( skel ) ;
		/// set img=skel 
		img = arma::icube( N, N, skel->n_slices+2 ) ; /// add white boundary
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
	Image3D im3D( img.n_cols, img.n_rows, img.n_slices) ; /// output labelled image
	
	Foo CCE ;
	CCE.setThresholdExtraction(0);
	CCE.job( img, im3D ) ;
	std::cout<<"Output result's size is "<<im3D.getWidth()<<" x "<<im3D.getHeight()<<" x " <<im3D.getDepth()<<std::endl;
	std::cout<<"Get "<<im3D.getConnexComponentNumber()<<" component(s)"<<std::endl;
	if ( im3D.getConnexComponentNumber() > 0 ) {
		/// build a labelled image
		BillonTpl<int32_t> labels( im3D.getWidth(), im3D.getHeight(), im3D.getDepth() ) ;
		labels.fill( 0 ) ;
		labels.setMinValue(0) ;
		int number = im3D.getConnexComponentNumber() ;
		register int x, y, z ;
		bool is_invariant, check_invariant = true ;
		bool xleft, xright,yleft,yright,zleft,zright ;
		for ( int iCC = 1 ; iCC <= number ; iCC++ ) {
			if ( iCC != 170 && iCC != 172 ) continue ;
			const ConnexComponent * pCC = im3D.getConnexComponent( iCC ) ;
			int32_t vol = 0 ;
			for ( z = 0 ; z < pCC->n_slices ; z++ )
				for ( y = 0 ; y < pCC->getHeight() ; y++ )
					for ( x = 0 ; x < pCC->getWidth() ; x++ ) {
						if ( (*pCC).value(x,y,z) ) {
							vol++ ;
							labels( y + pCC->getMinimalCoord().y, x + pCC->getMinimalCoord().x, z + pCC->getMinimalCoord().z ) = iCC ;
							if ( ! check_invariant ) continue ;
							xleft=(x>0) ;
							xright=(x<pCC->n_rows-1);
							yleft=(y>0) ;
							yright=(y<pCC->n_cols-1);
							zleft=(z>0) ;
							zright=(z<pCC->n_slices-1);
							/// check the invariant : if a neighbor is not null, its value should be equal ...... ON LABELS!
							is_invariant =  ( ( xleft  && 			( (*pCC).value(x-1,y  ,z  ) == 0 || (*pCC).value(x-1,y  ,z  ) == (*pCC).value(x,y,z) ) ) || !xleft) &&
											( ( xright && 			( (*pCC).value(x+1,y  ,z  ) == 0 || (*pCC).value(x+1,y  ,z  ) == (*pCC).value(x,y,z) ) ) || !xright) &&
											( ( 		  yleft  &&	( (*pCC).value(x  ,y-1,z  ) == 0 || (*pCC).value(x  ,y-1,z  ) == (*pCC).value(x,y,z) ) ) || !yleft) &&
											( ( xleft  && yleft  && ( (*pCC).value(x-1,y-1,z  ) == 0 || (*pCC).value(x-1,y-1,z  ) == (*pCC).value(x,y,z) ) ) || (!xleft  || !yleft) ) &&
											( ( xright && yleft  && ( (*pCC).value(x+1,y-1,z  ) == 0 || (*pCC).value(x+1,y-1,z  ) == (*pCC).value(x,y,z) ) ) || (!xright || !yleft) ) &&
											( ( yright && 			( (*pCC).value(x  ,y+1,z  ) == 0 || (*pCC).value(x  ,y+1,z  ) == (*pCC).value(x,y,z) ) ) || !yright) &&
											( ( xleft  && yright && ( (*pCC).value(x-1,y+1,z  ) == 0 || (*pCC).value(x-1,y+1,z  ) == (*pCC).value(x,y,z) ) ) || (!xleft  || !yright) ) &&
											( ( xright && yright && ( (*pCC).value(x+1,y+1,z  ) == 0 || (*pCC).value(x+1,y+1,z  ) == (*pCC).value(x,y,z) ) ) || (!xright || !yright) ) ;
							if ( zleft )
								is_invariant &= 					( (*pCC).value(x  ,y  ,z-1) == 0 || (*pCC).value(x  ,y  ,z-1) == (*pCC).value(x,y,z) ) &&
											( ( xleft  && 			( (*pCC).value(x-1,y  ,z-1) == 0 || (*pCC).value(x-1,y  ,z-1) == (*pCC).value(x,y,z) ) ) || !xleft) &&
											( ( xright && 			( (*pCC).value(x+1,y  ,z-1) == 0 || (*pCC).value(x+1,y  ,z-1) == (*pCC).value(x,y,z) ) ) || !xright) &&
											( ( yleft  &&			( (*pCC).value(x  ,y-1,z-1) == 0 || (*pCC).value(x  ,y-1,z-1) == (*pCC).value(x,y,z) ) ) || !yleft) &&
											( ( xleft  && yleft  &&	( (*pCC).value(x-1,y-1,z-1) == 0 || (*pCC).value(x-1,y-1,z-1) == (*pCC).value(x,y,z) ) ) || (!xleft  || !yleft) ) &&
											( ( xright && yleft  &&	( (*pCC).value(x+1,y-1,z-1) == 0 || (*pCC).value(x+1,y-1,z-1) == (*pCC).value(x,y,z) ) ) || (!xright || !yleft) ) &&
											( ( 		  yright &&	( (*pCC).value(x  ,y+1,z-1) == 0 || (*pCC).value(x  ,y+1,z-1) == (*pCC).value(x,y,z) ) ) || !yright) &&
											( ( xleft  && yright &&	( (*pCC).value(x-1,y+1,z-1) == 0 || (*pCC).value(x-1,y+1,z-1) == (*pCC).value(x,y,z) ) ) || (!xleft  || !yright) ) &&
											( ( xright && yright &&	( (*pCC).value(x+1,y+1,z-1) == 0 || (*pCC).value(x+1,y+1,z-1) == (*pCC).value(x,y,z) ) ) || (!xright || !yright) ) ;
							if ( zright )
								is_invariant &=						( (*pCC).value(x  ,y  ,z+1) == 0 || (*pCC).value(x  ,y  ,z+1) == (*pCC).value(x,y,z) ) &&
											( ( xleft  && 			( (*pCC).value(x-1,y  ,z+1) == 0 || (*pCC).value(x-1,y  ,z+1) == (*pCC).value(x,y,z) ) ) || !xleft) &&
											( ( xright && 			( (*pCC).value(x+1,y  ,z+1) == 0 || (*pCC).value(x+1,y  ,z+1) == (*pCC).value(x,y,z) ) ) || !xright) &&
											( ( 		  yleft  &&	( (*pCC).value(x  ,y-1,z+1) == 0 || (*pCC).value(x  ,y-1,z+1) == (*pCC).value(x,y,z) ) ) || !yleft) &&
											( ( xleft  && yleft  &&	( (*pCC).value(x-1,y-1,z+1) == 0 || (*pCC).value(x-1,y-1,z+1) == (*pCC).value(x,y,z) ) ) || (!xleft  || !yleft) ) &&
											( ( xright && yleft  &&	( (*pCC).value(x+1,y-1,z+1) == 0 || (*pCC).value(x+1,y-1,z+1) == (*pCC).value(x,y,z) ) ) || (!xright || !yleft) ) &&
											( ( 		  yright && ( (*pCC).value(x  ,y+1,z+1) == 0 || (*pCC).value(x  ,y+1,z+1) == (*pCC).value(x,y,z) ) ) || !yright) &&
											( ( xleft  && yright && ( (*pCC).value(x-1,y+1,z+1) == 0 || (*pCC).value(x-1,y+1,z+1) == (*pCC).value(x,y,z) ) ) || (!xleft  || !yright) ) &&
											( ( xright && yright && ( (*pCC).value(x+1,y+1,z+1) == 0 || (*pCC).value(x+1,y+1,z+1) == (*pCC).value(x,y,z) ) ) || (!xright || !yright) ) ;
							if ( !is_invariant ) {
								std::cerr<<"Error : The image we obtained is ill-defined as labelled map can get adjacent voxels with distinct labels, e.g. "<<x<<","<<y<<","<<z<<std::endl;
								check_invariant = false ;
								register int xx,yy,zz;
								for ( zz=std::max((int)0,z-1);zz<=std::min((int)(*pCC).n_slices-1,z+1);zz++ )
									std::cerr<<"[z="<<zz<<"] ";
								std::cerr<<std::endl;

								for ( yy=std::max((int)0,y-1);yy<=std::min((int)(*pCC).n_rows-1,y+1);yy++ ) {
									for ( zz=std::max((int)0,z-1);zz<=std::min((int)(*pCC).n_slices-1,z+1);zz++ ) {
										for ( xx=std::max((int)0,x-1);xx<=std::min((int)(*pCC).n_cols-1,x+1);xx++ )
											std::cerr<<(*pCC).value(xx,yy,zz)<<" " ;
										std::cerr<<"\t\t";
									}
									std::cerr<<std::endl;
								}
								
							}
						
						}
					}
			std::cout<<setw(6)<<(int)iCC<<":"<<setw(7)<<vol<<";"<<setw(4)<<pCC->getMinimalCoord().x<<";"<<setw(4)<<pCC->getMinimalCoord().y<<";"<<setw(4)<<pCC->getMinimalCoord().z<<","<<pCC->getMaximumCoord().x<<";"<<setw(4)<<pCC->getMaximumCoord().y<<";"<<setw(4)<<pCC->getMaximumCoord().z<<std::endl;
		}
		labels.setMaxValue( number ) ;
		IOPgm3d< int32_t,qint32, false>::write( labels, QString("/tmp/labelWithV3D.pgm3d") ) ;
	}
	return 1 ;
}
