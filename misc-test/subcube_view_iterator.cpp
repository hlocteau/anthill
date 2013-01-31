#include <armadillo>
#include <stdint.h>
int main( int narg, char **argv ) {
	arma::cube	C( 5,9, 11 ) ;
	int x,y,z ;
	int v=0;
	for ( z=0;z<C.n_slices;z++)
		for ( x=0;x<C.n_cols;x++ )
			for ( y = 0 ; y < C.n_rows;y++ )
				C(y,x,z) = v++ ;

	std::cout<<"all"<<std::endl;
	for ( arma::cube::iterator it = C.begin() ; it != C.end() ; it++ )
		std::cout<<*it<<" " ;
	std::cout<<std::endl;
	
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

arma::Cube< uint8_t > t ;

	return 0 ;
}
