/**
 * \brief reconstruct the connected component from the given skeleton and depth map
 */
#include <def_coordinate.h>
#include <coordinate.h>
#include <utils.h>
#include <io/Pgm3dFactory.h>

#include <DGtal/images/ImageContainerBySTLVector.h>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/geometry/volumes/distance/ReverseDistanceTransformation.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <geom2d.h>
namespace po = boost::program_options;

using namespace DGtal ;
using namespace Z3i;
typedef Pgm3dFactory< int32_t > 					IPgm3dFactory ;
typedef ImageSelector< Domain, uint>::Type 			Image ;
typedef ReverseDistanceTransformation< Image, 2 > 	RDT ;
typedef RDT::OutputImage							OutImage ;


#define DEBUG_ENCODING_IMAGE

//#define BUILD_INDIVIDUAL_COMP

void process1( const Image &partialDepthImg, DigitalSet &output ) {
	RDT rdt ;
	rdt.reconstructionAsSet( output, partialDepthImg ) ;
}

void process2( const Image &fullDepthImg, const Image &sample, DigitalSet &output ) {
	/// build partial depth keeping only values whenever sample(x,y,z) > 0
	const Domain & domain = fullDepthImg.domain() ;
	Image partialDepthImg( domain ) ;
	for ( Domain::ConstIterator pt = domain.begin() ; pt != domain.end() ; pt++ ) {
		if ( sample( *pt ) > 0 )
			partialDepthImg.setValue( *pt, fullDepthImg( *pt ) ) ;
	}
	process1( partialDepthImg, output ) ;
}

void process3( const Image &fullDepthImg, const DigitalSet &sample, DigitalSet &output ) {
	/// build partial depth keeping only values whenever sample(x,y,z) > 0
	const Domain & domain = fullDepthImg.domain() ;
	Image partialDepthImg( domain ) ;
	for ( DigitalSet::ConstIterator pt = sample.begin() ; pt != sample.end() ; pt++ ) {
		partialDepthImg.setValue( *pt, fullDepthImg( *pt ) ) ;
	}
	process1( partialDepthImg, output ) ;
}

void process4( const Image &partialDepthImg, OutImage & output ) {
	RDT rdt ;
	output = rdt.reconstruction( partialDepthImg ) ;
}

void process5( const Image &fullDepthImg, const DigitalSet &sample, OutImage & output ) {
	const Domain & domain = fullDepthImg.domain() ;
	Image partialDepthImg( domain ) ;
	for ( DigitalSet::ConstIterator pt = sample.begin() ; pt != sample.end() ; pt++ ) {
		partialDepthImg.setValue( *pt, fullDepthImg( *pt ) ) ;
	}
	process4( partialDepthImg, output ) ;
}

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Rebuild labelled regions from a labelled skeleton and a depth map."<<std::endl
				<< general_opt << "\n";
}
void missingParam ( std::string param )
{
  /*trace.error()*/std::cerr <<" Parameter: "<<param<<" is required.."<<std::endl;
  exit ( 1 );
}

