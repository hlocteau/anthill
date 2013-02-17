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
		for ( uint k = 0 ; k < source.value().size() ; k++ ) {
			uint m = source.value().lastIndexOf( source.value().at(k) ) ;
			while ( k != m ) {
				source.value().removeAt(m) ; 
				m-- ;
			}
		}
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
	
	if ( (int)label->max() < (int)std::numeric_limits<unsigned int>::max() ) {
std::cerr<<"[export rooms @"<<__LINE__<<"] trace max()="<<(int)label->max()<<" vs "<<(int)from<<std::endl;
		IOPgm3d< arma::u32, qint8, false >::write( *label, QString( "/tmp/rooms.pgm3d" ) ) ;
	} else if ( (int)label->max() < (int)std::numeric_limits<unsigned short>::max() ) {
std::cerr<<"[export rooms @"<<__LINE__<<"] trace max()="<<(int)label->max()<<" vs "<<(int)from<<std::endl;
		IOPgm3d< arma::u32, qint16, false >::write( *label, QString( "/tmp/rooms.pgm3d" ) ) ;
	} else {
std::cerr<<"[export rooms @"<<__LINE__<<"] trace max()="<<(int)label->max()<<" vs "<<(int)from<<std::endl;
		IOPgm3d< arma::u32, qint32, false >::write( *label, QString( "/tmp/rooms.pgm3d" ) ) ;
	}
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
	/// reconstruct each individual connected component being a room
	///
	QList< arma::u32> noLabels ;
	for ( arma::u32 i=from;i<to;i++) 
		noLabels.append( i ) ;
	ConnexComponentRebuilder< arma::u32, int32_t, arma::u32 > CCR( *label, &noLabels );
	CCR.setDepth( QString( params._depthFilePath.c_str() ) ) ;
	trace.beginBlock("Reconstruction") ;
	for ( arma::u32 i=1;i<from;i++) {
		std::cerr<<"step "<<(int)i<<" / "<<(int)from<<std::endl;
		CCR.run( i,i ) ;
	}
	touching.clear() ;
	
	/// save before renaming wrt adjacency relations
	if ( CCR.result().max() & 0x00ff )
		IOPgm3d< arma::u32, qint8, false >::write( CCR.result(), QString( "/tmp/rebuildfarbound.pgm3d" ) ) ;
	else if ( label->max() & 0x0000ffff )
		IOPgm3d< arma::u32, qint16, false >::write( CCR.result(), QString( "/tmp/rebuildfarbound.pgm3d" ) ) ;
	else
		IOPgm3d< arma::u32, qint32, false >::write( CCR.result(), QString( "/tmp/rebuildfarbound.pgm3d" ) ) ;
	
	
	extract_adjacency( CCR.result(), touching, (arma::u32)1 ) ;
	
	for ( iterRewrite = touching.begin() ; iterRewrite != touching.end() ; iterRewrite++ ) {
		std::cout<<cast_integer<arma::u32, int>(iterRewrite.key())<<" : ";
		for ( int k=0;k<iterRewrite.value().size();k++ )
			std::cout<<cast_integer<arma::u32, int>(iterRewrite.value().at(k))<<" ";
		std::cout<<std::endl;
	}
	
	
	iterRewrite = touching.begin() ;
	
	for ( ; iterRewrite != touching.end() ;  ) {
		if ( iterRewrite.key() > iterRewrite.value().at(0) ) {
			touching[ iterRewrite.value().at(0) ].append( iterRewrite.key() ) ;
			touching[ iterRewrite.value().at(0) ].append( iterRewrite.value() ) ;
			qSort( touching[ iterRewrite.value().at(0) ].begin(), touching[ iterRewrite.value().at(0) ].end(), qLess<arma::u32>() ) ;
			iterRewrite = touching.erase( iterRewrite ) ;
		} else
			iterRewrite++;
	}
	iterRewrite = touching.begin() ;
	for ( ; iterRewrite != touching.end() ; iterRewrite++ ) {
		/**
		 * \see code being used in connexcomponentextractor.ih
		 */
		QList< arma::u32 > &voisinage = iterRewrite.value() ;
		QMap< arma::u32, arma::u32 > &tableEquiv = translation ;
		arma::u32 mini = iterRewrite.key() ;
		arma::u32 currentEquiv ;
		
		for ( int ind = 0 ; ind < voisinage.size() ; ind++ ) {
			if ( voisinage[ ind ] == mini ) continue ;
			if ( !tableEquiv.contains( voisinage[ ind ] ) ) {
				tableEquiv[ voisinage[ind] ] = mini ;
			} else {
				currentEquiv = tableEquiv[ voisinage[ ind ] ] ;
				if ( currentEquiv == mini ) continue ;
				if ( mini > currentEquiv ) {
					tableEquiv[ voisinage[ ind ] ] = mini ;
					if ( tableEquiv.contains( mini ) ) {
						while ( tableEquiv.contains( mini ) ) {
							mini = tableEquiv[ mini ] ;
						}
					}
					if ( currentEquiv < mini ) {
						tableEquiv[ mini ] = currentEquiv ;
						//labels.at(j,i) = currentEquiv ;
						touching[ currentEquiv ].append( iterRewrite.value() ) ;
					} else if ( currentEquiv > mini ) {
						tableEquiv[ currentEquiv ] = mini ;
						//labels.at(j,i) = mini ;
						touching[ mini ].append( iterRewrite.value() ) ;
					}
				} else {
					while ( tableEquiv.contains( currentEquiv ) )
						currentEquiv = tableEquiv[ currentEquiv ] ;
					if ( currentEquiv > mini ) {
						tableEquiv[ currentEquiv ] = mini ;
						//labels.at(j,i) = mini ;
						touching[ mini ].append( iterRewrite.value() ) ;
					} else if ( currentEquiv < mini ) {
						tableEquiv[ mini ] = currentEquiv ;
						//labels.at(j,i) = currentEquiv ;
						touching[ currentEquiv ].append( iterRewrite.value() ) ;
					}
				}
			}
		}
	}
	{
		QMap< arma::u32, arma::u32 > &tableEquiv = translation ;
		QMapIterator<arma::u32, arma::u32> iter(tableEquiv) ;
		while ( iter.hasNext() ) {
			iter.next();
			if ( tableEquiv.contains(iter.value()))
				tableEquiv[iter.key()]=tableEquiv[iter.value()];
		}		
	}
	
	QList<arma::u32> terminal ;
	/// insert isolated cc
	for ( arma::u32 k=1;k<from;k++ ) {
		if ( !translation.contains( k ) ) {
			translation[k]=k;
			terminal.append(k) ;
		} else {
			if ( translation[k] == k )
				terminal.append(k) ;
		}
	}
	/// minimal distinct values
	{
		for ( uint k = 1 ; k < from ; k++ ) {
			translation[ k ] = terminal.indexOf( translation[ k ] ) + 1;
		}
	}
	
	BillonTpl< arma::u32 > ccRooms = CCR.result() ;
	apply_translation( ccRooms, translation ) ;

	/// remaining part is attached iff it is connected to a single region
	BillonTpl< arma::u8 > sceneRemain( n_rows, n_cols, n_slices ) ;
	sceneRemain.fill(0);
	BillonTpl< arma::u32 >::const_iterator iterLabelCC = ccRooms.begin() ;
	BillonTpl< arma::u32 >::const_iterator iterDepth = depthimg->begin() ;
	BillonTpl< arma::u8 >::iterator iterRemain = sceneRemain.begin(),
									iterRemainEnd = sceneRemain.begin() ;
	while ( iterRermain != iterRemainEnd ) {
		if ( *iterDepth ) {
			if ( *iterLabelCC == 0 )
				*iterRemain = 1 ;
		}
		iterDepth++ ;
		iterLabelCC++;
		iterRemain++;
	}
	ConnexComponentExtractor< arma::u8, arma::u32 > CCERemain ;
	BillonTpl< arma::u32 > *lblRemain = CCERemain.run( sceneRemain ) ;
	arma::u32 nSeeds = ccRooms.max() ;
	*lblRemain += nSeeds ;
	*lblRemain += ccRooms ;
	extract_adjacency( lblRemain, touching, nSeeds ) ;

	if ( ccRooms.max() & 0x00ff )
		IOPgm3d< arma::u32, qint8, false >::write( ccRooms, QString( params._outputFilePath.c_str() ) ) ;
	else if ( label->max() & 0x0000ffff )
		IOPgm3d< arma::u32, qint16, false >::write( ccRooms, QString( params._outputFilePath.c_str() ) ) ;
	else
		IOPgm3d< arma::u32, qint32, false >::write( ccRooms, QString( params._outputFilePath.c_str() ) ) ;
	
	/** 
	 * \todo should be a method of Pgm3dFactory
	 * \brief we may used the masks 0x00ff, 0x0000ffff on label->max() to determiner what should be the encoding of the output file
	 */
	delete label ;
	#if 0
	delete reconstruction ;
	#endif
	return 0 ;
}
