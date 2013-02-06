/**
 * \file view3dPgm.cpp
 */
#include <DGtal/helpers/StdDefs.h>

#include <io/Pgm3dFactory.h>

#include <QtGui/qapplication.h>
#include <DGtal/io/viewers/Viewer3D.h>
#include <DGtal/io/colormaps/HueShadeColorMap.h>
#include <utils.h>

#include <DGtal/io/Color.h>
#include <DGtal/io/colormaps/GradientColorMap.h>

using namespace std;
using namespace DGtal;
using namespace Z3i;

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Display of pgm images."<<std::endl
				<< general_opt << "\n";
}
void missingParam ( std::string param )
{
  /*trace.error()*/std::cerr <<" Parameter: "<<param<<" is required.."<<std::endl;
  exit ( 1 );
}

typedef uint32_t src_type ;

const double golden_ratio_conjugate = 0.618033988749895 ;
QColor makeRgbColor( double &hue ) {
  hue += golden_ratio_conjugate ;
  if ( hue > 1. ) hue -= 1. ;
  return  QColor::fromHsv( (int)floor(hue*359), 76, 252) ;
}

int main( int narg, char **argv ) {

	
	double *param_clipping = 0 ;
	int nClipping = 0 ;
	
	// parse command line ----------------------------------------------
	po::options_description general_opt ( "Allowed options are: " );
	general_opt.add_options()
		( "help,h", "display this message." )
		( "input,i", po::value< std::string >()->multitoken(), "Input pgm filename(s)." )
		( "clipping,p", po::value<std::string>()->multitoken(), "vector's coordinates. <val_a> <val_b> <val_c> <val_d>" )
		( "domain,d", po::value<bool>()->default_value(true),"Display the overall domain." )
		( "bbox,b", po::value<bool>()->default_value(true),"Display bounding boxes." )
		( "mono,m", po::value<bool>()->default_value(true),"Use a single channel color." )
		( "number,n", po::value<int>()->default_value(0), "minimum number of colors (impact on the distance between two \'consecutive\' colors).")
		( "colormap,c", po::value<bool>()->default_value(true),"Use a gradient based colormap when using multiple channels' color." )
		( "boundary,b", po::value<bool>()->default_value(true),"Display only boundary voxels." )
		( "selection,s", po::value< std::string >()->multitoken(), "draw only specific id.")
		( "export,e", po::value< std::string >(), "export the corresponding scene (filename of a pgm3d file).")
		( "rgb", po::value<bool>()->default_value(true), "When export is used, export colors (labels only when value is false).")
		( "xmin,x", po::value<int>()->default_value(0), "xmin.")
		( "Xmax,X", po::value<int>()->default_value(-1), "xmax.")
		( "ymin,y", po::value<int>()->default_value(0), "ymin.")
		( "Ymax,Y", po::value<int>()->default_value(-1), "ymax.")
		( "zmin,z", po::value<int>()->default_value(0), "zmin.")
		( "Zmax,Z", po::value<int>()->default_value(-1), "zmax.")
		( "voxel,v", po::value<bool>()->default_value(true), "display voxel instead of surfel when boundary is required.")
		( "transparency,t", po::value<bool>()->default_value(true), "display voxel using transparency")
		( "label,l", po::value<bool>()->default_value(false),"Use existing labeling." )
		( "verbose", po::value<bool>()->default_value(false),"output additional information." )
		( "golden,g",po::value<bool>()->default_value(false),"use hsv color generator." );

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
	arma::Cube< arma::u8 > *resultingscene_8 = 0 ;
	arma::Cube< arma::u16 > *resultingscene_16 = 0 ;
	QList< src_type > Labels;
	if ( vm.count("selection") ) {
		QStringList selectedLabels = QString( "%1").arg( vm["selection"].as< std::string >().c_str() ).split( " ", QString::SkipEmptyParts) ;
		while ( !selectedLabels.isEmpty() )
			Labels.append( (src_type)selectedLabels.takeAt(0).toInt() ) ;
		qSort( Labels.begin(), Labels.end(), qLess<src_type>() ) ;
	}

	//Parse options
	if ( ! ( vm.count ( "input" ) ) ) missingParam ( "input" );
	std::string inputFileNames = vm["input"].as< std::string >();
	QStringList files = QString( "%1").arg( inputFileNames.c_str() ).split( " ", QString::SkipEmptyParts) ;
	bool clipping = vm.count ( "clipping" ) ;
	if ( clipping ) {
		std::string stringparams = vm["clipping"].as< std::string >();
		QStringList params = QString( "%1").arg( stringparams.c_str() ).split( " ", QString::SkipEmptyParts) ;
		if ( params.size() % 4  != 0 ) { std::cerr<<"Warning : expecting 4 values. Clipping desactivated."<<std::endl; clipping = false ; }
		else {
			param_clipping = new double [ params.size() ] ;
			for (int iParam = 0 ; iParam < params.size() ; iParam++ )
				param_clipping[ iParam ] = params.at( iParam ).toDouble( );
			nClipping = params.size() / 4 ;
		}
	}
	bool display_domain = vm["domain"].as<bool>();
	bool use_labeling = vm["label"].as<bool>();
	bool boundary_only = vm["boundary"].as<bool>();
	bool single_channelColor = vm["mono"].as<bool>();
	bool display_bbox = vm["bbox"].as<bool>();
	bool display_voxel = vm["voxel"].as<bool>();
	
	int xmin = vm["xmin"].as<int>() ;
	int ymin = vm["ymin"].as<int>() ;
	int zmin = vm["zmin"].as<int>() ;
	int xmax = vm["Xmax"].as<int>() ;
	int ymax = vm["Ymax"].as<int>() ;
	int zmax = vm["Zmax"].as<int>() ;

	
	// parse command line ----------------------------------------------
	
	Pgm3dFactory< src_type > factory ;
	BillonTpl<src_type> *img = /*io::Pgm3dFactory::*/factory.read( files.at(0) ) ;

	int 	width = img->n_cols,
			height = img->n_rows,
			depth = img->n_slices ;
	
	if ( xmax == -1 ) xmax = width ;
	if ( ymax == -1 ) ymax = height ;
	if ( zmax == -1 ) zmax = depth ;
	
	xmax = min( xmax, width ) ;
	ymax = min( ymax, height ) ;
	zmax = min( zmax, depth ) ;
	
	xmin = min( max(xmin,0), xmax-1 ) ;
	ymin = min( max(ymin,0), ymax-1 ) ;
	zmin = min( max(zmin,0), zmax-1 ) ;
	
	Point 	p( xmin,ymin,zmin ),
			q( xmax, ymax, zmax ) ;

	Domain 	domain( p, q ) ;
	QApplication application(narg,argv);
	Viewer3D viewer ;
	viewer.setWindowTitle( QString("%1 - x [%2:%3] y [%4:%5] z [%6:%7]").arg(files.at(0).toStdString().c_str()).arg(xmin).arg(xmax).arg(ymin).arg(ymax).arg(zmin).arg(zmax).toStdString().c_str() );
	viewer.show();
	

	GrayLevelHistogram<src_type> h( *img,xmin,xmax,ymin,ymax,zmin,zmax ) ;
	GrayLevelHistogram<src_type>::THistogram &histo = h._bin ;
	if ( inputFileNames.size() > 1 ) {
		for ( int iFile = 1 ; iFile != files.size() ; iFile++ ) {
			BillonTpl<src_type> *imgAdd = /*io::Pgm3dFactory::*/factory.read( files.at(iFile) ) ;
			/// the image values have to be shift by histo.rbegin()->first when values are different from 0
			int32_t shift_value = h._bin.rbegin()->first ;
			for ( int z = zmin ; z < zmax ; z++ )
				for ( int y = ymin ; y < ymax ; y++ )
					for ( int x = xmin ; x < xmax ; x++ )
						if ( imgAdd->at(y,x,z) != 0 ) {
							img->at( y, x, z ) = imgAdd->at(y,x,z) + shift_value ;
							if (histo.find( img->at( y, x, z ) ) == histo.end() ) histo[ img->at( y, x, z ) ] = 0 ;
							histo[ img->at( y, x, z ) ]++ ;
						}
			delete imgAdd ;
		}
	}

	uint size_voxel = 1 ;
	if ( vm.count ( "export" ) ) {
		if ( vm["rgb"].as<bool>() && use_labeling ) size_voxel = 3 ;
		uint nRequired ;
		if ( size_voxel == 1 ) {
			if ( Labels.isEmpty() ) nRequired = histo.size() ;
			else nRequired = Labels.size() ;
		}
		/**
		 * \todo enable to export the label and the palette only
		 *       enable import a colored image! (with or without a palette)
		 */
		if ( size_voxel == 3  ) 
			resultingscene_8 = new arma::Cube< arma::u8 >( img->n_rows, img->n_cols * size_voxel, img->n_slices ) ;
		else if ( nRequired < 256 )
			resultingscene_8 = new arma::Cube< arma::u8 >( img->n_rows, img->n_cols, img->n_slices ) ;
		else
			resultingscene_16 = new arma::Cube< arma::u16 >( img->n_rows, img->n_cols, img->n_slices ) ;
		if ( resultingscene_8 ) resultingscene_8->fill( 0 ) ;
		else resultingscene_16->fill( 0 ) ;
	}

	if ( display_domain ) {
		viewer << SetMode3D( domain.className(), "BoundingBox") ;
		viewer << domain ;
	}
	//std::vector< std::vector< Z3i::SCell > > vectConnectedSCell;
	if ( use_labeling ) {
		double hue=( rand() % 1000 ) / 1000. ;
		
		std::map< src_type, DigitalSet * > map_obj ;
		for ( GrayLevelHistogram<src_type>::THistogram::const_iterator bin = h._bin.begin() ; bin != h._bin.end() ; bin ++ ) {
			map_obj[ bin->first ] = new DigitalSet( domain ) ;
			if ( vm["verbose"].as<bool>() ) std::cout<<bin->first<<" : "<<bin->second<<std::endl;
		}
		for ( int z = zmin ; z < zmax ; z++ )
			for ( int y = ymin ; y < ymax ; y++ )
				for ( int x = xmin ; x < xmax ; x++ )
					if ( img->at(y,x,z) != 0 ) {
						if ( boundary_only ) {
							/// insert this voxel iff at least one neighbor is 0
							if (img->at(y,max(0,x-1),z) <img->at(y,x,z) || img->at(y, min(width-1,x+1),z) <img->at(y,x,z) || 
								img->at(max(0,y-1),x,z) <img->at(y,x,z) || img->at(min(height-1,y+1),x,z) <img->at(y,x,z) || 
								img->at(y,x,max(0,z-1)) <img->at(y,x,z) || img->at(y,x,min(depth-1,z+1)) <img->at(y,x,z) )
								map_obj[ img->at(y,x,z) ]->insertNew( Point(x,y,z) ) ;
						} else {
							map_obj[ img->at(y,x,z) ]->insertNew( Point(x,y,z) ) ;
						}
					}
		delete img ;	
		int iColor = 1 ;
		int nColor = histo.size()+1 ;
		if ( vm["number"].as<int>() > nColor ) nColor = vm["number"].as<int>() ;
		int stepColor = (int)floor( log( (double)nColor ) / log( 3. ) + 1 );
		
		GradientColorMap<int> cmap_grad( 0, nColor, CMAP_HOT );
		//cmap_grad.addColor( Color( 50, 50, 255 ) );
		//cmap_grad.addColor( Color( 255, 0, 0 ) );
		//cmap_grad.addColor( Color( 255, 255, 10 ) );

		
		/*cmap_grad.addColor(Color::Blue);
		cmap_grad.addColor(Color::Magenta);
		cmap_grad.addColor(Color::Red);
		cmap_grad.addColor(Color::Yellow);*/
		
		for ( GrayLevelHistogram<src_type>::THistogram::const_iterator bin = h._bin.begin() ; bin != h._bin.end() ; bin ++ ) {
			if ( !Labels.isEmpty() ) {
				if ( !Labels.contains( bin->first ) ) {
					iColor++ ; // do not change color order whenever selection is done or not
					makeRgbColor( hue );
					delete map_obj[ bin->first ] ;
					map_obj[ bin->first ] = 0 ;
					continue ;
				}
			}
			Color cVoxel, cEdgel ;
			if ( single_channelColor ) {
				cVoxel = Color( 0,(iColor*256)/nColor,0, 240 );
				cEdgel = Color( 0,(iColor*256)/nColor,0, 120 );
			} else {
				if ( !vm["colormap"].as<bool>() ) {
					if ( vm["golden"].as<bool>() ) {
						QColor qColor = makeRgbColor( hue ) ;
						cVoxel = Color( qColor.red(), qColor.green(), qColor.blue(), 240 ) ;
						cEdgel = Color( qColor.red(), qColor.green(), qColor.blue(), 120 ) ;
					} else {
						cVoxel = Color((256/stepColor)*(iColor/(stepColor*stepColor)), (256/stepColor)*( (iColor/stepColor) % stepColor ),( 256 / stepColor ) * ( iColor % stepColor) , 240) ;
						cEdgel = Color(cVoxel.red(), cVoxel.green(),cVoxel.blue() , 120) ;						
					}
				} else {
					Color col = cmap_grad( iColor-1) ;
					cVoxel = Color( col.red(),col.green(),col.blue(), 240 );
					cEdgel = Color( col.red(),col.green(),col.blue(), 120 );
				}
			}
			if ( !vm["transparency"].as<bool>() ) {
				cVoxel.alpha(255);
				cEdgel.alpha(255);
			}
			viewer << CustomColors3D( cVoxel, cEdgel );
			viewer << *( map_obj[ bin->first ] );
			
			if ( resultingscene_8 ) {
				if ( size_voxel == 3 )
					for ( Z3i::DigitalSet::ConstIterator pt = map_obj[ bin->first ]->begin() ; pt != map_obj[ bin->first ]->end() ; pt++ ) {
						resultingscene_8->at( pt->at(1), 3*pt->at(0),pt->at(2) ) = cVoxel.red() ;
						resultingscene_8->at( pt->at(1), 3*pt->at(0)+1,pt->at(2) ) = cVoxel.green() ;
						resultingscene_8->at( pt->at(1), 3*pt->at(0)+2,pt->at(2) ) = cVoxel.blue() ;
					}
				else
					for ( Z3i::DigitalSet::ConstIterator pt = map_obj[ bin->first ]->begin() ; pt != map_obj[ bin->first ]->end() ; pt++ )
						resultingscene_8->at( pt->at(1), pt->at(0),pt->at(2) ) = iColor ;
			} else if ( resultingscene_16 ) {
				for ( Z3i::DigitalSet::ConstIterator pt = map_obj[ bin->first ]->begin() ; pt != map_obj[ bin->first ]->end() ; pt++ )
					resultingscene_16->at( pt->at(1), pt->at(0),pt->at(2) ) = iColor ;
			}
			
			if ( display_bbox ) {
				Point lower, upper ;
				map_obj[ bin->first ]->computeBoundingBox( lower, upper ) ;
				viewer << SetMode3D( domain.className(), "BoundingBox") ;
				viewer << Domain( lower, upper ) ;
				if ( vm["verbose"].as<bool>() ) std::cout<<"obj "<<bin->first<<" size "<<bin->second<<"(reduced to "<<map_obj[ bin->first ]->size()<<" boundary voxels) color rgb("<<(iColor*stepColor)/(256*256)<<";"<<(iColor*stepColor/256)%256<<";"<<(iColor*stepColor)%256<<")";
				if ( vm["verbose"].as<bool>() ) std::cout<<" bb ["<<lower.at(0)<<","<<lower.at(1)<<","<<lower.at(2)<<" -- "<<upper.at(0)<<","<<upper.at(1)<<","<<upper.at(2)<<"]"<<std::endl;
			}
			iColor++ ;
			delete map_obj[ bin->first ] ;
			map_obj[ bin->first ] = 0 ;
		}
		std::cout<< h._bin.begin()->first<<" to "<<h._bin.rbegin()->first<<std::endl;
	} else {
		Color cVoxel, cEdgel ;
		cVoxel = Color( 128,128,0,240 );
		cEdgel = Color( 128,128,0,120 );
		if ( !vm["transparency"].as<bool>() ) {
			cVoxel.alpha(255);
			cEdgel.alpha(255);
		}
		viewer<<SetMode3D( p.className(), "Paving" ) ;
		viewer << CustomColors3D( cVoxel, cEdgel );
		unsigned int nbVoxel = 0 ;
		KSpace K ;
		for ( int z = zmin ; z < zmax ; z++ )
			for ( int y = ymin ; y < ymax ; y++ )
				for ( int x = xmin ; x < xmax ; x++ )
					if ( img->at(y,x,z) != 0 ) {
						src_type value = img->at(y,x,z) ;
						if ( !Labels.isEmpty() ) {
							if ( !Labels.contains( value ) ) {
								continue ;
							}
						}
						if ( boundary_only ) {
							/// insert this voxel iff at least one neighbor is 0
							if ( display_voxel && ( (img->at(y,max(0,x-1),z) ==0 || img->at(y, min(width-1,x+1),z) ==0 || 
								img->at(max(0,y-1),x,z) ==0 || img->at(min(height-1,y+1),x,z) ==0 || 
								img->at(y,x,max(0,z-1)) ==0 || img->at(y,x,min(depth-1,z+1)) ==0 ) ) ) {
								viewer << Point(x,y,z) ;nbVoxel++;
							} else if ( !display_voxel ) {
								if (img->at(y,max(0,x-1),z) ==0 )
									viewer << Z3i::Cell( Point( 2*x, 2*y+1, 2*z+1)/*, true*/ ) ;
								if ( img->at(y, min(width-1,x+1),z) ==0 )
									viewer << Z3i::Cell( Point( 2*x+2, 2*y+1, 2*z+1)/*, false*/ ) ;
								if ( img->at(max(0,y-1),x,z) ==0 )
									viewer << Z3i::Cell( Point( 2*x+1, 2*y, 2*z+1)/*, true*/ ) ;
								if ( img->at(min(height-1,y+1),x,z) ==0 )
									viewer << Z3i::Cell( Point( 2*x+1, 2*y+2, 2*z+1)/*, false*/ ) ;
								if ( img->at(y,x,max(0,z-1)) ==0 )
									viewer << Z3i::Cell( Point( 2*x+1, 2*y+1, 2*z)/*, true*/ ) ;
								if ( img->at(y,x,min(depth-1,z+1)) ==0 )
									viewer << Z3i::Cell( Point( 2*x+1, 2*y+1, 2*z+2)/*, false*/ ) ;
							}
						} else {
							viewer << Point(x,y,z) ;nbVoxel++;
						}
						if ( resultingscene_8 ) {
							if ( Labels.isEmpty() )
								resultingscene_8->at( y, x,z ) = value ;
							else
								resultingscene_8->at( y, x,z ) = Labels.indexOf( value )+1 ;
						} else if ( resultingscene_16 ) {
							if ( Labels.isEmpty() )
								resultingscene_16->at( y, x,z ) = value ;
							else
								resultingscene_16->at( y, x,z ) = Labels.indexOf( value )+1 ;
						}							
					}
		std::cout<<"Display "<<nbVoxel<<" voxel(s)"<<std::endl;
	}
	if ( clipping ) {
		for ( int iClipping = 0 ; iClipping < nClipping ; iClipping++ )
			viewer << ClippingPlane( param_clipping[iClipping*4+0], param_clipping[iClipping*4+1], param_clipping[iClipping*4+2], param_clipping[iClipping*4+3] ) ;
		
		delete [] param_clipping ;
	}
	viewer << Viewer3D::updateDisplay;
	
	if ( resultingscene_8 ) {
		std::cout<<"(8 bits) min-max while exporting : "<<(arma::s32)resultingscene_8->min()<<" "<<(arma::s32)resultingscene_8->max()<<std::endl;
		IOPgm3d< arma::u8, qint8, false>::write( *resultingscene_8, QString("%1").arg( vm["export"].as< std::string >().c_str() ) ) ;
		delete resultingscene_8 ;
	} else if ( resultingscene_16 ) {
		std::cout<<"(16 bits) min-max while exporting : "<<(arma::s32)resultingscene_16->min()<<" "<<(arma::s32)resultingscene_16->max()<<std::endl;
		IOPgm3d< arma::u16, qint16, false>::write( *resultingscene_16, QString("%1").arg( vm["export"].as< std::string >().c_str() ) ) ;
		delete resultingscene_16 ;
	}
	
	return application.exec();
}