template<typename T> std::pair<Z3i::DigitalSet**,Z3i::Domain**>  labelimage_as_voxelsLayer( const BillonTpl< T > *pimg ) {
	QList< iCoord3D > *pts = new QList< iCoord3D > [ pimg->maxValue()+1 ] ;
	Z3i::Point *lower = new Z3i::Point [ pimg->maxValue()+1 ] ;
	Z3i::Point *upper = new Z3i::Point [ pimg->maxValue()+1 ] ;
	register int x,y,z ;
	uint32_t label ;
	
	for ( label = 1 ; label <= pimg->maxValue() ; label++ ) {
		lower[ label ] = Z3i::Point ( pimg->n_cols, pimg->n_rows, pimg->n_slices ) ;
		upper[ label ] = Z3i::Point ( 0, 0, 0 ) ;
	}
		
	for ( z = 0 ; z < pimg->n_slices ; z++ )
		for ( x = 0 ; x < pimg->n_cols ; x++ )
			for ( y = 0 ; y < pimg->n_rows ; y++ ) {
				if ( (*pimg)(y,x,z) == 0 ) continue ;
				label = (*pimg)(y,x,z) ;
				pts[ label ].append( iCoord3D(x,y,z) ) ;
				lower[ label ].at(0) = std::min( lower[ label ].at(0), x ) ;
				lower[ label ].at(1) = std::min( lower[ label ].at(1), y ) ;
				lower[ label ].at(2) = std::min( lower[ label ].at(2), z ) ;
				upper[ label ].at(0) = std::max( upper[ label ].at(0), x ) ;
				upper[ label ].at(1) = std::max( upper[ label ].at(1), y ) ;
				upper[ label ].at(2) = std::max( upper[ label ].at(2), z ) ;
			}
	
	Z3i::Domain ** pDomains = new Z3i::Domain* [ pimg->maxValue()+1 ] ;
	Z3i::DigitalSet ** pDigitalSets = new Z3i::DigitalSet* [ pimg->maxValue()+1 ] ;
	for ( label = 1 ; label <= pimg->maxValue() ; label++ ) {
		pDomains[ label ] = (Z3i::Domain*) 0 ;
		if ( upper[label] < lower[label] ) continue ;
		pDomains[ label ] 		= new Z3i::Domain( lower[label], upper[label] ) ;
		pDigitalSets[ label ]	= new Z3i::DigitalSet( *pDomains[ label ] ) ;
		while ( !pts[ label ].empty() ) {
			iCoord3D aPoint = pts[ label ].takeAt(0);
			pDigitalSets[ label ]->insertNew( Z3i::Point( aPoint.x,aPoint.y, aPoint.z ) ) ;
		}
	}
	delete [] lower ;
	delete [] upper ;
	return std::pair< Z3i::DigitalSet**,Z3i::Domain**>( pDigitalSets, pDomains ) ;
}

template <typename T, typename U> void rebuildcc_indomain( const BillonTpl<T> & Distances, const Z3i::DigitalSet & layer_pts, uint color, BillonTpl<U> &componentImg, Z3i::Point **boundary ) {
	Z3i::Point 	lower=layer_pts.domain().lowerBound(),
				upper=layer_pts.domain().upperBound() ;
	for ( Z3i::DigitalSet::ConstIterator pt = layer_pts.begin() ; pt != layer_pts.end(); pt++ ) {
		lower.at(0) = std::min ( pt->at(0) - Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) , lower.at(0) ) ;
		lower.at(1) = std::min ( pt->at(1) - Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) , lower.at(1) ) ;
		lower.at(2) = std::min ( pt->at(2) - Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) , lower.at(2) ) ;
		upper.at(0) = std::max ( pt->at(0) + Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) , upper.at(0) ) ;
		upper.at(1) = std::max ( pt->at(1) + Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) , upper.at(1) ) ;
		upper.at(2) = std::max ( pt->at(2) + Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) , upper.at(2) ) ;
	}
	Z3i::Domain domain( Z3i::Point(0,0,0), upper-lower) ;
	Z3i::DigitalSet skeletonSet(domain),
					componentSet(domain) ;
	Image 		distances( domain ) ;
	for ( Z3i::DigitalSet::ConstIterator pt = layer_pts.begin() ; pt != layer_pts.end(); pt++ ) {
		distances.setValue( *pt-lower, Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) ) ;
		skeletonSet.insertNew( *pt - lower ) ;
	}
	trace.info()<<"input "<<skeletonSet.size()<<" voxels"<<std::endl;
#ifdef USE_DIGITAL_SET_FOR_RECONSTRUCTION
	process3( distances, skeletonSet, componentSet ) ;
	for ( Z3i::DigitalSet::ConstIterator pt = componentSet.begin() ; pt != componentSet.end() ; pt++ )
		componentImg( (*pt).at(1)+lower.at(1),(*pt).at(0)+lower.at(0),(*pt).at(2)+lower.at(2) ) = color ;
	trace.info() <<"output (A) "<<componentSet.size()<<" voxels"<<std::endl;
