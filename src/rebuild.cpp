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
	std::cerr 	<< "Selection (or separation when reject is specified) of skeleton point wrt their depth. When both percentage and value are specified, selection is done wrt the first criterion being valid."<<std::endl
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
		rebuildcc_indomain( *pDistances, *layers.first[ *id ], (vm["preserve"].as<bool>()? *id : 1 ), componentImg, boundaries + (*id-1) * 2 ) ;
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
