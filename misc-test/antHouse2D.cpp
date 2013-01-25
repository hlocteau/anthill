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

#include <writerBinaryPgm3D.h>

using namespace DGtal ;

typedef GrayscaleColorMap<unsigned char> 					GrayColorMap;
typedef ImageSelector< Z2i::Domain, unsigned char>::Type 	GrayImage;

typedef ImageSelector< Z3i::Domain, unsigned char>::Type 	Gray3DImage;

typedef std::list< Z2i::Point > 							VertexPolygon ;
typedef StraightLineFrom2Points<Z2i::Point> 				EdgeType ;
typedef std::vector< EdgeType > 							EdgePolygon ;

Z2i::DigitalSet* newDigitalSet2DFromBinaryImage( const GrayImage &image ) {
	Z2i::DigitalSet * objpts = new Z2i::DigitalSet( image.domain() ) ;
	for ( Z2i::Domain::ConstIterator pt = image.domain().begin() ; pt != image.domain().end() ; pt++ )
	{
		if ( image( *pt ) == (unsigned char) 255 && 
					(*pt).at(0) > 96 && (*pt).at(0) < 390  /** 3 * image.domain().upperBound().at(0)*/ &&
					(*pt).at(1) > 112 && (*pt).at(1) < 416  /** 3 * image.domain().upperBound().at(1)*/ )
			objpts->insertNew( *pt ) ;
	}
	return objpts ;
}

void init_edges_polygon( EdgePolygon &edges, const VertexPolygon &vertices ) {
	VertexPolygon::const_iterator	vertex_begin = vertices.begin(),
									vertex_end ;
	for ( uint iSeg = 0 ; iSeg < vertices.size() ; iSeg++ )
	{
		vertex_end = vertex_begin ;
		vertex_end++ ;
		if ( vertex_end == vertices.end() ) vertex_end = vertices.begin() ;
		edges.push_back( EdgeType( *vertex_begin, *vertex_end ) );
		vertex_begin++ ;
	}
}

void process_slice( const boost::filesystem::path &folderPath, uint z, Z3i::DigitalSet &set3d ) {
	boost::filesystem::path filePath = folderPath ;
	filePath /= QString("slice-%1.pgm").arg( z, 0, 10 ).toStdString() ;
	
	GrayImage image = PNMReader<GrayImage>::importPGM( filePath.string() );
	
	Z2i::DigitalSet *objpts = newDigitalSet2DFromBinaryImage( image ) ;
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
				if ( image( *pt ) == (unsigned char) 255 ) continue ;
				bool bInside = true ;
				for ( uint iSeg = 0 ; iSeg != segs.size() && bInside; iSeg++ )
					if ( segs[iSeg].signedDistance( *pt ) < 0 ) bInside = false ;
				if ( !bInside ) continue ;
				set3d.insertNew( Z3i::Point( (*pt).at(0),(*pt).at(1), z ) ) ;
				new3Dpts++ ;
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
	trace.beginBlock("Reading slices.");
	Z3i::DigitalSet set3d( domain ) ;
	
	SetPredicate<Z3i::DigitalSet> set3dPredicate( set3d ) ;
	for ( uint depth = 0 ; depth < domain.upperBound().at(2) ; depth++ )
		process_slice( argv[1], depth, set3d ) ;
	trace.endBlock() ;

	if ( 0) {
		try {
			Gray3DImage aImage( domain );
			for ( Z3i::Domain::ConstIterator pt = domain.begin() ; pt != domain.end() ; pt++ )
				aImage.setValue( (*pt), 255 ) ;
			boost::filesystem::path pathvolname = argv[1] ;
			pathvolname /= "obj.vol" ;
			if ( ! DGtal::VolWriter< Gray3DImage, GrayColorMap >::exportVol ( pathvolname.string(), aImage, 0, 255 ) )
			{
				trace.error() <<"Can not export data (generated 3D object)"<<std::endl;
				return -4 ;
			} else {
				trace.info()<<"export data to "<<pathvolname.string()<<std::endl;
			}
			pathvolname = argv[1] ;
			pathvolname /= "obj.pgm3d" ;
			if ( ! PNMWriter<Gray3DImage, GrayColorMap >::exportPGM3D( pathvolname.string(), aImage, 0, 255 ) )
			{
				trace.error() <<"Can not export data (generated 3D object)"<<std::endl;
				return -4 ;
			} else {
				trace.info()<<"export data to "<<pathvolname.string()<<std::endl;
			}
			
		} catch ( std::exception &e ) {
			trace.error() <<"Can not export data (generated 3D object) "<<e.what()<<std::endl;
			return -8 ;
		}
	}

	trace.beginBlock( "Decompose the object into connected components." );
	{
		DGtal::Z3i::Object6_18 scene( DGtal::Z3i::dt6_18, set3d ) ;
		std::vector< Z3i::Object6_18 > v_obj ;
		back_insert_iterator< std::vector< Z3i::Object6_18 > > it( v_obj ) ;
		scene.writeComponents( it ) ;
		boost::filesystem::path scenefilename = argv[1] ;
		scenefilename /= "scene.pgm3d" ;
		IOPgm3d::write( v_obj, scenefilename.string().c_str() ) ;
	}
	trace.endBlock( );

	return -64;
	
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
}