#else
	OutImage componentDGImg( domain ) ;
	
#if 1
arma::icube thisCC( domain.upperBound().at(1), domain.upperBound().at(0), domain.upperBound().at(2) ) ;
thisCC.fill(0);
#endif
	process4( distances, componentDGImg ) ;
	uint nvoxels = 0 ;
	bool bDebugThisLoop = false && ( color== 6 ) ;
for ( int iBound = 0 ; iBound < 4 ; iBound++ )
if ( boundary[iBound] ) std::cerr<<(*boundary[iBound]).at(0)<<","<<(*boundary[iBound]).at(1)<<","<<(*boundary[iBound]).at(2)<<" " ;
else std::cerr<<" none " ;
std::cerr<<std::endl;
	
	for ( Domain::ConstIterator pt = domain.begin() ; pt != domain.end(); pt++ ) {
		if ( componentDGImg( *pt ) > 0 ) {
			bool 	right_inside = true,
					left_inside = true ;
			if ( boundary[0] != 0 ) {
				Z3i::Point dep1 = *pt + lower - *boundary[1] ;
				Z3i::Point::Component d1 = dep1.dot( dep1 ) ;
				Z3i::Point dep2 = *pt + lower - *boundary[0] ;
				Z3i::Point::Component d2 = dep2.dot( dep2 ) ;
				right_inside = ( d1 < d2 ) ;
				if ( bDebugThisLoop ) std::cerr<<"Voxel "<<*pt + lower<<" has"<<(right_inside?" not ":" " )<<"smallest distance to "<<*boundary[0]<<" than to "<<*boundary[1]<<std::endl;
			}
			if ( right_inside ) {
				if ( boundary[1+2] != 0 ) {
					Z3i::Point dep1 = *pt + lower - *boundary[0+2] ;
					Z3i::Point::Component d1 = dep1.dot( dep1 ) ;
					Z3i::Point dep2 = *pt + lower - *boundary[1+2] ;
					Z3i::Point::Component d2 = dep2.dot( dep2 ) ;
					left_inside = ( d1 < d2 ) ;
					if ( bDebugThisLoop ) std::cerr<<"Voxel "<<*pt + lower<<" has"<<(left_inside?" not ":" " )<<"smallest distance to "<<*boundary[0+2]<<" than to "<<*boundary[1+2]<<std::endl;
				}
			}
			if ( right_inside && left_inside ) {
				componentImg( (*pt).at(1)+lower.at(1),(*pt).at(0)+lower.at(0),(*pt).at(2)+lower.at(2) ) = color ;
				#if 1
				thisCC( (*pt).at(1), (*pt).at(0),(*pt).at(2) ) = 1 ;
				#endif
				nvoxels++ ;
			}
		}
	}
#if 1
IOPgm3d<int,qint8,false>::write( thisCC, QString( "/tmp/cc%1.pgm3d" ).arg( color ) ) ;
std::cerr<<"CC "<<color<<" : "<<accu( thisCC )<<std::endl;
#endif
	trace.info() <<"output (B) "<<nvoxels<<" voxels"<<std::endl;
#endif
	if ( bDebugThisLoop ) exit(-1);
}

#define Y_ADJ( voxel, neighbor ) (voxel).at(1) + (neighbor/9-1)
#define X_ADJ( voxel, neighbor ) (voxel).at(0) + ( (neighbor%9)/3 -1 )
#define Z_ADJ( voxel, neighbor ) (voxel).at(2) + ( neighbor % 3 -1 )


