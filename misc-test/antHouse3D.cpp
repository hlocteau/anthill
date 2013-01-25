#include <DGtal/helpers/StdDefs.h>
#include <DGtal/io/readers/PNMReader.h>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/io/writers/PNMWriter.h>
#include <DGtal/io/colormaps/GrayscaleColorMap.h>
#include <DGtal/shapes/fromPoints/StraightLineFrom2Points.h>

#include <DGtal/kernel/sets/SetPredicate.h>
#include <geom2d.h>

#include <boost/filesystem.hpp>
#include <QString>

#include <QtGui/qapplication.h>
#include <DGtal/io/viewers/Viewer3D.h>
#include <DGtal/io/boards/Board2D.h>

#include <DGtal/io/writers/VolWriter.h>

#include <def_billon.h>
#include <billon.h>
#include <connexcomponentextractor.h>
#include <io/Pgm3dFactory.h>
#include <utils.h>
using namespace DGtal ;

typedef GrayscaleColorMap<unsigned char> 					GrayColorMap;
typedef ImageSelector< Z2i::Domain, unsigned char>::Type 	GrayImage;

typedef ImageSelector< Z3i::Domain, unsigned char>::Type 	Gray3DImage;


Z3i::DigitalSet * newDigitalSet3DFromValue( const Z3i::Domain &domain, const GrayCube *labels, uint current_label ) {
	Z3i::DigitalSet *pts = new Z3i::DigitalSet( domain ) ;
	
	for ( uint k = 0 ; k < labels->n_slices ; k++ )
		for ( uint j = 0 ; j < labels->n_cols ; j++ )
			for ( uint i = 0 ; i < labels->n_rows ; i++ )
				if ( labels->at( i, j, k ) == current_label )
					pts->insertNew( Z3i::Point( i, j, k ) ) ;
	//trace.info()<<"cc #"<<current_label<<" has "<<pts->size()<<" voxel(s) "<<pts->domain().lowerBound()<<" "<<pts->domain().upperBound()<<std::endl;
	return pts ;
}

Z2i::DigitalSet* newDigitalSet2DFromBinaryImage( const GrayImage &image, const GrayCube &mask ) {
	Z2i::DigitalSet * objpts = new Z2i::DigitalSet( image.domain() ) ;
	GrayImage im( image.domain() ) ;
	for ( Z2i::Domain::ConstIterator pt = image.domain().begin() ; pt != image.domain().end() ; pt++ )
	{
		if ( image( *pt ) == (unsigned char) 255 && mask( image.domain().upperBound().at(1)-(*pt).at(1), (*pt).at(0),0 ) !=1 ) {
			objpts->insertNew( *pt ) ;
			im.setValue( *pt, 255 ) ;
		}
	}
	#if 0
	PNMWriter<GrayImage, GrayColorMap>::exportPGM( "immask.pgm", im, 0, 255);
	for ( Z2i::Domain::ConstIterator pt = image.domain().begin() ; pt != image.domain().end() ; pt++ )
	{
		if ( mask( image.domain().upperBound().at(1)-(*pt).at(1), (*pt).at(0),0 ) !=1 ) {
			im.setValue( *pt, 255 ) ;
		} else {
			im.setValue( *pt, 0 ) ;
		}
	}
	PNMWriter<GrayImage, GrayColorMap>::exportPGM( "themask.pgm", im, 0, 255);
	#endif
	return objpts ;
}




void push_slice( const boost::filesystem::path &folderPath, uint z, GrayCube &scene, const GrayCube &mask ) {
	boost::filesystem::path filePath = folderPath ;
	filePath /= QString("slice-%1.pgm").arg( z, 0, 10 ).toStdString() ;
	GrayImage * img = io::Pgm3dFactory::read( QString( filePath.string().c_str() ) ) ;
	register uint 
	for ( x = 0 ; x < img->n_cols ; x++ )
		for ( y = 0 ; y < img->n_rows ; y++ )
			if ( img->at( y, x ) > 0 && mask( y, x, 0 ) != 1 )
				scene.at( y, x, z ) = 1 ;
	delete img ;
}

void init_holes( const GrayCube &scene, GrayCube &holes ) {
	GrayCube *labels = ConnexComponentExtractor::extractConnexComponents( scene3d, 1, 0 ) ;
	__GrayCube_type__ lbl ;
	for ( lbl = 1 ; lbl < labels->maxValue() ; lbl++ ) {
		Z3i::Point shift ;
		GrayCube *boundingVolume = Bounding::convexHull2D( *labels, BOUNDING_XY_PLANE, lbl, shift ) ;
		/// reverse
		holes = boundingVolume minus scene wrt shift :
		delete boundingVolume ;
	}
}


