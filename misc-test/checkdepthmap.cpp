#include <io/IOUtils.h>
#include <iomanip>
typedef arma::u32 DepthType ;
typedef arma::u8 MaskType ;
using DGtal::Z3i::Point ;
int main( int narg, char **argv ) {
	BillonTpl< MaskType > *mask = load_maincc( argv[2] ) ;
	BillonTpl< DepthType > *pdata = load_data_withmask<DepthType >( argv[1], mask ) ;

	Point lower(195-108-10,282- 163-10,266- 13-10), upper ;
	upper = Point( lower.at(0)+20, lower.at(1)+20, lower.at(2)+20 ) ;

	uint z, y, x ;
	char c ;
	for ( z = lower.at(2) ; z != upper.at(2) ;z++ ) {
		for ( y = lower.at(1) ; y != upper.at(1) ;y++ ) {
			for ( x = lower.at(0) ; x != upper.at(0) ;x++ )
				std::cout<<setw(3)<<(int) (*pdata)(y,x,z)<<" " ;
			std::cout<<std::endl;
		}
		std::cin>> c ;
	}
	
	delete pdata ;
	return 0 ;
}