template <typename T> void set_adjacency( 	BillonTpl< T > &im, Z3i::DigitalSet **pts, 
											QMap< T, QSet<T> > &relations, QMap< T, QList<std::pair< Z3i::Point,Z3i::Point > > > &locations ) {
	uint32_t nLabels = im.maxValue() ;
	uint32_t adj_label ;
	Z3i::Point dep, curDep ;
	int x, y, z,n ;
	for ( uint32_t label = 1 ; label < nLabels ; label++ ) {
		for ( Z3i::DigitalSet::Iterator pt = pts[ label ]->begin() ; pt != pts[ label ]->end() ; pt++ )
			for ( n = 0 ; n < 27 ; n++ ) {
				if ( n == 13 ) continue ; /// this is *pt
				x = X_ADJ( *pt, n ) ;
				if ( x < 0 || x == im.n_cols ) continue ;
				y = Y_ADJ( *pt, n ) ;
				if ( y < 0 || y == im.n_rows ) continue ;
				z = Z_ADJ( *pt, n ) ;
				if ( z < 0 || z == im.n_slices ) continue ;
				adj_label = im(y,x,z) ;
				if ( adj_label <= label ) continue ;
				T key = label * nLabels + adj_label ;
				
				if ( !relations.contains( label ) ) relations.insert( label, QSet<T>() ) ;
				if ( !relations[ label ].contains( adj_label ) ) {
					relations[ label ].insert( adj_label ) ;
					locations.insert( key, QList< std::pair< Z3i::Point, Z3i::Point > > () ) ;
				}
				locations[ key ].append( std::pair<Z3i::Point,Z3i::Point>( *pt, Z3i::Point(x,y,z) ) ) ;
				if ( !relations.contains( adj_label ) ) relations.insert( adj_label, QSet<T>() ) ;
				if ( !relations[ adj_label ].contains( label ) ) {
					relations[ adj_label ].insert( label ) ;
				}
			}
	}
}

template <typename T> const T & get_first( const std::pair< T, T > & value ) {
	return value.first ;
}
template <typename T> const T & get_second( const std::pair< T, T > & value ) {
	return value.second ;
}

uint32_t n_can_not_seg = 0 ;

template <typename T, typename U, typename V> void rebuildcc_indomain( 	const BillonTpl<T> & Distances, const Z3i::DigitalSet & layer_pts, 
																		U color, BillonTpl<U> &componentImg, 
																		V node, const QMap< V, QSet<V> > &relations, 
																		const QMap< V, QList<std::pair< Z3i::Point,Z3i::Point > > > &locations, 
																		size_t nLabels ) {
	Z3i::Point 	lower=layer_pts.domain().lowerBound(),
				upper=layer_pts.domain().upperBound() ;
	for ( Z3i::DigitalSet::ConstIterator pt = layer_pts.begin() ; pt != layer_pts.end(); pt++ ) {
		lower.at(0) = std::min ( pt->at(0) - Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) , lower.at(0) ) ;
		lower.at(1) = std::min ( pt->at(1) - Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) , lower.at(1) ) ;
		lower.at(2) = std::min ( pt->at(2) - Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) , lower.at(2) ) ;
		upper.at(0) = std::max ( pt->at(0) + Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) , upper.at(0) ) ;
		upper.at(1) = std::max ( pt->at(1) + Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) , upper.at(1) ) ;
		upper.at(2) = std::max ( pt->at(2) + Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) , upper.at(2) ) ;
	}
	Z3i::Domain domain( Z3i::Point(0,0,0), upper-lower) ;
	Z3i::DigitalSet skeletonSet(domain),
					componentSet(domain) ;
	Image 		distances( domain ) ;
	for ( Z3i::DigitalSet::ConstIterator pt = layer_pts.begin() ; pt != layer_pts.end(); pt++ ) {
		distances.setValue( *pt-lower, Distances( (*pt).at(1),(*pt).at(0),(*pt).at(2) ) ) ;
		skeletonSet.insertNew( *pt - lower ) ;
	}
	trace.info()<<"input "<<skeletonSet.size()<<" voxels"<<std::endl;
#if BUILD_INDIVIDUAL_COMP
arma::icube thisCC( domain.upperBound().at(1), domain.upperBound().at(0), domain.upperBound().at(2) ) ;
thisCC.fill(0);
#endif

#ifdef USE_DIGITAL_SET_FOR_RECONSTRUCTION
	process3( distances, skeletonSet, componentSet ) ;
	for ( Z3i::DigitalSet::ConstIterator pt = componentSet.begin() ; pt != componentSet.end() ; pt++ )
		componentImg( (*pt).at(1)+lower.at(1),(*pt).at(0)+lower.at(0),(*pt).at(2)+lower.at(2) ) = color ;
	trace.info() <<"output (A) "<<componentSet.size()<<" voxels"<<std::endl;
