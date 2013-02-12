/**
 * \brief segmentation of the skeleton based on a min depth value
 */
#include <def_coordinate.h>
#include <SkeletonGraph.hpp>
#include <io/IOPgm3d.h>
#include <io/Pgm3dFactory.h>
#include <GrayLevelHistogram.h>
#include <connexcomponentextractor.h>
#include <ConnexComponentRebuilder.hpp>
#include <geom2d.h>

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

typedef struct _TRefLine {
	int x,z ;
	int dx,dz ;
	double len ;
} TRefLine ;

void set_projections( const TRefLine & refLine, QMap< arma::u32, Interval<double> > *bounds, uint nSamples, const BillonTpl< arma::u32 > &img, arma::u32 th ) {
	BillonTpl< arma::u32 >::const_iterator iterImg = img.begin(),
	                                       iterEndImg = img.end() ;
	uint n_rows = img.n_rows ,
	     n_cols = img.n_cols;
	uint x=0,y=0,z=0;
	
//std::cerr<<"[ Debug ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
	
	for ( ; iterImg != iterEndImg ; iterImg++ ) {
		if ( *iterImg && *iterImg < th ) {
			double d = ( refLine.dx * ( x - refLine.x ) + refLine.dz * ( z - refLine.z ) ) / refLine.len ;
			if ( ! bounds[ y*nSamples/n_rows ].contains( *iterImg ) ) bounds[ y*nSamples/n_rows ].insert( *iterImg, Interval<double>( d, d ) ) ;
			else {
				Interval<double>& aBound = bounds[ y*nSamples/n_rows ].find( *iterImg ).value() ;
				if ( aBound.min() > d ) aBound.setMin( d ) ;
				else if ( aBound.max() < d ) aBound.setMax( d ) ;
			}
		}
		y++ ;
		if ( y == n_rows ) {
			y = 0 ;
			x++ ;
		}
		if ( x == n_cols ) {
			x = 0 ;
			z++ ;
		}
	}
//std::cerr<<"[ Debug ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
}

std::pair< uint, double > eval_projections( const QMap< arma::u32, Interval<double> > &bounds, QList< arma::u32 > *pLstBreaks ) {
	///
	/// \brief build a sorted list of projection intervals wrt first the min, next the max
	/// \note to keep the linked with the corresponding identifier, we define a list of identifiers...
	///
	QList< arma::u32 > lexiLabels ;
	QList< arma::u32 >::Iterator position,end_of_list ;
	QMap< arma::u32, Interval<double> >::ConstIterator iterBound = bounds.begin(),
	                                                   iterEndBound = bounds.end() ;
//std::cerr<<"[ Debug ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
	for ( ; iterBound != iterEndBound ; iterBound++ ) {
		position = lexiLabels.begin() ;
		end_of_list = lexiLabels.end() ;
		while ( position != end_of_list ) {
			if ( bounds[ *position ].min() > iterBound.value().min() ) break ;
			if ( bounds[ *position ].min() == iterBound.value().min() && bounds[ *position ].max() < iterBound.value().max() ) break ;
			position++ ;
		}
		lexiLabels.insert( position, iterBound.key() ) ;
	}
	
	position = lexiLabels.begin() ;
	end_of_list = lexiLabels.end() ;
	double last_max = -1 ;
	double gap_strength = 0 ;
	uint number_of_cluster = 0 ;
	
	for ( ; position != end_of_list ; position++ ) {
		if ( bounds[ *position ].min() > last_max ) {
			if ( last_max > 0 ) gap_strength += bounds[ *position ].min() - last_max ;
			number_of_cluster++ ;
			if ( pLstBreaks )
				pLstBreaks->append( *position ) ;
		}
		if ( bounds[ *position ].max() > last_max ) {
			last_max = bounds[ *position ].max() ;
		}
	}
//std::cerr<<"[ Debug ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
	return std::pair< uint, double > ( number_of_cluster, gap_strength ) ;
}

