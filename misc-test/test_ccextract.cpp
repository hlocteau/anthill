#include <connexcomponentextractor.h>

typedef char src_type ;
typedef short label_type ;

int main( int narg, char **argv ) {
	BillonTpl<src_type> im(5,10,7) ;
	im.fill(0);
	im(2,4,1)=1;
	im(2,5,1)=1;
	im(2,4,2)=1;
	
	im(3,7,5)=1;
	
	im(1,9,6)=1;
	
	ConnexComponentExtractor< src_type, label_type> extractor ;
	BillonTpl<label_type> *label = extractor.run( im ) ;
	
	std::cerr<<"Info : "<<(*label)(2,4,1)<<" / "<<(*label)(2,5,1)<<" / "<<(*label)(2,4,2)<<std::endl;
	std::cerr<<"Info : "<<(*label)(3,7,5)<<std::endl;
	std::cerr<<"Info : "<<(*label)(1,9,6)<<std::endl;
	
	
	delete label ;
	
	return 0 ;
}