#else
	OutImage componentDGImg( domain ) ;
	process4( distances, componentDGImg ) ;
#endif

	uint nvoxels = 0 ;
	bool bDebugThisLoop = false && ( color== 6 ) ;
	
	for ( typename QSet<T>::ConstIterator adj_node = relations[ node ].begin() ; adj_node != relations[ node ].end() ; adj_node++ ) {
		T key = std::min( *adj_node, node ) * nLabels + std::max( *adj_node, node ) ;
		std::cerr<<node<<"|"<<*adj_node<<" : ";
		for ( QList< std::pair< Z3i::Point, Z3i::Point > >::ConstIterator loc = locations[ key ].begin() ; loc != locations[ key ].end() ; loc++ )
			std::cerr<<(*loc).first.at(0)<<","<<(*loc).first.at(1)<<","<<(*loc).first.at(2)<<" "
					<<(*loc).second.at(0)<<","<<(*loc).second.at(1)<<","<<(*loc).second.at(2)<<" ";
		std::cerr<<std::endl;
	}
	const Z3i::Point & (*me)( const std::pair< Z3i::Point, Z3i::Point > &) ;
	const Z3i::Point & (*other)( const std::pair< Z3i::Point, Z3i::Point > &) ;
	Z3i::Point relative_loc_me, relative_loc_other ;
	Z3i::Point::Component squared_distance_me, squared_distance_other ;
	for ( Domain::ConstIterator pt = domain.begin() ; pt != domain.end(); pt++ ) {
		if ( componentDGImg( *pt ) > 0 ) {
			bool 	bInside = true ;
			for ( typename QSet<T>::ConstIterator adj_node = relations[ node ].begin() ; bInside && adj_node != relations[ node ].end() ; adj_node++ ) {
				if ( node < *adj_node ) {
					me = &get_first ;
					other = &get_second ;
				} else {
					other = &get_first ;
					me = &get_second ;					
				}
				T key = std::min( *adj_node, node ) * nLabels + std::max( *adj_node, node ) ;
				for ( QList< std::pair< Z3i::Point, Z3i::Point > >::ConstIterator loc = locations[ key ].begin() ; bInside && loc != locations[ key ].end() ; loc++ ) {
					relative_loc_me = *pt + lower - (*me)( *loc ) ;
					squared_distance_me = relative_loc_me.dot( relative_loc_me ) ;
					relative_loc_other = *pt + lower - (*other)(*loc) ;
					squared_distance_other = relative_loc_other.dot( relative_loc_other ) ;
					if ( squared_distance_other > Distances( (*other)(*loc).at(1),(*other)(*loc).at(0),(*other)(*loc).at(2) ) ) continue ;
					
					if ( squared_distance_me > Distances( (*me)(*loc).at(1),(*me)(*loc).at(0),(*me)(*loc).at(2) ) ) { bInside = false ; continue ; }
					if ( squared_distance_other < squared_distance_me ) bInside = false ;
					if ( bInside && squared_distance_other == squared_distance_me ) n_can_not_seg++ ;
				}
			}
			if ( bInside ) {
				if ( componentImg( (*pt).at(1)+lower.at(1),(*pt).at(0)+lower.at(0),(*pt).at(2)+lower.at(2) ) ) {
					//std::cout<<(*pt).at(1)+lower.at(1)<<","<<(*pt).at(0)+lower.at(0)<<","<<(*pt).at(2)+lower.at(2)<<std::endl;
					#if BUILD_INDIVIDUAL_COMP
					thisCC( (*pt).at(1), (*pt).at(0),(*pt).at(2) ) = 2 ;
					#endif
				} else {
					#if BUILD_INDIVIDUAL_COMP
					thisCC( (*pt).at(1), (*pt).at(0),(*pt).at(2) ) = 1 ;
					#endif
				}
				componentImg( (*pt).at(1)+lower.at(1),(*pt).at(0)+lower.at(0),(*pt).at(2)+lower.at(2) ) = color ;
				nvoxels++ ;
			}
		}
	}
