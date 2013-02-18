#include <io/Pgm3dFactory.h>
#include <ConnexComponentRebuilder.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <boost/filesystem.hpp>

typedef arma::u8  InType ;
typedef arma::u32 DistType ;
typedef arma::u8  OutType ;
typedef ConnexComponentRebuilder< InType, DistType, OutType > CCRB ;

using arma::span ;

namespace fs=boost::filesystem;
namespace po=boost::program_options ;


typedef struct _TProgramArg {
	fs::path  _skelFilePath ;
	fs::path  _depthFilePath ;
	fs::path  _outputFilePath ;
	uint      _xunit ;
	uint      _yunit ;
	uint      _zunit ;
} TProgramArg ;

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Shrinking scene based on the input filtered skeleton."<<std::endl
				<< general_opt << "\n";
}

bool missingParam ( std::string param ) {
	std::cerr <<" Parameter: "<<param<<" is required.."<<std::endl;
	return false ;
}

bool process_arg( int narg, char **argv, TProgramArg &params ) {
	po::options_description general_opt ( "Allowed options are: " );
	general_opt.add_options()
		( "help,h", "display this message." )
		( "skel,s",   po::value<std::string>(), "Input colored skeleton pgm filename." )
		( "depth,d",  po::value<std::string>(), "Input depth map pgm filename." )
		( "output,o", po::value<string>(),"Output pgm filename." )
		( "grid,g",   po::value<string>()->default_value("100x100x100"),"<xsize> x <ysize> x <zsize>" );

	bool parseOK = true ;
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(narg, argv, general_opt), vm);  
	} catch ( const std::exception& ex ) {
		parseOK = false ;
		std::cerr<< "Error checking program options: "<< ex.what()<< endl;
		return false ;
	}

	po::notify ( vm );
	if (!parseOK || vm.count ( "help" ) ||narg<=1 ) {
		errorAndHelp( general_opt ) ;
		return false ;
	}

	//Parse options
	if ( ! ( vm.count ( "skel" ) ) )   return missingParam ( "skel" );
	if ( ! ( vm.count ( "depth" ) ) )  return missingParam ( "depth" );
	if ( ! ( vm.count ( "output" ) ) ) return missingParam ( "output" );
	params._skelFilePath = vm["skel"].as<std::string>();
	params._depthFilePath = vm["depth"].as<std::string>();
	params._outputFilePath = vm["output"].as<std::string>();
	
	QString grid = vm["grid"].as<std::string>().c_str() ;
	QStringList grid_dims = grid.split( "x", QString::SkipEmptyParts);
	if ( grid_dims.size() != 3 ) {
		errorAndHelp( general_opt ) ;
		return false ;
	}
	params._xunit = grid_dims.at(0).toInt() ;
	params._yunit = grid_dims.at(1).toInt() ;
	params._zunit = grid_dims.at(2).toInt() ;
	return true ;
}


int main( int narg, char **argv ) {
	TProgramArg params ;
	
	if ( ! process_arg( narg, argv, params ) ) return -1 ;
	BillonTpl< InType >   *Img ;
	{
		Pgm3dFactory< InType > factory ;
		Img = factory.read( QString( params._skelFilePath.c_str() ) ) ;
		if ( Img->max() > 1 )
			*Img /= Img->max() ;
	}
	BillonTpl< DistType > *Depth ;
	{
		Pgm3dFactory< DistType > factory ;
		Depth = factory.read( QString( params._depthFilePath.c_str() ) ) ;
		factory.correctEncoding( Depth ) ;
	}
	

	DistType margin = Depth->max() ;
	
	if ( std::min( params._xunit, std::min(params._yunit,params._zunit) ) < 2*margin ) {
		std::cerr<<"[ Error ] : as max depth is "<<cast_integer<DistType,int>(margin)<<", grid is too small"<<std::endl;
		delete Img ;
		delete Depth ;
		return -2 ;
	}
	BillonTpl< OutType > scene( Img->n_rows, Img->n_cols, Img->n_slices ) ;
	scene.fill( 0 ) ;
	
	BillonTpl< InType >  subImg  ( params._yunit, params._xunit, params._zunit ) ;
	BillonTpl< DistType > subDepth( params._yunit, params._xunit, params._zunit ) ;
	
	uint x, y, z, xo, yo, zo,
	     n_rows, n_cols, n_slices ;
	
	for ( z = 0 ; z < Img->n_slices ; z+= params._zunit-2*margin )
		for ( y = 0 ; y < Img->n_rows ; y+= params._yunit-2*margin )
			for ( x = 0 ; x < Img->n_cols ; x+= params._xunit-2*margin ) {
				n_rows = std::min(y + params._yunit - 2 * margin ,Img->n_rows-1) - y ;
				n_cols = std::min(x + params._xunit - 2 * margin,Img->n_cols-1) - x ;
				n_slices = std::min(z + params._zunit - 2 * margin,Img->n_slices-1) - z ;
				subImg.fill(0) ;
				subImg  ( span( margin,margin+n_rows), span( margin,margin+n_cols), span( margin, margin+n_slices ) ) =
				          (*Img)( span( y, y + n_rows ), span( x, x + n_cols ), span( z, z + n_slices ) ) ;
				subDepth.fill(0) ;
				subDepth( span( margin,margin+n_rows), span( margin,margin+n_cols), span( margin, margin+n_slices ) ) =
				          (*Depth)( span( y, y + n_rows ), span( x, x + n_cols ), span( z, z + n_slices ) ) ;
				CCRB ccr( subImg ) ;
				ccr.setDepth( &subDepth ) ;
				ccr.run( ) ;
				const BillonTpl<OutType> &res = ccr.result() ;
				for ( zo = 0 ; zo < params._zunit; zo++ )
					for ( yo = 0; yo < params._yunit; yo++ )
						for ( xo = 0; xo < params._xunit; xo++ )
							if ( res( yo,xo,zo ) )
								scene( yo+y-margin,xo+x-margin,zo+z-margin ) = 1 ;
			}
	delete Img ;
	delete Depth ;
	IOPgm3d< OutType, qint8, false >::write( scene, QString( params._outputFilePath.c_str() ) ) ;
	return 0 ;
}
