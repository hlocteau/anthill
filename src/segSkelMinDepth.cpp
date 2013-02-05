/**
 * \brief segmentation of the skeleton based on a min depth value
 */
#include <def_coordinate.h>
#include <SkeletonGraph.hpp>
#include <io/IOPgm3d.h>
#include <io/Pgm3dFactory.h>
#include <GrayLevelHistogram.h>
#include <connexcomponentextractor.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <boost/filesystem.hpp>

#include <boost/graph/connected_components.hpp>

namespace fs=boost::filesystem;
namespace po=boost::program_options ;

typedef struct _TProgramArg {
	fs::path _skelFilePath ;
	fs::path _depthFilePath ;
	fs::path _outputFilePath ;
	int      _minDepth ;
	int      _minSize ;
} TProgramArg ;

	typedef SkeletonGraph< arma::u8 > SG_u8 ;

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Segmentation of the skeleton based on a min depth value."<<std::endl
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
		( "skel,s", po::value<std::string>(), "Input colored skeleton pgm filename." )
		( "depth,d", po::value<std::string>(), "Input depth map pgm filename." )
		( "output,o", po::value<string>(),"Output pgm filename." )
		( "threshold,t", po::value<int>(),"threshold to be used for classification of skeleton's voxels based on their depth value." )
		( "volume,v", po::value<int>()->default_value(0),"threshold to be used for reconstructed connected component regarding its size." );

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
	if ( ! vm.count( "threshold" ) )   return missingParam ( "threshold" );

	params._minDepth = vm["threshold"].as<int>() ;
	params._minSize = vm["volume"].as<int>() ;
	params._skelFilePath = vm["skel"].as<std::string>();
	params._depthFilePath = vm["depth"].as<std::string>();
	params._outputFilePath = vm["output"].as<std::string>();
	return true ;
}

template < typename T > T do_labeling( SG_u8 * sg, BillonTpl< T > & result, T from ) {
	const SG_u8::graph_t & graph = sg->graph() ;
	
	typename std::vector<T> component(num_vertices(graph));
	T num = connected_components(graph, &component[0]);
	for ( SkeletonGraph<arma::u8>::ConstVoxelIterator it = sg->encoding_begin() ; it != sg->encoding_end() ; it++ ) {
		iCoord3D voxel = sg->from_linear_coord( it.key() ) ;
		result( voxel.y, voxel.x, voxel.z ) = component[ it.value() ]+from ;
	}
	return num + from ;
}

template <typename T > void extract_adjacency( const BillonTpl< T > &label, QMap< T, QList< T > > &touching, T th ) {
	uint x,y,z,n ;
	uint n_slices = label.n_slices,
	     n_cols = label.n_cols,
	     n_rows = label.n_rows;
	T cur_value, adj_value ;
	for ( z = 0 ; z < n_slices ; z++ )
		for ( y = 0 ; y < n_rows ; y++ )
			for ( x = 0 ; x < n_cols ; x++ ) {
				cur_value = label( y,x,z ) ;
				if ( cur_value < th ) continue ;
				for ( n = 0 ; n < 27 ; n++ )
					if ( ( y + (n/9-1)) >= 0 		&& ( y + (n/9-1)) < n_rows &&
						 ( x + ( (n%9)/3 -1 )) >= 0 && ( x + ( (n%9)/3 -1 ))<n_cols &&
						 ( z + ( n % 3 -1 )) >= 0 	&& ( z + ( n % 3 -1 ))<n_slices ) {
						adj_value = label( y + (n/9-1), x + ( (n%9)/3 -1 ), z + ( n % 3 -1 ) ) ;
						if ( adj_value && adj_value != cur_value ) {
							if ( !touching.contains( cur_value ) ) touching.insert( cur_value, QList<T>() ) ;
							touching[ cur_value ].append( adj_value ) ;
						}
					}
			}
	/// remove duplicates
	typename QMap< T, QList< T > >::Iterator source = touching.begin(),
									         sourceEnd = touching.end() ;
	for ( ; source != sourceEnd ; source++ ) {
		qSort( source.value().begin(), source.value().end(), qLess< T >( ) ) ;
	}
}

