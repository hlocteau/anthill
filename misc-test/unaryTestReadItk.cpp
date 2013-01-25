#include <itkImage.h>
#include <itkImageFileReader.h>

int main( int narg, char **argv ) {
	typedef /*unsigned char*/double 						PixelType ;
	const unsigned int							Dimension = 2 ;
	typedef itk::Image< PixelType, Dimension >	ImageType ;
	typedef itk::ImageFileReader< ImageType >	ReaderType ;
	
	
	ReaderType::Pointer reader = ReaderType::New() ;
	reader->SetFileName( argv[1] ) ;
	std::cout<<"ok at line "<<__LINE__<<std::endl;
	reader->Update() ;
	std::cout<<"ok at line "<<__LINE__<<std::endl;
	ImageType::Pointer image = reader->GetOutput() ;
	std::cout<<"ok at line "<<__LINE__<<std::endl;
	itk::ImageIOBase * imageio = reader->GetImageIO() ;
	std::cout<<"ok at line "<<__LINE__<<std::endl;
	
	
	unsigned int 	width 	= imageio->GetDimensions( 0 ),
					height 	= imageio->GetDimensions( 1 ) ;
					
	std::cout<<"Size of image \""<< argv[1]<<"\" is "<<width<<" x " <<height<<std::endl;
					
	return 0 ;
	
}