void process_slice( const boost::filesystem::path &folderPath, uint z, GrayCube &scene /** Z3i::DigitalSet &set3d*/, const GrayCube &mask, GrayCube *boundingVolume ) {
	boost::filesystem::path filePath = folderPath ;
	filePath /= QString("slice-%1.pgm").arg( z, 0, 10 ).toStdString() ;
	GrayImage image = PNMReader<GrayImage>::importPGM( filePath.string() );
	Z2i::DigitalSet *objpts = newDigitalSet2DFromBinaryImage( image, mask ) ;
	if ( !objpts->empty() ) {
		VertexPolygon convexhull ;
		Z2i::Object4_8 obj( Z2i::dt4_8, *objpts) ;
		Z2i::Object4_8 boundary = obj.border() ;
		if ( boundary.size() <= 3 )
		{
			trace.info()<<"Warning : boundary is ";
			for ( Z2i::DigitalSet::ConstIterator pt = boundary.pointSet().begin() ; pt != boundary.pointSet().end() ; pt++ )
				trace.info()<<(*pt)<<" " ;
			trace.info()<<std::endl;
		}
		Geom2D::ConvexHull( boundary.pointSet(), convexhull ) ;
		EdgePolygon segs ;
		init_edges_polygon( segs, convexhull ) ;
		uint new3Dpts = 0 ;
		if ( segs.size() >= 3 )
			for ( Z2i::Domain::ConstIterator pt = image.domain().begin() ; pt != image.domain().end() ; pt++ )
			{
				if ( image( *pt ) != (unsigned char) 255 ) {
					bool bInside = true ;
					for ( uint iSeg = 0 ; iSeg != segs.size() && bInside; iSeg++ )
						if ( segs[iSeg].signedDistance( *pt ) < 0 ) bInside = false ;
					if ( !bInside ) continue ;
					/** set3d.insertNew( Z3i::Point( (*pt).at(0),(*pt).at(1), z ) ) ; */
					scene.at( (*pt).at(0),(*pt).at(1), z ) = 255 ;
					new3Dpts++ ;
				}
				if ( boundingVolume != 0 ) {
					bool bInside = true ;
					for ( uint iSeg = 0 ; iSeg != segs.size() && bInside; iSeg++ )
						if ( segs[iSeg].signedDistance( *pt ) < 0 ) bInside = false ;
					if ( !bInside ) continue ;
					boundingVolume->at( (*pt).at(0),(*pt).at(1), z ) = 255 ;
				}
			}
		Z2i::Point UL, BR ;
		boundary.pointSet().computeBoundingBox( UL, BR ) ;
		std::cerr<<QString( "Slice %1").arg( z,10,10,QLatin1Char('0')).toStdString() ;
		std::cerr<<setw(6)<<objpts->size()<<" "
					<<setw(6)<<boundary.size()<<" "
					<<setw(6)<<new3Dpts<<" "
					<<setw(4)<<UL.at(0)<<","
					<<setw(4)<<UL.at(1)<<" - "
					<<setw(4)<<BR.at(0)<<","
					<<setw(4)<<BR.at(1)<<std::endl;
	}
	delete objpts ;
}

void extendMaskToImageBoundary( GrayCube & mask ) {
	int width = mask.n_cols,
		height = mask.n_rows,
		depth = mask.n_slices ;
	int minDist[] = { 0,0,0,0 } ;
	int x,y, u ;
	for ( x = 0 ; x < width ; x++ )
		for ( y = 0 ; y < height ; y++ ) {
			if ( mask(x,y,0) == 0 ) continue ;
			minDist[0] = max( minDist[0], x ) ;
			minDist[1] = max( minDist[1], y ) ;
			
			minDist[2] = max( minDist[2], width-x ) ;
			minDist[3] = max( minDist[3], height-y ) ;
		}
	if ( min( minDist[0], minDist[2] ) < min( minDist[1], minDist[3] ) ) {
		/// the support is either at the left or the right side
		if ( minDist[0] < minDist[2] ) {
			/// left side
			for ( y = 0 ; y < height ; y++ )
				for ( x = width-1 ; x >= 0 ; x-- ) {
					if ( mask(x,y,0) == 0 ) continue ;
					for ( u = 0 ; u < x ; u++ )
						mask(u,y,0) = 1 ;
					break ;
				}
		} else {
			/// right side
			for ( y = 0 ; y < height ; y++ )
				for ( x = 0 ; x < width ; x++ ) {
					if ( mask(x,y,0) == 0 ) continue ;
					for ( u = x ; u < width ; u++ )
						mask(u,y,0) = 1 ;
					break ;
				}
		}
	} else {
		/// the support is either at the top or the bottom side
		if ( minDist[1] < minDist[3] ) {
			/// top side
			for ( x = 0 ; x < width ; x++ )
				for ( y = height-1 ; y >= 0 ; y-- ) {
					if ( mask(x,y,0) == 0 ) continue ;
					for ( u = 0 ; u < y ; u++ )
						mask(x,u,0) = 1 ;
					break ;
				}
		} else {
			/// bottom side
			for ( x = 0 ; x < width ; x++ )
				for ( y = 0 ; y < height ; y++ ) {
					if ( mask(x,y,0) == 0 ) continue ;
					for ( u = x ; u < height ; u++ )
						mask(x,u,0) = 1 ;
					break ;
				}
		}
		
	}
	mask.slice(1) = mask.slice(0);
}