void discriminate_projection( const BillonTpl< arma::u32 > &label, const QMap< arma::u32, Interval<double> > *bounds, uint nSamples ) {
	BillonTpl< arma::u32 >::const_iterator iterImg = label.begin(),
	                                       iterEndImg = label.end() ;
	uint n_rows = label.n_rows ,
	     n_cols = label.n_cols,
	     n_slices = label.n_slices ;
	uint x=0,y=0,z=0;

	BillonTpl< arma::u32 > gt( n_rows, n_cols, n_slices ) ;
	gt.fill( 0 ) ;
	BillonTpl< arma::u32 >::iterator iterWrite = gt.begin() ;
	
	QMap< arma::u32, Interval<double> >::ConstIterator bound_label ;
	QList< arma::u32 > *breaks_per_sample = new QList< arma::u32 > [ nSamples ];
	for ( uint iSample = 0 ; iSample < nSamples; iSample++ ) {
		uint qNum ;
		double qCluster ;
		boost::tie( qNum, qCluster ) = eval_projections( bounds[ iSample ], breaks_per_sample + iSample ) ;
		std::cout<<"Sample "<<(int)iSample<<"/"<<nSamples<<"    : "<<qNum<<" cluster(s) with energy "<<qCluster<<std::endl;
		for ( uint iCut=0;iCut < breaks_per_sample[ iSample ].size() ; iCut++ )
			std::cout<<"\t"<<breaks_per_sample[ iSample ].at(iCut)
			         <<" interval "<<bounds[ iSample ][ breaks_per_sample[ iSample ].at(iCut) ].min()<<" : "
			                       <<bounds[ iSample ][ breaks_per_sample[ iSample ].at(iCut) ].max()<<std::endl;
	}
	
	QList< arma::u32 > & LstBreaks = breaks_per_sample[ 0 ] ;
	const QMap< arma::u32, Interval<double> > * current_bound = bounds + 0 ;	
	QMap< arma::u32, Interval<double> >::ConstIterator undefined_label = current_bound->end() ;
	arma::u32 number_of_previous_breaks = 0 ;
	
		
	for ( ; iterImg != iterEndImg ; iterImg++, iterWrite++ ) {
		if ( *iterImg ) {
			bound_label = current_bound->constFind( *iterImg ) ;
			if ( bound_label != undefined_label ) {
				arma::u32 iBreak = 0 ;
				while ( iBreak < LstBreaks.size() ) {
					if ( bound_label.value().min() <= (*current_bound)[ LstBreaks.at( iBreak ) ].min() ) break ;
					iBreak++ ;
				}
				*iterWrite = iBreak + number_of_previous_breaks;
			}
		}
		y++ ;
		if ( y == n_rows ) {
			y = 0 ;
			x++ ;
		}
		if ( y % (n_rows / nSamples) == 0 ) {
			current_bound = bounds + ( y * nSamples/n_rows ) ;
			undefined_label = current_bound->end() ;
			if( y > 0 )
				number_of_previous_breaks += breaks_per_sample[ (y-1) * nSamples/n_rows ].size() ;
			else
				number_of_previous_breaks = 0 ;
			LstBreaks = breaks_per_sample[ y * nSamples/n_rows ] ;
		}
		if ( x == n_cols ) {
			x = 0 ;
			z++ ;
		}	
	}
	delete [] breaks_per_sample ;
	IOPgm3d< arma::u32, qint32, false >::write( gt, "/tmp/gt.pgm3d");
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
		ConnexComponentExtractor< arma::u8,arma::u32 > cce ;
		BillonTpl< arma::u32 > *lblSkelImg = cce.run( *skelimg ) ;
		ConnexComponentExtractor< arma::u8,arma::u32 >::TMapVolume::ConstIterator iterVolume = cce.volumes().begin(),
		                                                                          iterVolumeEnd = cce.volumes().end(),
		                                                                          mainVolume ;
		for ( mainVolume = iterVolume ; iterVolume != iterVolumeEnd ; iterVolume++ )
			if ( mainVolume.value() < iterVolume.value() )
				mainVolume = iterVolume ;
		BillonTpl< arma::u32 >::const_iterator iterLbl = lblSkelImg->begin(),
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
	BillonTpl< arma::u32 > * label = new BillonTpl< arma::u32 > ( classifiedVoxel->n_rows, classifiedVoxel->n_cols, classifiedVoxel->n_slices ) ;
	label->fill( 0 ) ;
	SG_u8 *sg_above = new SG_u8( *classifiedVoxel, color_above ) ;
	arma::u32 from = 1 ;
	from = do_labeling( sg_above, *label, from ) ;
	delete sg_above ;
	
	IOPgm3d< arma::u32, qint32, false >::write( *label, QString( "/tmp/rooms.pgm3d" ) ) ;
	
	///
	/// Extracting corridors
	///
	std::cout<<(int)from<<std::endl<<"extracting corridors..."<<std::endl;
	SG_u8 *sg_below = new SG_u8( *classifiedVoxel, color_below ) ;
	arma::u32 to = do_labeling( sg_below, *label, from ) ;
	delete sg_below ;
	delete classifiedVoxel ;
	
	///
	/// Extracting candidate merge
	///
	std::cout<<(int)to<<std::endl<<"extracting adjacency relations..."<<std::endl;
	QMap< arma::u32, QList< arma::u32 > > touching ;
	extract_adjacency( *label, touching, from ) ;
	
	///
	/// a corridor being only linked to a single room can be merged with that room
	///
	std::cout<<"identifying merge operation..."<<std::endl;
	QMap< arma::u32, QList< arma::u32 > >::Iterator iterRewrite = touching.begin() ;
	QMap< arma::u32, arma::u32 > translation ;
	for ( ; iterRewrite != touching.end() ; ) {
		if ( iterRewrite.value().size() == 1 ) {
			translation.insert( iterRewrite.key(), iterRewrite.value().takeFirst() ) ;
			iterRewrite = touching.erase( iterRewrite ) ;
		} else
			iterRewrite++ ;
	}
	std::cout<<"applying "<< translation.size()<<" selected operations..."<<std::endl;
	apply_translation( *label, translation ) ;
	translation.clear() ;
	
	
	///
	/// reconstruct each individual connected component to evaluate their volume
	///
	#if 0
	typedef ConnexComponentRebuilder< arma::u32, arma::u32, arma::u32 > TConnexComponentRebuilder ;
	TConnexComponentRebuilder *ccr = new TConnexComponentRebuilder( *label ) ;
	ccr->setDepth( QString( params._depthFilePath.c_str() ) ) ;
	ccr->run() ;
	BillonTpl< arma::u32 > *reconstruction = new BillonTpl< arma::u32 >( ccr->result() ) ;
	delete ccr ;
	GrayLevelHistogram<arma::u32> volumes_ext( *reconstruction ) ;
	GrayLevelHistogram<arma::u32>::THistogram &volumes = volumes_ext._bin ;
	/// build a sorted list of ccx (per type) wrt their volume
	QList< arma::u32 > corridors, rooms ;
	QList< arma::u32 > *ptr ;
	QList< arma::u32 >::Iterator position ;
	for ( GrayLevelHistogram<arma::u32>::THistogram::iterator it = volumes.begin() ; it != volumes.end() ; it++ ) {
		ptr = & corridors ;
		if ( it->first < from ) ptr = & rooms ;
		position = ptr->begin() ;
		while ( position != ptr->end() ) {
			if ( volumes[ *position ] > it->second ) break ;
			position++ ;
		}
		ptr->insert( position, it->first ) ;
	}
	#endif
	/// criteria based on size...	
	
	
	if ( false ) {
		///
		/// for algorithm design only on MeMo0013/serie_2 ...
		///
		uint n_cols = label->n_cols,
		     n_rows = label->n_rows,
		     n_slices = label->n_slices ;
		
		uint nSamples ;
		std::cout<<"Number of samples to discretize the y values : " ;
		std::cin >> nSamples ;
		
		QMap< arma::u32, Interval<double> > *bounds = new QMap< arma::u32, Interval<double> >[ nSamples ] ;
		
		TRefLine refLine ;
		refLine.x = 0 ;
		refLine.z = 60 ;
		refLine.dx = 287 ;
		
		refLine.dz = 433 ;
		int best_dz = -1 ;
		uint best_num = 0, nCluster ;
		double qCluster ;
		for ( refLine.dz = refLine.dx ; refLine.dz < 2 * refLine.dx ; refLine.dz += 10 ) {
			refLine.len = sqrt( refLine.dx*refLine.dx+refLine.dz*refLine.dz ) ;
			set_projections( refLine, bounds, nSamples, *label, from ) ;
			boost::tie( nCluster, qCluster ) = eval_projections( bounds[ nSamples/2 ],0 ) ;
			if ( nCluster > best_num ) {
				best_dz = refLine.dz ;
				best_num= nCluster ;
			}
			for ( uint iSample = 0 ; iSample < nSamples ; iSample ++ ) bounds[ iSample ].clear() ;
		}
		refLine.dz = best_dz ;
		set_projections( refLine, bounds, nSamples, *label, from ) ;
		std::cout<<"Optimal projection on line "<<refLine.x<<","<<refLine.z<<" - "<<refLine.dx<<","<<refLine.dz<<std::endl;
		discriminate_projection( *label, bounds, nSamples ) ;
		BillonTpl< arma::u8 > proj_xz( n_cols, n_slices,nSamples ) ;
		BillonTpl< arma::u8 > proj_yz( n_rows, n_slices,nSamples ) ;
		proj_xz.fill( 0 ) ;
		proj_yz.fill( 0 ) ;
		proj_xz.setMaxValue(1) ;
		proj_yz.setMaxValue(1) ;
		BillonTpl< arma::u32 >::const_iterator iterRoom = label->begin(),
		                                       iterRoomEnd = label->end() ;
		uint x=0,y=0,z=0 ;
		for ( ; iterRoom != iterRoomEnd ; iterRoom++ ) {
			if ( *iterRoom && *iterRoom < from ) {
				if ( true /*y*4 > 0*n_rows && y*4 < 1*n_rows*/ )
					proj_xz(x,z,y*nSamples/n_rows ) = y*nSamples/n_rows+1 ;
				if ( true /*x*4 > 0*n_cols && x*4 < 1*n_cols*/ )
					proj_yz(y,z,y*nSamples/n_rows ) = y*nSamples/n_rows+1 ;
			}
			y++ ;
			if ( y == n_rows ) {
				y = 0 ;
				x++ ;
			}
			if ( x == n_cols ) {
				x = 0 ;
				z++ ;
			}
		}
		
		for ( uint iSample = 0 ; false && iSample < nSamples ; iSample++ ) {
			std::cout<<"= begin =======;"<<iSample+1<<std::endl;
			for ( QMap< arma::u32,Interval<double> >::ConstIterator it = bounds[iSample].begin() ; it != bounds[iSample].end() ; it++ )
				std::cout<<";"<<(int)it.key()<<" ;"<<it.value().min()<<" "<<it.value().max()<<";"<<std::endl;
			std::cout<<"======== end ==;"<<std::endl;
		}
		delete [] bounds ;
		
		for ( uint iCut = 0 ; false && iCut < 20 ; iCut++ ) {
			unsigned int len ;
			Z2i::Point *pts = Geom2D::computeD8Segment( Z2i::Point(0,(n_slices-1)/40*(iCut+18.5)), Z2i::Point(n_cols-1,(n_slices-1)/40*(iCut+0.5)), len  ) ;
			for ( Z2i::Point *pt = pts ; pt != pts + len ; pt++ )
				proj_xz( pt->at(0), pt->at(1), 0 ) = 64 ;
			delete [] pts ;
		}
		uint len,c ;
		Z2i::Point *pts = Geom2D::computeD8Segment( Z2i::Point(refLine.x,refLine.z), Z2i::Point(refLine.x+refLine.dx, refLine.z+refLine.dz ) , len  ) ;
		for ( Z2i::Point *pt = pts ; pt != pts + len ; pt++ ) {
			if ( pt->at(0) < n_cols && pt->at(1) < n_slices )
				for ( c = 0 ; c < proj_xz.n_slices ; c++ )
					proj_xz( pt->at(0), pt->at(1), c ) = 96 ;
		}
		delete [] pts ;
		
		IOPgm3d< arma::u8, qint8, false >::write ( proj_xz, "/tmp/proj_xz.pgm" ) ;
		IOPgm3d< arma::u8, qint8, false >::write ( proj_yz, "/tmp/proj_yz.pgm" ) ;
	}
	
	if ( false ) {
		///
		/// to easier vizualizing rooms
		///
		BillonTpl< arma::u32 >::iterator iterLbl = label->begin(),
		                                 iterLblEnd = label->end() ;
		for ( ; iterLbl != iterLblEnd ; iterLbl ++ )
			if ( *iterLbl >= from ) *iterLbl = from + 1 ; /// only one label for all corridors
	}
	IOPgm3d< arma::u32, qint32, false >::write( *label, QString( params._outputFilePath.c_str() ) ) ;
	delete label ;
	#if 0
	delete reconstruction ;
	#endif
	return 0 ;
}