#if BUILD_INDIVIDUAL_COMP
IOPgm3d<int,qint8,false>::write( thisCC, QString( "/tmp/cc%1.pgm3d" ).arg( (int)color ) ) ;
std::cerr<<"CC "<<(int)color<<" : "<<accu( thisCC )<<std::endl;
#endif
	trace.info() <<"output (B) "<<nvoxels<<" voxels ** "<<n_can_not_seg<<std::endl;

	if ( bDebugThisLoop ) exit(-1);
}

int main( int narg, char **argv ) {

	// parse command line ----------------------------------------------
	po::options_description general_opt ( "Allowed options are: " );
	general_opt.add_options()
		( "help,h", "display this message." )
		( "skel,s", po::value<std::string>(), "Input colored skeleton pgm filename." )
		( "depth,d", po::value<std::string>(), "Input depth map pgm filename." )
		( "output,o", po::value<string>(),"Output pgm filename." )
		( "preserve,p", po::value<bool>()->default_value(true),"preserve colors.")
		( "id,i", po::value<int>()->default_value(-1),"id of the connected component to be rebuild (default, build everything)." );

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
	if ( ! ( vm.count ( "skel" ) ) ) missingParam ( "skel" );
	std::string inputFileName = vm["skel"].as<std::string>();
	if ( ! ( vm.count ( "depth" ) ) ) missingParam ( "depth" );
	std::string depthFileName = vm["depth"].as<std::string>();	
	if ( ! ( vm.count ( "output" ) ) ) missingParam ( "output" );
	std::string outputFileName = vm["output"].as<std::string>();
	
	
	int idComponent = vm["id"].as<int>() ;
	
	IPgm3dFactory ifactory ;
	
	trace.beginBlock("Importing skeleton") ;
	BillonTpl< IPgm3dFactory::value_type > * pSkeleton = ifactory.read( QString( inputFileName.c_str() ) );
	std::cerr<<"Min-max on input label skeleton : "<<pSkeleton->min()<<" "<<pSkeleton->max()<<std::endl
				<<"Min-max billon's prop "<<pSkeleton->minValue()<<" "<<pSkeleton->maxValue()<<std::endl;
	std::pair<Z3i::DigitalSet **, Z3i::Domain **> layers = labelimage_as_voxelsLayer( pSkeleton ) ;
	size_t n_cc = pSkeleton->max() ;
	QMap< IPgm3dFactory::value_type, QSet< IPgm3dFactory::value_type > > relations ;
	QMap< IPgm3dFactory::value_type, QList< std::pair< Z3i::Point, Z3i::Point > > > locations ;
	set_adjacency<IPgm3dFactory::value_type>( *pSkeleton, layers.first, relations, locations ) ;
	{
		/// \warning we can get several touching voxels for the same pair of connected components!
		/// print relations & their corresponding locations
		std::cerr<<"Relations:"<<std::endl;
		for ( QMap< IPgm3dFactory::value_type, QSet< IPgm3dFactory::value_type > >::iterator rel_source = relations.begin() ; rel_source != relations.end() ; rel_source++ )
			for ( QSet< IPgm3dFactory::value_type >::iterator rel_target = rel_source.value().begin() ; rel_target != rel_source.value().end() ; rel_target++ ) {
				std::cerr<<"cc "<<rel_source.key()<<" and cc "<< *rel_target<<" : " ;
				QMap< IPgm3dFactory::value_type, QList< std::pair< Z3i::Point, Z3i::Point > > >::iterator loc = locations.find( std::min(rel_source.key(),*rel_target) * (n_cc+1) + std::max(rel_source.key(),*rel_target) ) ;
				for ( int iLoc = 0 ; iLoc < loc.value().size() ; iLoc ++ )
					std::cerr<< loc.value()[ iLoc ].first<<" + "<<loc.value()[ iLoc ].second << " ";
				std::cerr<<std::endl;
			}
	}
	delete pSkeleton ;
	std::set<int> availablelabels ;
	Z3i::Point **boundaries = new Z3i::Point * [ (n_cc+1)*2 ] ;
	boundaries[0] = boundaries[1] = 0 ;
	boundaries[ (n_cc+1)*2-2 ] = boundaries[ (n_cc+1)*2-1 ] = 0 ;
	for ( int label = 1 ; label < n_cc ; label++ ) {
		boundaries[ (label)*2 + 0 ] = new Z3i::Point ;
		boundaries[ (label)*2 + 1 ] = new Z3i::Point ;
		/// find pair of adjacent voxels labelled (label,label+1)
		int32_t min_length_dep = std::numeric_limits< int32_t >::max(),
				length_dep ;
		Z3i::Point dep ;
		for ( Z3i::DigitalSet::ConstIterator pt_A = layers.first[ label ]->begin() ; pt_A != layers.first[ label ]->end() ; pt_A++ )
			for ( Z3i::DigitalSet::ConstIterator pt_B = layers.first[ label+1 ]->begin() ; pt_B != layers.first[ label+1 ]->end() ; pt_B++ ) {
				dep = *pt_A - *pt_B ;
				length_dep = dep.dot( dep ) ;
				if ( length_dep < min_length_dep ) {
					*boundaries[ (label)*2+0 ] = *pt_A ;
					*boundaries[ (label)*2+1 ] = *pt_B ;
					min_length_dep = length_dep ;
				}
			}
		assert( min_length_dep <= 3 ) ;
	}
	
	std::cerr<<"Info : histogram of the colored skeleton"<<std::endl;
	for ( int bin = 1 ; bin <= n_cc ; bin++ ) {
		std::cerr<<bin<<" : "<<layers.first[ bin ]->size() <<std::endl;
		availablelabels.insert( bin ) ;
	}
	std::cerr<<std::endl;

	trace.endBlock() ;
	
	trace.beginBlock("Importing depth map") ;
	BillonTpl< IPgm3dFactory::value_type > * pDistances = ifactory.read( QString( depthFileName.c_str() ) ) ;
	ifactory.correctEncoding( pDistances ) ;
	BillonTpl<char> componentImg( pDistances->n_rows,pDistances->n_cols,pDistances->n_slices ) ;
	componentImg.fill(0) ;
	trace.beginBlock("Reconstruction") ;
	if ( idComponent != -1 ) {
		if ( availablelabels.find( idComponent ) == availablelabels.end() ) {
			std::cerr<<"Missing value "<<idComponent<<std::endl;
			return -4 ;
		}
		availablelabels.clear() ;
		availablelabels.insert( idComponent);
	}
	for ( std::set<int>::const_iterator id = availablelabels.begin() ; id != availablelabels.end() ; id++ ) {
		trace.info() <<"Domain for layer "<< *id <<" : "<<* layers.second[ *id ]<<std::endl;
		//rebuildcc_indomain( *pDistances, *layers.first[ *id ], (vm["preserve"].as<bool>()? *id : 1 ), componentImg, boundaries + (*id-1) * 2 ) ;
		rebuildcc_indomain( *pDistances, *layers.first[ *id ], (char)(vm["preserve"].as<bool>()? *id : 1 ), componentImg, (IPgm3dFactory::value_type)*id, relations, locations, n_cc+1 ) ;
		componentImg.setMaxValue(*id) ;
		delete layers.first[ *id ] ;
		delete layers.second[ *id ] ;
	}
	delete [] layers.first ;
	delete [] layers.second ;
	for ( int iBound = 0 ; iBound < (n_cc+1)*2 ; iBound++ ) {
		if ( boundaries[ iBound ] )
			delete boundaries[ iBound ] ;
	}
	delete [] boundaries ;
	trace.endBlock() ;
	delete pDistances ;
	trace.endBlock() ;
	IOPgm3d< char,qint8, false >::write( componentImg, QString( outputFileName.c_str() ) ) ;
	return 0 ;
}