/// identify the mask we may apply on images to remove the support
void identify_scanning_support( const boost::filesystem::path &folderPath, GrayCube & mask, const Z3i::Domain &domain ) {
	int width  = domain.upperBound().at(0)+1,
		height = domain.upperBound().at(1)+1,
		slice_number = domain.upperBound().at(2) ;
	
	
	mask = GrayCube( width, height, 2 ) ;
	mask.setMaxValue(255);
	mask.slice(0).fill( 1 ) ;
	mask.slice(1).fill( 0 ) ;

	int xmax = width,
		ymax = height,
		xmin = 0,
		ymin = 0,
		lxmin, lymin, lxmax, lymax ;
	register int x, y ;
	boost::filesystem::path filePath;
	for ( uint slice_position = 0 ; slice_position < slice_number ; slice_position++ ) {
		filePath = folderPath ;
		filePath /= QString("slice-%1.pgm").arg( slice_position, 0, 10 ).toStdString() ;
		GrayCube *image = io::Pgm3dFactory::read( QString( filePath.string().c_str() ) );
		*image *= -1 ;
		//std::cout<<"Slice "<<setw(3)<<slice_position<<" bb "<<setw(3)<<xmin<<":"<<setw(3)<<ymin<<" to "<<setw(3)<<xmax<<":"<<setw(3)<<ymax<<" Sum "<<arma::accu(*image)<<std::endl;
#ifdef BINARY_MASK_IDENTIFICATION
		for ( y = ymin ; y < ymax ; y++ )
			for ( x = xmin ; x < xmax ; x++ ) {
				if ( (*image)( x,y,0 ) == 0 )
					mask( x, y,0 ) = 0 ;
			}
		delete image ;
		lxmin = xmax ; lxmax = xmin ;
		lymin = ymax ; lymax = ymin ;
		for ( y = ymin ; y < ymax ; y++ )
			for ( x = xmin ; x < xmax ; x++ )
				if ( mask( x, y,0 ) == 1 ) {
					lxmin = min( lxmin, x ) ;
					lxmax = max( lxmax, x+1 ) ;
					lymin = min( lymin, y ) ;
					lymax = max( lymax, y+1 ) ;
				}


		xmin = lxmin ; xmax = lxmax ; ymax = lymax ; ymin = lymin ;
		if ( xmin >= xmax || ymin >= ymax ) break ;
#else
		for ( y = ymin ; y < ymax ; y++ )
			for ( x = xmin ; x < xmax ; x++ )
				mask( x, y,0 ) += (*image)( x,y,0 ) ;
#endif
	}
#ifndef BINARY_MASK_IDENTIFICATION
		for ( y = ymin ; y < ymax ; y++ )
			for ( x = xmin ; x < xmax ; x++ )
				mask( x, y,0 ) = ( mask( x, y,0 ) > (slice_number*70)/100? 1 : 0 ) ;
#endif
	register int x_margin, y_margin ;
	for ( x = xmin ; x < xmax ; x++ )
		for ( y = ymin ; y < ymax; y++ )
			if ( mask (x,y,0) == 1 ) {
				mask(x, y, 1 ) = 1 ;
				for ( x_margin = 1 ; x_margin < 4 ; x_margin++ )
					for ( y_margin = 1 ; y_margin < 4 ; y_margin++ ) {
						if ( x >= x_margin ) {
							mask.at(x-x_margin, y, 1 ) = 1 ;
							if ( y >= y_margin )		mask.at(x-x_margin, y-y_margin, 1 ) = 1 ;
							if ( y <= height-y_margin )	mask.at(x-x_margin, y+y_margin, 1 ) = 1 ;
						}
						if ( x <= width-x_margin ) {
							mask.at(x+x_margin, y, 1 ) = 1 ;
							if ( y >= y_margin )		mask.at(x+x_margin, y-y_margin, 1 ) = 1 ;
							if ( y <= height-y_margin )	mask.at(x+x_margin, y+y_margin, 1 ) = 1 ;
						}
						if ( y >= y_margin ) 			mask.at(x, y-y_margin, 1 ) = 1 ;
						if ( y <= height-y_margin )		mask.at(x, y+y_margin, 1 ) = 1 ;
					}
			}
	mask.slice(0) = mask.slice(1) ;
	
	io::IOPgm3d< qint32, false >::write( mask, QString("maskbefore.pgm") ) ;
	extendMaskToImageBoundary( mask ) ;
	
	io::IOPgm3d< qint32, false >::write( mask, QString("mask.pgm") ) ;

}


