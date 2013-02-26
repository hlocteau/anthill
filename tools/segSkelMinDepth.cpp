/**
 * \brief segmentation of the skeleton based on a min depth value
 */
#include <def_coordinate.h>
#include <SkeletonGraph.hpp>
#include <io/IOPgm3d.h>
#include <io/Pgm3dFactory.h>
#include <io/IOUtils.h>
#include <utils.h>
#include <connexcomponentextractor.h>
#include <ConnexComponentRebuilder.hpp>
#include <geom2d.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <boost/filesystem.hpp>

#include <boost/graph/connected_components.hpp>
#include <rag.hpp>

namespace fs=boost::filesystem;
namespace po=boost::program_options ;

typedef arma::u32 DepthType ;
typedef arma::u32 LabelType ;

typedef struct _TProgramArg {
	fs::path  _skelFilePath ;
	fs::path  _sceneFilePath ;
	fs::path  _depthFilePath ;
	fs::path  _outputFilePath ;
	bool      _high ;
	DepthType _depthThreshold ;
	double_t  _mergeThreshold ;
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
		( "inner,i", po::value<std::string>(), "Input inner scene pgm filename." )
		( "depth,d", po::value<std::string>(), "Input depth map pgm filename." )
		( "output,o", po::value<string>(),"Output pgm filename." )
		( "high,h", po::value<bool>(),"seeds are voxels that have a distance-to-closed-boundary higher than <threshold>.")
		( "low,l", po::value<bool>(),"seeds are voxels that have a distance-to-closed-boundary smaller than <threshold>.")
		( "threshold,t", po::value<int>(),"threshold to be used for classification of skeleton's voxels based on their depth value." )
		( "merge,m", po::value<double_t>()->default_value(0),"threshold to be used for reconstructed connected component." );

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
	if ( ! ( vm.count ( "inner" ) ) )  return missingParam ( "inner" );
	if ( ! ( vm.count ( "output" ) ) ) return missingParam ( "output" );
	if ( ! vm.count( "threshold" ) )   return missingParam ( "threshold" );
	if ( ! vm.count( "low" ) && ! vm.count( "high" ))   return missingParam ( "low|high" );
	params._depthThreshold = vm["threshold"].as<int>() ;
	params._mergeThreshold = vm["merge"].as<double_t>() ;
	params._skelFilePath = vm["skel"].as<std::string>();
	params._depthFilePath = vm["depth"].as<std::string>();
	params._sceneFilePath = vm["inner"].as<std::string>();
	params._outputFilePath = vm["output"].as<std::string>();
	if ( vm.count("high") )
		params._high = vm["high"].as<bool>();
	else
		params._high = ! vm["low"].as<bool>();
	return true ;
}


void export2dot( GraphAdj & g, QString filename ) ;