template <typename T > void apply_translation( BillonTpl< T > &label, const QMap< T, T > &translation ) {
	typename BillonTpl< T >::iterator iterVoxel = label.begin(),
	                                  iterVoxelEnd = label.end() ;
	typename QMap< T,T >::ConstIterator rule, undefined_rule = translation.end() ;
	for ( ; iterVoxel != iterVoxelEnd ; iterVoxel++ ) {
		if ( ! *iterVoxel ) continue ;
		rule = translation.constFind( *iterVoxel ) ;
		if ( rule != undefined_rule )
			*iterVoxel = rule.value() ;
	}
}

int main( int narg, char **argv ) {
	TProgramArg params ;
	if ( !process_arg( narg, argv, params ) ) return -1 ;
	
	BillonTpl< arma::u8 > * skelimg   ; /// ...
	BillonTpl< arma::u32 > * depthimg ; /// ...
	
	{
		Pgm3dFactory< arma::u8 > factory ;
		skelimg = factory.read( QString( params._skelFilePath.c_str() ) ) ;
		
		/// keep only the biggest connected component
		ConnexComponentExtractor< arma::u8,arma::u16 > cce ;
		BillonTpl< arma::u16 > *lblSkelImg = cce.run( *skelimg ) ;
		ConnexComponentExtractor< arma::u8,arma::u16 >::TMapVolume::ConstIterator iterVolume = cce.volumes().begin(),
		                                                                          iterVolumeEnd = cce.volumes().end(),
		                                                                          mainVolume ;
		for ( mainVolume = iterVolume ; iterVolume != iterVolumeEnd ; iterVolume++ )
			if ( mainVolume.value() < iterVolume.value() )
				mainVolume = iterVolume ;
		BillonTpl< arma::u16 >::const_iterator iterLbl = lblSkelImg->begin(),
		                                       iterLblEnd = lblSkelImg->end() ;
		BillonTpl< arma::u8 >::iterator        iterSkel = skelimg->begin();
		for ( ; iterLbl != iterLblEnd ; iterLbl++,iterSkel++ )
			*iterSkel = ( *iterLbl == mainVolume.key() ? 1 : 0 ) ;
		delete lblSkelImg ;
	}
	{
		Pgm3dFactory< arma::u32 > factory ;
		depthimg = factory.read( QString( params._depthFilePath.c_str() ) ) ;
		factory.correctEncoding( depthimg );
		
		/// histogram for debuging purpose
		/// \note to be meaningful, we should only evaluate voxels that belong to the skeleton
		{
			BillonTpl< arma::u8 >::const_iterator iterSkel = skelimg->begin(),
	                                              iterSkelEnd = skelimg->end() ;
			BillonTpl< arma::u32 >::iterator iterDepth = depthimg->begin() ;
			for ( ; iterSkel != iterSkelEnd ; iterSkel++, iterDepth++ ) {
				if ( ! *iterSkel ) *iterDepth = 0 ;
			}
		}
		GrayLevelHistogram< arma::u32 > h( *depthimg ) ;
		for ( GrayLevelHistogram< arma::u32 >::THistogram::iterator bin = h._bin.begin() ; bin != h._bin.end() ; bin++ )
			std::cout<<(int) bin->first<<" : "<<(int) bin->second<<std::endl;
	}	
	assert( skelimg->n_slices == depthimg->n_slices && skelimg->n_rows == depthimg->n_rows && skelimg->n_cols == depthimg->n_cols ) ;
	///
	/// Initialization
	///
	std::cout<<"initialization..."<<std::endl;
	BillonTpl< arma::u8 > * classifiedVoxel = new BillonTpl< arma::u8 > ( skelimg->n_rows, skelimg->n_cols, skelimg->n_slices ) ;
	BillonTpl< arma::u8 >::const_iterator iterSkel = skelimg->begin(),
	                                      iterSkelEnd = skelimg->end() ;
	BillonTpl< arma::u32 >::const_iterator iterDepth = depthimg->begin() ;
	BillonTpl< arma::u8 >::iterator       iterClassified = classifiedVoxel->begin() ;
	
	arma::u8 color_below = 1 ;
	arma::u8 color_above = 2 ;
	for ( ; iterSkel != iterSkelEnd ; iterSkel++, iterDepth++, iterClassified++ ) {
		if ( *iterSkel ) {
			*iterClassified = ( *iterDepth > params._minDepth ? color_above : color_below ) ;
		} else {
			*iterClassified = 0 ;
		}
	}
	
	delete skelimg ;
	delete depthimg ;
	
	///
	/// Extracting rooms
	///
	std::cout<<"extracting rooms..."<<std::endl;
	BillonTpl< arma::u16 > * label = new BillonTpl< arma::u16 > ( classifiedVoxel->n_rows, classifiedVoxel->n_cols, classifiedVoxel->n_slices ) ;
	label->fill( 0 ) ;
	SG_u8 *sg_above = new SG_u8( *classifiedVoxel, color_above ) ;
	arma::u16 from = 1 ;
	from = do_labeling( sg_above, *label, from ) ;
	
	{
		/// \warning with command line : segSkelMinDepth --skel ~/outputData/MeMo0013/serie_2/anthill.innerskel.mk3.pgm3d --depth ~/outputData/MeMo0013/serie_2/anthillcontent.dist.pgm3d --output /tmp/segdepth.pgm3d --threshold 24
		/// component 5 and 553 are touching!
	
	}
	delete sg_above ;
	
	IOPgm3d< arma::u16, qint16, false >::write( *label, QString( "/tmp/rooms.pgm3d" ) ) ;
	
	///
	/// Extracting corridors
	///
	std::cout<<"extracting corridors..."<<std::endl;
	SG_u8 *sg_below = new SG_u8( *classifiedVoxel, color_below ) ;
	arma::u16 to = do_labeling( sg_below, *label, from ) ;
	delete sg_below ;
	delete classifiedVoxel ;
	
	///
	/// Extracting candidate merge
	///
	std::cout<<"extracting adjacency relations..."<<std::endl;
	QMap< arma::u16, QList< arma::u16 > > touching ;
	extract_adjacency( *label, touching, from ) ;
	
	std::cout<<"identifying merge operation..."<<std::endl;
	QMap< arma::u16, QList< arma::u16 > >::Iterator iterRewrite = touching.begin() ;
	QMap< arma::u16, arma::u16 > translation ;
	for ( ; iterRewrite != touching.end() ; ) {
		if ( iterRewrite.value().size() == 1 ) {
			translation.insert( iterRewrite.key(), iterRewrite.value().takeFirst() ) ;
			iterRewrite = touching.erase( iterRewrite ) ;
		} else
			iterRewrite++ ;
	}
	std::cout<<"applying "<< translation.size()<<" selected operations..."<<std::endl;
	apply_translation( *label, translation ) ;
	
	{
		///
		/// to easier vizualizing rooms
		///
		BillonTpl< arma::u16 >::iterator iterLbl = label->begin(),
		                                 iterLblEnd = label->end() ;
		for ( ; iterLbl != iterLblEnd ; iterLbl ++ )
			if ( *iterLbl >= from ) *iterLbl = from + 1 ; /// only one label for all corridors
	}
	IOPgm3d< arma::u16, qint16, false >::write( *label, QString( params._outputFilePath.c_str() ) ) ;
	delete label ;
	/// criteria based on size...
	
	return 0 ;
}