Z3i::Domain scene_dimensions( const boost::filesystem::path &folderPath ) {
	uint depth = 0 ;
	boost::filesystem::path filePath;
	do {
		filePath = folderPath ;
		filePath /= QString("slice-%1.pgm").arg( depth, 0, 10 ).toStdString() ;
		depth++ ;
	} while ( boost::filesystem::exists( filePath ) ) ;
	depth-- ;
	if ( depth == 0 ) return Z3i::Domain( Z3i::Point(0,0,0), Z3i::Point(0,0,0) ) ;
	filePath = folderPath ;
	filePath /= QString("slice-%1.pgm").arg( depth/2, 0, 10 ).toStdString() ; /** "slice-0.pgm" ;*/
	GrayImage image = PNMReader<GrayImage>::importPGM( filePath.string() );
	#if 0
	Z2i::DigitalSet *pts = newDigitalSet2DFromBinaryImage( image ) ;
	Board2D board ;
	board << image.domain()<<*pts ;
	board.saveSVG( "cropcheck.svg");
	trace.info()<<"Get "<<pts->size()<<" points on slide "<<filePath.string()<<std::endl;
	delete pts ;
	#endif
	
	return Z3i::Domain( Z3i::Point(0,0,0), Z3i::Point( image.domain().upperBound().at(0),image.domain().upperBound().at(1),depth) ) ;
}