template < typename T > BillonTpl< T > * do_labeling( SG_u8 &sg ) {
	typedef T elem_type ;
	
	const SG_u8::graph_t & graph = sg.graph() ;
	int n_rows, n_cols, n_slices ;
	sg.size( n_rows, n_cols, n_slices ) ;
	BillonTpl< elem_type > * label = new BillonTpl< elem_type > ( n_rows, n_cols, n_slices ) ;
	label->fill( 0 ) ;
	
	std::vector<elem_type> component(num_vertices(graph));
	elem_type num = connected_components(graph, &component[0]);
	for ( SkeletonGraph<arma::u8>::ConstVoxelIterator it = sg.encoding_begin() ; it != sg.encoding_end() ; it++ ) {
		iCoord3D voxel = sg.from_linear_coord( it.key() ) ;
		(*label)( voxel.y, voxel.x, voxel.z ) = component[ it.value() ]+1 ;
	}
	return label ;
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
							uint k =  0;
							uint end = touching[ cur_value ].size() ;
							while ( k < end ) {
								if ( touching[ cur_value ].at(k) >= adj_value ) break ;
								k++ ;
							}
							if ( k < end ) {
								if ( touching[ cur_value ].at(k) != adj_value )
									touching[ cur_value ].insert( k, adj_value ) ;
							} else
								touching[ cur_value ].append( adj_value ) ;
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

template <typename T > void merge_adjacent_cc( BillonTpl< T > *label, QMap< uint32_t,uint32_t > &volumes ) {
	typedef T elem_type ;
	
	QMap< elem_type, QList< elem_type > >           touching ;
	QMap< elem_type, elem_type >                    tableEquiv ;
	typename QMap< elem_type, QList< elem_type > >::iterator iterRewrite ;
	elem_type nSeeds = label->max() ;
	extract_adjacency( *label, touching, nSeeds ) ;
	
	for ( iterRewrite = touching.begin() ; iterRewrite != touching.end() ; iterRewrite++ ) {
		std::cout<<__FUNCTION__<<" @ line "<<__LINE__<<" "<<cast_integer<T, int>(iterRewrite.key())<<" : ";
		for ( int k=0;k<iterRewrite.value().size();k++ )
			std::cout<<cast_integer<T, int>(iterRewrite.value().at(k))<<" ";
		std::cout<<std::endl;
	}
	
	for ( iterRewrite = touching.begin() ; iterRewrite != touching.end() ;  ) {
		if ( iterRewrite.key() > iterRewrite.value().at(0) ) {
			touching[ iterRewrite.value().at(0) ].append( iterRewrite.key() ) ;
			touching[ iterRewrite.value().at(0) ].append( iterRewrite.value() ) ;
			qSort( touching[ iterRewrite.value().at(0) ].begin(), touching[ iterRewrite.value().at(0) ].end(), qLess<T>() ) ;
			iterRewrite = touching.erase( iterRewrite ) ;
		} else
			iterRewrite++;
	}

	for ( iterRewrite = touching.begin() ; iterRewrite != touching.end() ; iterRewrite++ ) {
		/**
		 * \see code being used in connexcomponentextractor.ih
		 */
		QList< elem_type > &voisinage = iterRewrite.value() ;
		elem_type mini = iterRewrite.key() ;
		elem_type currentEquiv ;
		
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

	QMapIterator<elem_type, elem_type> iter(tableEquiv) ;
	while ( iter.hasNext() ) {
		iter.next();
		if ( tableEquiv.contains(iter.value())) {
			tableEquiv[iter.key()]=tableEquiv[iter.value()];
		}
	}

	QList<elem_type> terminal ;
	/// insert isolated cc
	for ( elem_type k=1;k<nSeeds;k++ ) {
		if ( !tableEquiv.contains( k ) ) {
			tableEquiv[k]=k;
			terminal.append(k) ;
		} else {
			if ( tableEquiv[k] == k )
				terminal.append(k) ;
		}
	}
	
	QMap< uint32_t,uint32_t > newvolumes ;
	
	/// minimal distinct values
	for ( elem_type k = 1 ; k < nSeeds ; k++ ) {
		tableEquiv[ k ] = terminal.indexOf( tableEquiv[ k ] ) + 1;
		if ( !newvolumes.contains( tableEquiv[k] ) ) newvolumes.insert( tableEquiv[k] , (uint32_t) 0 ) ;
		newvolumes[ tableEquiv[k] ] += volumes[ tableEquiv[k] ] ;
	}
	
	apply_translation<elem_type>( *label, tableEquiv ) ;
	
	volumes = newvolumes ;
	
}

template <typename T > BillonTpl< LabelType > * do_labeling_complement( const BillonTpl<T> &ccLabelSeeds, QString filename ) {
	Pgm3dFactory< arma::u8 > factory ;
	BillonTpl< arma::u8 > *sceneimg = factory.read( filename ) ;
	
	int n_rows = sceneimg->n_rows,
	    n_cols = sceneimg->n_cols,
	    n_slices = sceneimg->n_slices ;

	typename BillonTpl< T >::const_iterator  iterLabelCC = ccLabelSeeds.begin() ;
	BillonTpl< arma::u8 >::iterator          iterRemain = sceneimg->begin(),
									         iterRemainEnd = sceneimg->end() ;
	while ( iterRemain != iterRemainEnd ) {
		if ( *iterLabelCC != 0 )
			assert( *iterRemain != 0 ) ;
		if ( *iterRemain ) {
			if ( *iterLabelCC != 0 )
				*iterRemain = 0 ;
		}
		iterLabelCC++;
		iterRemain++;
	}
	ConnexComponentExtractor< arma::u8, LabelType > CCERemain ;
	BillonTpl< LabelType > *lblRemain = new BillonTpl< LabelType > ( *CCERemain.run( *sceneimg ) );
	delete sceneimg ;
	return lblRemain ;
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






void iterative_merge( BillonTpl< LabelType > &labelComp, LabelType nSeeds, QMap< uint32_t,uint32_t > &volumes) {
	QMap< LabelType, QList< LabelType > > adjacencies ;
	QList< LabelType >::iterator adj ;
	QMap< LabelType, LabelType > tableEquiv ;
	extract_adjacency( labelComp, adjacencies, (LabelType)1 ) ;
	
	
	trace.info() << "== Volumes | Adjacencies =="<<std::endl;
	for ( QMap< uint32_t, uint32_t >::iterator iterVol = volumes.begin() ; iterVol != volumes.end() ; iterVol++ ) {
		trace.info() <<"cc # "<<(int) iterVol.key()<<" : "<<(int) iterVol.value()<<" | ";
		if ( !adjacencies.contains( iterVol.key() ) ) { std::cerr<<"0"<<std::endl;continue ;}
		//for ( adj = adjacencies[ iterVol.key() ].begin() ; adj != adjacencies[ iterVol.key() ].end() ; adj++ )
		//	trace.info()<<*adj<<" " ;
		std::cerr<<adjacencies[ iterVol.key() ].size()<<std::endl;
		trace.info()<<std::endl;
	}
	
	
	
	/// a vertex - not corresponding to a seed - having degree 1 has to be merged
	LabelType nRemains = 1 ;
	QMap< uint32_t,uint32_t > newvolumes ;
	QMap< LabelType, QList< LabelType > > newadjacencies ;
	
	std::cerr<<"Size of volume is "<<volumes.size()<<" @ line "<<__LINE__<<std::endl;
	
	for ( LabelType vertex = labelComp.max() ; vertex > nSeeds ; vertex-- ) {
		if ( !adjacencies.contains( vertex ) ) {
			/// exists?
			continue ;
		}
		if ( adjacencies[ vertex ].size() == 1 ) {
			LabelType theSeed = adjacencies[ vertex ].at(0) ;
			assert( theSeed <= nSeeds ) ;
			tableEquiv[ vertex ] = theSeed ;
			if ( !newvolumes.contains( theSeed ) ) newvolumes.insert( theSeed, (uint32_t ) 0 ) ;
			newvolumes[ theSeed ] += volumes[ vertex ] ;
			volumes.remove( vertex ) ;
			adjacencies[ theSeed ].removeAt( adjacencies[ theSeed ].indexOf(vertex) ) ;
			assert( !adjacencies[ theSeed ].isEmpty() ) ;
			adjacencies.remove( vertex ) ;
		} else {
			tableEquiv[ vertex ] = nSeeds + nRemains ;
			newvolumes.insert( nSeeds + nRemains, volumes[ vertex ] ) ;
			volumes.remove( vertex ) ;
			nRemains++ ;
		}
	}
	std::cerr<<"Number of non-seed regions remaining "<<nRemains-1<<std::endl;
	std::cerr<<"Size of volume is "<<volumes.size()<<" @ line "<<__LINE__<<std::endl;
	if ( !tableEquiv.isEmpty() ) {
		std::cout<<tableEquiv.size()<<" merge operation(s)"<<std::endl;
		apply_translation( labelComp, tableEquiv ) ;
		save_minspace<LabelType>( labelComp, "/tmp/step0.pgm3d" ) ;
		
		for ( QMap< LabelType, QList< LabelType > >::ConstIterator vertex = adjacencies.begin() ; vertex != adjacencies.end() ; vertex++ ) {
			LabelType key = vertex.key() ;
			LabelType value ;
			if ( tableEquiv.contains( vertex.key() ) ) key = tableEquiv[ vertex.key() ] ;
			newadjacencies.insert( key, QList< LabelType >() ) ;
			for ( uint k = vertex.value().size() ; k > 0 ; k-- ) {
				value = vertex.value().at(k-1) ;
				if ( tableEquiv.contains( vertex.value().at(k-1) ) )
					value = tableEquiv[ vertex.value().at(k-1) ] ;
				newadjacencies[ key ].append( value ) ;
			}
			qSort( newadjacencies[ key ] ) ;
		}
		adjacencies = newadjacencies ;
	}
	
	std::cerr<<"Newvolumes first keys is "<<newvolumes.begin().key()<<std::endl;
	
	while ( !newvolumes.isEmpty() ) {
		if ( !volumes.contains( newvolumes.begin().key() ) ) volumes.insert( newvolumes.begin().key(), (uint32_t) 0 ) ;
		volumes[ newvolumes.begin().key() ] += newvolumes.begin().value() ;
		
		if ( newvolumes.size() == 1 ) std::cerr<<"Newvolumes last keys is "<<newvolumes.begin().key()<<std::endl;
		
		newvolumes.erase( newvolumes.begin() ) ;
	}
	trace.info() << "== Volumes =="<<std::endl;
	for ( QMap< uint32_t, uint32_t >::iterator iterVol = volumes.begin() ; iterVol != volumes.end() ; iterVol++ ) {
		std::cerr <<"cc # "<<cast_integer<uint32_t,int32_t>( iterVol.key() )<<" : "<<cast_integer<uint32_t,int64_t>( iterVol.value() ) ;
		if ( !adjacencies.contains( iterVol.key() ) ) { std::cerr<<" 0"<<std::endl; continue ; }
		std::cerr<<" "<<adjacencies[ iterVol.key() ].size()<<" ";
		//for ( adj = adjacencies[ iterVol.key() ].begin() ; adj != adjacencies[ iterVol.key() ].end() ; adj++ )
		//	std::cerr<<*adj<<" " ;
		std::cerr<<std::endl;
	}
	
	
	/// attach small region's voxels to a bigger one in it neighborhood

	
	
}

template <typename T>
void insert_priority( const QMap< uint, T > &f_map, const GraphAdj::vertex_descriptor v, QList< GraphAdj::vertex_descriptor > &priority,boost::property_map< GraphAdj, _NodeTag >::type &node_map ) {
	uint pos = 0 ;
	uint length = priority.size() ;
	while ( pos != length ) {
		if ( f_map[ node_map[ v ].id() ] <= f_map[ node_map[ priority.at( pos ) ].id() ] ) break ;
		pos++ ;
	}
	priority.insert( pos, v ) ;
}

double_t priority_feature_0( GraphAdj::vertex_descriptor v, GraphAdj &g, 
					boost::property_map< GraphAdj, _NodeTag >::type &node_map, 
					boost::property_map< GraphAdj, _EdgeTag >::type &edge_map ) {
	return (double_t)cast_integer<uint, int32_t>( node_map[ v ].volume() ) ;
}

template <typename T>
GraphAdj::vertex_descriptor newton_biggest_feature_0( GraphAdj &g, const QMap< uint, T > &f_map, const GraphAdj::vertex_descriptor v,boost::property_map< GraphAdj, _NodeTag >::type &node_map ) {
	GraphAdj::vertex_descriptor newton ;
	GraphAdj::adjacency_iterator adj, adj_end ;
	boost::tie( adj, adj_end ) = boost::adjacent_vertices( v, g) ;
	newton = *adj ;
	for ( ; adj != adj_end ; adj++ )
		if ( f_map[ node_map[ *adj ].id() ] > f_map[ node_map[ newton ].id() ] )
			newton = *adj ;
	return newton ;
}

template <typename T>
GraphAdj::vertex_descriptor newton_smallest_feature_0( GraphAdj &g, const QMap< uint, T > &f_map, const GraphAdj::vertex_descriptor v,boost::property_map< GraphAdj, _NodeTag >::type &node_map ) {
	GraphAdj::vertex_descriptor newton ;
	GraphAdj::adjacency_iterator adj, adj_end ;
	boost::tie( adj, adj_end ) = boost::adjacent_vertices( v, g) ;
	newton = *adj ;
	for ( ; adj != adj_end ; adj++ )
		if ( f_map[ node_map[ *adj ].id() ] < f_map[ node_map[ newton ].id() ] )
			newton = *adj ;
	return newton ;
}

template <typename T>
GraphAdj::vertex_descriptor newton_biggest_feature_1( GraphAdj &g, const QMap< uint, T > &f_map, 
					const GraphAdj::vertex_descriptor v,
					boost::property_map< GraphAdj, _NodeTag >::type &node_map, 
					boost::property_map< GraphAdj, _EdgeTag >::type &edge_map) {
	GraphAdj::vertex_descriptor newton ;
	
	NodeData & ndv = node_map[ v ] ;
	
	GraphAdj::in_edge_iterator in_edge, in_edge_end ;
	boost::tie( in_edge, in_edge_end ) = boost::in_edges( v, g) ;
	
	double_t ratio, min_ratio = -1 ;
	/// there we want to retrieve an edge leading to the stored feature.
	/// if several edges are elected, we select the one having the biggest opposite node
	for ( ; in_edge != in_edge_end ; in_edge++ ) {
		EdgeData & ed = edge_map[ *in_edge ] ;
		ratio =1-ed.per_side( ndv.id() )->size() / (double_t)cast_integer<uint, int32_t>( ndv.volume() ) ;
		if ( ratio == f_map[ ndv.id() ] ) {
			if ( min_ratio < 0 )
				newton = ( boost::source( *in_edge, g ) == v ? boost::target( *in_edge, g ) : boost::source( *in_edge, g ) ) ;
			else {
				if ( node_map[ newton ].volume() < node_map[ ( boost::source( *in_edge, g ) == v ? boost::target( *in_edge, g ) : boost::source( *in_edge, g ) ) ].volume() )
					newton = ( boost::source( *in_edge, g ) == v ? boost::target( *in_edge, g ) : boost::source( *in_edge, g ) ) ;
			}
		}
	}		
	return newton ;
}

template <typename T>
GraphAdj::vertex_descriptor newton_smallest_feature_1( GraphAdj &g, const QMap< uint, T > &f_map, 
					const GraphAdj::vertex_descriptor v,
					boost::property_map< GraphAdj, _NodeTag >::type &node_map, 
					boost::property_map< GraphAdj, _EdgeTag >::type &edge_map) {
	GraphAdj::vertex_descriptor newton ;
	
	NodeData & ndv = node_map[ v ] ;
	
	GraphAdj::in_edge_iterator in_edge, in_edge_end ;
	boost::tie( in_edge, in_edge_end ) = boost::in_edges( v, g) ;
	
	double_t ratio, min_ratio = -1 ;
	/// there we want to retrieve an edge leading to the stored feature.
	/// if several edges are elected, we select the one having the biggest opposite node
	for ( ; in_edge != in_edge_end ; in_edge++ ) {
		EdgeData & ed = edge_map[ *in_edge ] ;
		ratio =1-ed.per_side( ndv.id() )->size() / (double_t)cast_integer<uint, int32_t>( ndv.volume() ) ;
		if ( ratio == f_map[ ndv.id() ] ) {
			if ( min_ratio < 0 )
				newton = ( boost::source( *in_edge, g ) == v ? boost::target( *in_edge, g ) : boost::source( *in_edge, g ) ) ;
			else {
				if ( node_map[ newton ].volume() > node_map[ ( boost::source( *in_edge, g ) == v ? boost::target( *in_edge, g ) : boost::source( *in_edge, g ) ) ].volume() )
					newton = ( boost::source( *in_edge, g ) == v ? boost::target( *in_edge, g ) : boost::source( *in_edge, g ) ) ;
			}
		}
	}		
	return newton ;
}

void set_impact_0( GraphAdj &g, GraphAdj::vertex_descriptor, QList< GraphAdj::vertex_descriptor > &needUpdate ) {
	return ;
}

void set_impact_1( GraphAdj &g, GraphAdj::vertex_descriptor v, QList< GraphAdj::vertex_descriptor > &needUpdate ) {
	GraphAdj::adjacency_iterator adj, adj_end ;
	boost::tie( adj, adj_end ) = boost::adjacent_vertices( v, g) ;
	for ( ; adj != adj_end ; adj++ )
	needUpdate.append( *adj ) ;
}

double_t priority_feature_1( GraphAdj::vertex_descriptor v, GraphAdj &g, 
					boost::property_map< GraphAdj, _NodeTag >::type &node_map, 
					boost::property_map< GraphAdj, _EdgeTag >::type &edge_map ) {
	NodeData & nd = node_map[ v ] ;
	if ( nd.volume() == 0 ) return -1 ;

	GraphAdj::in_edge_iterator in_edge, in_edge_end ;
	boost::tie( in_edge, in_edge_end ) = boost::in_edges(v, g) ;
	double_t ratio, min_ratio = 2 ;
	GraphAdj::vertex_descriptor vertex_max_ratio ;
	for ( ; in_edge != in_edge_end ; in_edge++ ) {
		EdgeData & ed = edge_map[ *in_edge ] ;
		ratio =1-ed.per_side( nd.id() )->size() / (double_t)cast_integer<uint, int32_t>( nd.volume() ) ;
		if ( ratio < min_ratio ) {
			min_ratio = ratio ;
			if ( boost::source( *in_edge, g ) == v ) vertex_max_ratio = boost::target( *in_edge, g ) ;
			else vertex_max_ratio = boost::source( *in_edge, g ) ;
		}
	}
	std::cerr<<"Node "<<nd.id()<<" [vol="<<nd.volume()<<"] may be merge with node "<<node_map[ vertex_max_ratio ].id()<<" [vol="<<node_map[ vertex_max_ratio ].volume()<<"] ratio "<<min_ratio<<std::endl;
	return min_ratio ;
}

/**
 * \warning if the threshold on size is too high, we will only get rooms!
 * \todo use a ratio (number of voxels on the bridge between a pair of regions) / ( total number of voxels in the smallest region of the pair)
 */
void threshold_size( BillonTpl< LabelType > &labelComp, LabelType nSeeds, double_t th ) {
	GraphAdj * pg = init_rag( labelComp, (LabelType)1 ) ;
	std::cout<<"graph created"<<std::endl;
	boost::property_map< GraphAdj, _NodeTag >::type node_map  = boost::get( _NodeTag(), *pg ) ;
	boost::property_map< GraphAdj, _EdgeTag >::type edge_map  = boost::get( _EdgeTag(), *pg ) ;
	
	GraphAdj::vertex_iterator vi, vi_end ;
	GraphAdj::vertex_descriptor v ;
	boost::tie( vi,vi_end) = boost::vertices( *pg ) ;
	QList< GraphAdj::vertex_descriptor > sorted_vertices ;
	QMap< uint, double_t > f_map ;
	/// set label and define priority queue
	for ( ; vi != vi_end ; vi++ ) {
		v = * vi ;
		NodeData & nd = node_map[ v ] ;
		if ( nd.id() <= nSeeds ) nd.setCategory( 2 ) ;
		else nd.setCategory( 1 ) ;
		f_map[ nd.id() ] = priority_feature_1( v, *pg, node_map, edge_map ) ;			/// CHARACTERISTIC
		insert_priority( f_map, v, sorted_vertices,node_map ) ;
	}
	std::cout<<"labels' nodes and priority queue defined"<<std::endl;
	uint pos = 0 ;
	GraphAdj::adjacency_iterator adj, adj_end ;
	GraphAdj::vertex_descriptor newton;
	QMap< LabelType, LabelType > tableEquiv ;
	QMap< LabelType, LabelType > tableEquivRemain ;
	QList<GraphAdj::vertex_descriptor> needUpdate ;
	
	while ( f_map[ node_map[ sorted_vertices.at( pos ) ].id() ] < th ) {
		if ( /*f_map[ node_map[ sorted_vertices.at( pos ) ].id() ] > 0 &&*/ node_map[ sorted_vertices.at( pos ) ].volume() > 0 ) {
			v = sorted_vertices.at( pos ) ;
			set_impact_1( *pg, v, needUpdate ) ;										/// INFLUENCE
			newton = newton_biggest_feature_1( *pg, f_map, v, node_map, edge_map ) ;	/// SELECTION
			needUpdate.push_back( newton ) ;
			NodeData & ndb = node_map[ newton ] ;
			tableEquiv[ node_map[ sorted_vertices.at( pos ) ].id() ] = ndb.id() ;
			merge_nodes( newton, v, *pg ) ;

			/// update its priority
			while ( ! needUpdate.isEmpty() ) {
				newton = needUpdate.takeFirst() ;
				f_map[ node_map[ newton ].id() ] = priority_feature_1( newton, *pg, node_map, edge_map ) ;	/// CHARACTERISTIC
				/// update position of newton
				uint pos_newton = 0 ;
				while ( sorted_vertices.at( pos_newton ) != newton ) {
					pos_newton++ ;
					assert( pos_newton <sorted_vertices.size() ) ;
				}
				sorted_vertices.removeAt( pos_newton ) ;
				insert_priority( f_map, newton, sorted_vertices,node_map ) ;
			}
		} else {
			pos++ ;
		}
	}


	for ( pos = sorted_vertices.size() ; pos > 0 ; pos-- ) {
		NodeData & nd = node_map[ sorted_vertices.at( pos-1 ) ] ;
		if ( nd.id() == -1 ) continue ;
		if ( nd.volume() == 0 ) break ;
		tableEquivRemain[ nd.id() ] = tableEquivRemain.size()+1 ;
	}

	export2dot( *pg, "/tmp/sample.dot" ) ;	
	
	
	
	for ( QMap< LabelType, LabelType >::iterator iter = tableEquiv.begin() ; iter != tableEquiv.end() ; iter++ ) {
		LabelType value = iter.value() ;
		while ( tableEquiv.contains( value ) )
			value = tableEquiv.find( value ).value() ;
		assert( tableEquivRemain.contains( value ) ) ;
		tableEquivRemain[ iter.key() ] = tableEquivRemain[ value ] ;
	}
	apply_translation( labelComp, tableEquivRemain ) ;
	save_minspace< LabelType >( labelComp, "/tmp/stepFinal.pgm3d" ) ;
	delete pg ;
}

void export2dot( GraphAdj & g, QString filename ) {
	boost::property_map< GraphAdj, _NodeTag >::type node_map  = boost::get( _NodeTag(), g ) ;
	boost::property_map< GraphAdj, _EdgeTag >::type edge_map  = boost::get( _EdgeTag(), g ) ;
	
	std::ofstream dot_file( filename.toStdString().c_str());
	dot_file << "graph D {\n"
		<< "  size=\"4,3\"\n"
		<< "  ratio=\"fill\"\n"
		<< "  edge[style=\"bold\"]\n" << "  node[shape=\"circle\"]\n";
	GraphAdj::vertex_iterator ni, ni_end;
	for (boost::tie(ni, ni_end) = boost::vertices(g); ni != ni_end; ++ni) {
		if ( node_map[ *ni ].id() > 0 && node_map[*ni].volume() > 0)
			dot_file << node_map[ *ni ].id()<<" [label=\""<<node_map[*ni].category()<<"\",volume=\""<< node_map[*ni].volume()<<"\"]"<<std::endl;
	}
	GraphAdj::edge_iterator ei, ei_end;
	for (boost::tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei) {
		GraphAdj::edge_descriptor e = *ei;
		GraphAdj::vertex_descriptor u = boost::source(e, g), v = boost::target(e, g);
		dot_file << node_map[ u ].id() << " -- " << node_map[ v ].id() << std::endl;
	}
	dot_file << "}";
	dot_file.close() ;
}

#define SAFETY_MEMORY_CONSUPTION

int main( int narg, char **argv ) {
	TProgramArg params ;
	if ( !process_arg( narg, argv, params ) ) return -1 ;
	
	BillonTpl< arma::u8 > * skelimg = load_maincc( QString( params._skelFilePath.c_str() ) ) ;
	BillonTpl< DepthType > * depthimg = load_data_withmask< arma::u32>( QString( params._depthFilePath.c_str() ), skelimg ) ;
	delete skelimg ;
	{
		GrayLevelHistogram< DepthType > h( *depthimg ) ;
		for ( GrayLevelHistogram< DepthType >::THistogram::iterator bin = h._bin.begin() ; bin != h._bin.end() ; bin++ )
			std::cout<<(int) bin->first<<" : "<<(int) bin->second<<std::endl;
	}	

	///
	/// Initialization
	///
	trace.beginBlock("Preprocessing inputs");
		BillonTpl< arma::u8 > * seedImg ;
		if ( params._high ) seedImg = filter_high<DepthType>( *depthimg, params._depthThreshold ) ;
		else                seedImg = filter_low<DepthType>( *depthimg, params._depthThreshold ) ;
		delete depthimg ;
	trace.endBlock() ;
	
	///
	/// Extracting seeds
	///
	trace.beginBlock("Extracting seeds");
		SG_u8 *sg_seed = new SG_u8( *seedImg, 1 ) ;
		BillonTpl< LabelType > *labelSeed = do_labeling<LabelType>( *sg_seed ) ;
		delete sg_seed ;
		save_minspace<LabelType>( *labelSeed, QString( "/tmp/seeds.pgm3d" ) ) ;
		delete seedImg ;
	trace.endBlock() ;
	
	///
	/// reconstruct each individual connected component being a seed
	///
	trace.beginBlock("Reconstructing seeds") ;
		typedef ConnexComponentRebuilder< LabelType, DepthType, LabelType > CCRType ;
		CCRType CCR( *labelSeed );
		CCR.setDepth( QString( params._depthFilePath.c_str() ) ) ;
		LabelType nSeeds = labelSeed->max() ;
		CCR.run( true ) ;
		delete labelSeed ;
		labelSeed = new BillonTpl< LabelType >( CCR.result() ) ;
		QMap< uint32_t, uint32_t > volumes( CCR.volumes() );
		save_minspace<LabelType>( *labelSeed, QString( "/tmp/rebuild.seeds.pgm3d" ) ) ;
		merge_adjacent_cc<LabelType>( labelSeed, volumes ) ;
		
		trace.info() << "== Volumes seeds (2) =="<<std::endl;
		for ( QMap< uint32_t, uint32_t >::iterator iterVol = volumes.begin() ; iterVol != volumes.end() ; iterVol++ )
			trace.info() <<"cc # "<<(int) iterVol.key()<<" : "<<cast_integer<uint32_t,int64_t>( iterVol.value() )<<std::endl;
		
		save_minspace<LabelType>( *labelSeed, QString( "/tmp/rebuild.seeds.disconnected.pgm3d" ) ) ;
		nSeeds = labelSeed->max() ;
	trace.endBlock() ;
	
	trace.beginBlock("Processing remaining components");
		BillonTpl< LabelType > *labelComp = do_labeling_complement<LabelType>( *labelSeed, QString( params._sceneFilePath.c_str() ) ) ;
		{
			BillonTpl<LabelType>::iterator iterResult = labelComp->begin(),
			                               iterResultEnd = labelComp->end(),
			                               iterSeed = labelSeed->begin() ;
			LabelType hugeCC = -1 ;
			while ( iterResult != iterResultEnd ) {
				if ( *iterResult ) *iterResult += nSeeds ;
				if ( *iterResult ) {
					if ( !volumes.contains( *iterResult ) ) volumes.insert( (uint32_t) *iterResult, (uint32_t) 1 ) ;
					else volumes[ *iterResult ]++ ;
					if ( hugeCC == -1 ) hugeCC = *iterResult ;
					else if ( volumes[ *iterResult ] > volumes[ hugeCC ] ) hugeCC = *iterResult ;
				}
				*iterResult += *iterSeed ;
				
				iterResult++ ;
				iterSeed++ ;
			}
			std::cout<<"Biggest component in the complement : "<< hugeCC<<" with volume "<<volumes[ hugeCC]<<std::endl;
		}
		save_minspace<LabelType>( *labelComp, QString( params._outputFilePath.c_str() ) ) ;
	trace.endBlock() ;
	
	trace.beginBlock("Simplification of the scene");
		//iterative_merge( *labelComp, nSeeds, volumes );
		threshold_size( *labelComp, nSeeds, params._mergeThreshold ) ;
	trace.endBlock() ;
	
	
	delete labelComp ;
	delete labelSeed ;
	
#if 0
	///
	/// Extracting candidate merge
	///
	std::cout<<(int)to<<std::endl<<"extracting adjacency relations..."<<std::endl;
	QMap< arma::u32, QList< arma::u32 > > touching ;
	extract_adjacency( CCR.result(), touching, nSeeds ) ;
	
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
	
	
	touching.clear() ;
	
	/// save before renaming wrt adjacency relations
	if ( CCR.result().max() & 0x00ff )
		IOPgm3d< arma::u32, qint8, false >::write( CCR.result(), QString( "/tmp/rebuildfarbound.pgm3d" ) ) ;
	else if ( label->max() & 0x0000ffff )
		IOPgm3d< arma::u32, qint16, false >::write( CCR.result(), QString( "/tmp/rebuildfarbound.pgm3d" ) ) ;
	else
		IOPgm3d< arma::u32, qint32, false >::write( CCR.result(), QString( "/tmp/rebuildfarbound.pgm3d" ) ) ;
	
	/// remaining part is attached iff it is connected to a single region



	save_minspace( ccRooms, QString( params._outputFilePath.c_str() ) ) ;
	
	delete label ;
#endif
	return 0 ;
}
