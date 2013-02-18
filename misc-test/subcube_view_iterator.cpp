#include <armadillo>
#include <stdint.h>

struct iCoord3D {
	uint x,y,z;
} ;

#define N_ROWS 9
#define N_COLS 9
#define N_SLICES 11

bool validDomain( iCoord3D &l, iCoord3D &u ) {
	std::cout<<"Bounds : ";
	std::cin>>l.x>>l.y>>l.z;
	std::cin>>u.x>>u.y>>u.z;
	return ( l.x>=0 && l.y>=0 && l.z>=0 &&
	         u.x>=l.x && u.y>=l.y && u.z>=l.z &&
	         u.x<N_COLS && u.y < N_ROWS && u.z <N_SLICES );
}
	
	
int main( int narg, char **argv ) {
	arma::cube	C( N_ROWS,N_COLS, N_SLICES ) ;
	uint x,y,z ;
	uint v=0;
	for ( z=0;z<C.n_slices;z++)
		for ( x=0;x<C.n_cols;x++ )
			for ( y = 0 ; y < C.n_rows;y++ )
				C(y,x,z) = v++ ;

	std::cout<<"all"<<std::endl;
	for ( arma::cube::iterator it = C.begin() ; it != C.end() ; it++ )
		std::cout<<*it<<" " ;
	std::cout<<std::endl;
/**	
	{
		std::cout<<"row 3"<<std::endl;
		arma::cube Cv = C( arma::span(3), arma::span(), arma::span() ) ;
		std::cout<<"with iterator"<<std::endl<<"\t";
		for ( arma::cube::iterator it = Cv.begin() ; it != Cv.end() ; it++ )
			std::cout<<*it<<" " ;
		std::cout<<std::endl;
		std::cout<<"with indices"<<std::endl<<"\t";
		for ( z = 0 ; z < C.n_slices ; z++ )
			for ( x=0;x<C.n_cols;x++ )
				std::cout<<C(3,x,z)<<" " ;
		std::cout<<std::endl;
	}

	{
		std::cout<<"col 7"<<std::endl;
		std::cout<<"with iterator"<<std::endl<<"\t";
		arma::cube Cv = C( arma::span(), arma::span(7), arma::span() ) ;
		for ( arma::cube::iterator it = Cv.begin() ; it != Cv.end() ; it++ )
			std::cout<<*it<<" " ;
		std::cout<<std::endl;
		std::cout<<"with indices"<<std::endl<<"\t";
		for ( z = 0 ; z < C.n_slices ; z++ )
			for ( y=0;y<C.n_rows;y++ )
				std::cout<<C(y,7,z)<<" " ;
		std::cout<<std::endl;
	}

	{
		std::cout<<"slice 10"<<std::endl;
		std::cout<<"with iterator"<<std::endl<<"\t";
		arma::cube Cv = C( arma::span(), arma::span(), arma::span(10) ) ;
		for ( arma::cube::iterator it = Cv.begin() ; it != Cv.end() ; it++ )
			std::cout<<*it<<" " ;
		std::cout<<std::endl;
		std::cout<<"with indices"<<std::endl<<"\t";
		for ( x = 0 ; x < C.n_cols ; x++ )
			for ( y=0;y<C.n_rows;y++ )
				std::cout<<C(y,x,10)<<" " ;
		std::cout<<std::endl;
	}
*/	
	iCoord3D l, u ;
	while ( validDomain( l,u ) ) {
		std::cout<<"print using index"<<std::endl;
		for ( uint z = l.z ; z <= u.z ; z++ ) {
			for ( uint x = l.x ;  x <= u.x ; x++ ) {
				for ( uint y = l.y ; y <= u.y ; y++ )
					std::cout << C(y,x,z)<<" " ;
			}
		}
		std::cout<<std::endl;
		std::cout<<"print using iterator"<<std::endl;
		arma::subview_cube<double> Cv = C(arma::span(l.y,u.y), arma::span(l.x,u.x), arma::span(l.z,u.z) ) ;
		for ( arma::subview_cube<double>::iterator it = Cv.begin() ; it != Cv.end() ; it++ ) {
			std::cout<<*it<<" " ;
		}
		std::cout<<std::endl;
		* Cv.begin() = 4 ;
		std::cout << C(l.y,l.x,l.z)<<" "<<Cv(0,0,0) ;
		std::cout<<std::endl;
	}
		
	

arma::Cube< uint8_t > t ;

	return 0 ;
}