int main( int narg, char **argv )
{
	trace.beginBlock("Identifying the domain.");
	Z3i::Domain domain = scene_dimensions( argv[1] ) ;
	if ( domain.lowerBound() == domain.upperBound() ) {
		trace.error()<<"Invalid domain ("<<__FILE__<<")"<<std::endl;
		return -1 ;
	}
	trace.endBlock() ;
	
	trace.beginBlock("Identifying the mask.");
	GrayCube mask ;
	identify_scanning_support( argv[1], mask, domain ) ;
	trace.info()<<"mask's size is "<<mask.n_cols<<" "<<mask.n_rows<<" "<<mask.n_slices<<std::endl;
	trace.endBlock() ;
	
	trace.beginBlock("Reading slices.");
	
	/**
	Z3i::DigitalSet set3d( domain ) ;
	SetPredicate<Z3i::DigitalSet> set3dPredicate( set3d ) ;
	*/
	GrayCube scene3d( domain.upperBound().at(0)+1, domain.upperBound().at(1)+1, domain.upperBound().at(2)+1 );
	GrayCube *boundingVolume = new GrayCube( domain.upperBound().at(0)+1, domain.upperBound().at(1)+1, domain.upperBound().at(2)+1 );
	
	for ( uint depth = 0 ; depth < domain.upperBound().at(2) ; depth++ )
		process_slice( argv[1], depth, /** set3d*/ scene3d, mask, boundingVolume ) ;
	io::IOPgm3d< qint8, false>::write( *boundingVolume, "bounding.pgm" ) ;
	delete boundingVolume ;
	//exit(-1);
	//scene3d /= 255 ;
	//io::IOPgm3d< qint32,false>::write( scene3d, QString("oneslice.pgm") ) ;
	
	trace.endBlock() ;



	trace.beginBlock( "Decompose the object into connected components." );
	std::vector< Z3i::Object6_26 * > v_obj ;
	//IOPgm3d<qint32,false> ioPGM;
	{

		boost::filesystem::path scenefilename = argv[1] ;
		scenefilename /= "scene.pgm3d" ;

		trace.info()<<"identifying the label of voxels ..."<<std::endl;
		GrayCube *labels = ConnexComponentExtractor::extractConnexComponents( scene3d, 1, 0 ) ;
		trace.info()<<"Cube's dimension is "<<labels->n_rows<<" "<<labels->n_cols<<" "<<labels->n_slices<<std::endl;
		trace.info()<<"build one object per connected component ..."<<std::endl;
		/*
		Z3i::DigitalSet *pts;
		for ( uint current_label = 1 ; current_label < labels->maxValue() ; current_label++ ) {
			pts = newDigitalSet3DFromValue( domain, labels, current_label ) ;
			v_obj.push_back( new DGtal::Z3i::Object6_26( DGtal::Z3i::dt6_26, domain ) ) ;
			v_obj[ current_label-1 ]->pointSet().insert( pts->begin(), pts->end() ) ;
			delete pts ;
		}
		trace.info()<<"export scene ..."<<std::endl;
		IOPgm3d<qint32>::write( v_obj, scenefilename.string().c_str() ) ;
		*/
		//scene3d /= 255 ;
		io::IOPgm3d< qint8, false>::write( scene3d, scenefilename.string().c_str() ) ;
		
		scenefilename = argv[1] ;
		scenefilename /= "scene_label.pgm3d" ;
		io::IOPgm3d< qint32, false>::write( *labels, scenefilename.string().c_str() ) ;
		//ioPGM.write( *labels, scenefilename.string().c_str() ) ;
	}
	trace.endBlock( );


	trace.beginBlock( "Extract the homotopic skeleton of connected components." );
	std::vector< Z3i::Object6_26* >::iterator cc = v_obj.end() ; // begin()
	while ( cc != v_obj.end() ) {
		int nb_simple=0;
		int layer = 0;
		do {
			Z3i::DigitalSet & S = (*cc)->pointSet();
			std::queue<Z3i::DigitalSet::Iterator> Q;
			for ( Z3i::DigitalSet::Iterator it = S.begin(); it != S.end(); ++it )
				if ( (*cc)->isSimple( *it ) )
					Q.push( it );
			nb_simple = 0;
			while ( ! Q.empty() ) {
				Z3i::DigitalSet::Iterator it = Q.front();
				Q.pop();
				if ( (*cc)->isSimple( *it ) )
				{
					S.erase( *it );
					++nb_simple;
				}
			}
			++layer;
		} while ( nb_simple != 0 );
	}
	trace.info()<<"export skeletons ..."<<std::endl;
	boost::filesystem::path scenefilename = argv[1] ;
	scenefilename /= "skeleton.pgm3d" ;
	//IOPgm3d<qint32>::write( v_obj, scenefilename.string().c_str() ) ;

	trace.endBlock( );
	
	for ( uint iObj = 0 ; iObj < v_obj.size() ; iObj++ )
		delete v_obj[ iObj ] ;
	v_obj.clear() ;

	return -64;
#if 0	
	trace.beginBlock( "Construct the Khalimsky space from the image domain." );
	Z3i::KSpace ks;
	bool space_ok = ks.init( domain.lowerBound(), domain.upperBound(), true );
	if (!space_ok)
	{
		trace.error() << "Error in the Khamisky space construction."<<std::endl;
		return -2;
	}
	trace.endBlock();

	typedef SurfelAdjacency<Z3i::KSpace::dimension> MySurfelAdjacency;
	MySurfelAdjacency surfAdj( true ); // interior in all directions.

	trace.beginBlock( "Extracting boundary by tracking from an initial bel." );
	Z3i::KSpace::SCellSet boundary;
	Z3i::SCell bel = Surfaces<Z3i::KSpace>::findABel( ks, set3dPredicate, 100000 );
	Surfaces<Z3i::KSpace>::trackBoundary( boundary, ks, surfAdj, set3dPredicate, bel );
	trace.endBlock();

	trace.beginBlock( "Displaying surface in Viewer3D." );
	QApplication application(narg,argv);
	Viewer3D viewer;
	viewer.show();
	viewer 	<< SetMode3D( boundary.begin()->className(), "") ; //CustomColors3D(Color(250, 0, 0 ), Color( 128, 128, 128 ) );
	unsigned long nbSurfels = 0;
	for ( Z3i::KSpace::SCellSet::const_iterator it = boundary.begin(), it_end = boundary.end(); it != it_end; ++it, ++nbSurfels )
	
			viewer << ks.uCell( Z3i::Point( (*it).myCoordinates.at(0), (*it).myCoordinates.at(1), (*it).myCoordinates.at(2) ) ) ;
	
	/**
	for ( Z3i::KSpace::SCellSet::const_iterator it = boundary.begin(), it_end = boundary.end(); it != it_end; ++it, ++nbSurfels )
		viewer << (*it) ;
	*/
	trace.info() << *boundary.begin()<<std::endl;
	viewer << Viewer3D::updateDisplay;
	trace.info() << "nb surfels = " << nbSurfels << std::endl;
	trace.endBlock();
	return application.exec();
#endif
}
