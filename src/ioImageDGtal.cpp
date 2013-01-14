#include "DGtal/helpers/StdDefs.h"
#include "DGtal/io/readers/PNMReader.h"
#include "DGtal/images/ImageSelector.h"
#include "DGtal/io/writers/PNMWriter.h"
#include "DGtal/io/colormaps/GrayscaleColorMap.h"
using namespace DGtal ;

int main( int narg, char **argv )
{

	typedef ImageSelector < Z2i::Domain, uint>::Type Image;
	Image image = PNMReader<Image>::importPGM( argv[1] ); 
	
	std::cout<< image.domain()<<std::endl;
	
	
	
	typedef SpaceND<2> TSpace ;
	typedef TSpace::Point Point ;
	typedef HyperRectDomain<TSpace> Domain;
	
	Point a ( 1, 1);
	Point b ( 16, 16);
	typedef GrayscaleColorMap<unsigned char> Gray;
	typedef ImageSelector< Z2i::Domain, unsigned char>::Type GrayImage;
	GrayImage image2(Domain(a,b));
	
	for ( uint y = 3 ; y < 12 ; y++ )
	{
		image2.setValue( Point( 6,y ), 255 ) ;
		image2.setValue( Point( 10,y ), 255 ) ;
	}
	image2.setValue( Point(7,8), 255 ) ;
	image2.setValue( Point(8,8), 255 ) ;
	image2.setValue( Point(9,8), 255 ) ;
	
	PNMWriter<GrayImage,Gray>::exportPGM("export-gray.pgm",image2,0,255);
	return 0 ;
}
