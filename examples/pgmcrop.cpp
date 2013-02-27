/**
 * \file pgmcrop.cpp
 */
#include <io/Pgm3dFactory.h>

typedef arma::u32 elem_type ;
typedef Pgm3dFactory<elem_type> IFactory ;

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem ;

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Display of pgm images."<<std::endl
				<< general_opt << "\n";
}
void missingParam ( std::string param )
{
  std::cerr <<" Parameter: "<<param<<" is required.."<<std::endl;
  exit ( 1 );
}

int main( int narg, char **argv ) {
	
	// parse command line ----------------------------------------------
	po::options_description general_opt ( "Allowed options are: " );
	general_opt.add_options()
		( "help,h", "display this message." )
		( "input,i", po::value< std::string >(), "Input pgm filename(s)." )
		( "output,o", po::value< std::string >(), "Output pgm filename(s)." )
		( "xmin,x", po::value<int>()->default_value(0), "xmin.")
		( "Xmax,X", po::value<int>()->default_value(-1), "xmax.")
		( "ymin,y", po::value<int>()->default_value(0), "ymin.")
		( "Ymax,Y", po::value<int>()->default_value(-1), "ymax.")
		( "zmin,z", po::value<int>()->default_value(0), "zmin.")
		( "Zmax,Z", po::value<int>()->default_value(-1), "zmax.");

	bool parseOK = true ;
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(narg, argv, general_opt), vm);  
	} catch ( const std::exception& ex ) {
		parseOK = false ;
		std::cerr<< "Error checking program options: "<< ex.what()<< endl;
		return -1 ;
	}

	po::notify ( vm );
	if (!parseOK || vm.count ( "help" ) ||narg<=1 ) {
		errorAndHelp( general_opt ) ;
		return -1 ;
	}

	//Parse options
	if ( ! ( vm.count ( "input" ) ) ) missingParam ( "input" );
	fs::path inputFilePath = vm["input"].as< std::string >();
	int xmin = vm["xmin"].as<int>() ;
	int ymin = vm["ymin"].as<int>() ;
	int zmin = vm["zmin"].as<int>() ;
	int xmax = vm["Xmax"].as<int>() ;
	int ymax = vm["Ymax"].as<int>() ;
	int zmax = vm["Zmax"].as<int>() ;
	
	fs::path outputFilePath ;
	if ( vm.count ( "output" ) ) outputFilePath = vm["output"].as< std::string >() ;
	
	
	IFactory factory ;
	BillonTpl< elem_type > *img = factory.read( inputFilePath.c_str() ) ;
	std::cout<<"Info : input size is "<<img->n_cols<<" x "<<img->n_rows<<" "<<img->n_slices<<std::endl;
	/// check bounds	
	if ( xmax == -1 ) xmax = img->n_cols ;
	if ( ymax == -1 ) ymax = img->n_rows ;
	if ( zmax == -1 ) zmax = img->n_slices ;
	
	xmax = std::min( xmax, (int)img->n_cols ) ;
	ymax = std::min( ymax, (int)img->n_rows ) ;
	zmax = std::min( zmax, (int)img->n_slices ) ;
	
	xmin = std::min( std::max(xmin,0), xmax-1 ) ;
	ymin = std::min( std::max(ymin,0), ymax-1 ) ;
	zmin = std::min( std::max(zmin,0), zmax-1 ) ;

	BillonTpl< elem_type > crop( ymax-ymin, xmax-xmin,zmax-zmin) ;
	std::cout<<"Info : output size is "<<crop.n_cols<<" x "<<crop.n_rows<<" x "<<crop.n_slices<<std::endl;
	int x,y,z;
	for ( z = zmin ; z < zmax ; z++ )
		for ( y = ymin ; y < ymax ; y++ )
			for ( x = xmin ; x < xmax ; x++ )
				crop(y-ymin,x-xmin,z-zmin) = (*img)( y,x,z ) ;
	crop.setMinValue(crop.min());
	crop.setMaxValue(crop.max());
	delete img ;
	if ( outputFilePath.empty() ) {
		outputFilePath = inputFilePath.parent_path() ;
		outputFilePath /= QString("%1.crop_x%2-%3_y%4-%5_z%6-%7%8").arg(inputFilePath.stem().c_str()).arg(xmin).arg(xmax).arg(ymin).arg(ymax).arg(zmin).arg(zmax).arg( inputFilePath.extension().c_str() ).toStdString().c_str() ;
		std::cout<<"Info : use default output file name convention "<<outputFilePath.c_str()<<std::endl;
	}
	if ( factory.was_char() )
		IOPgm3d< elem_type, qint8, false>::write( crop, outputFilePath.c_str() ) ;
	else if ( factory.was_integer() )
		IOPgm3d< elem_type, qint32, false>::write( crop, outputFilePath.c_str() ) ;
	else {
		std::cerr<<"Error : input data were not integers. This program is supposed to manipulate this kind of image. No output."<<std::endl;
		return -2 ;
	}
	return 1 ;
}
