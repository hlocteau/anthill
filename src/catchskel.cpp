/**
 * \brief catch skeleton branch from the initial skeleton to a given component of the simplified skeleton
 **/
#include <io/Pgm3dFactory.h>
#include <connexcomponentextractor.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <QMap>
#include <QList>

namespace po = boost::program_options;

void errorAndHelp( const po::options_description & general_opt ) {
	std::cerr 	<< "Catch skeleton branch from the initial skeleton to a given component of the simplified skeleton."<<std::endl
				<< general_opt << "\n";
}
void missingParam ( std::string param )
{
  std::cerr <<" Parameter: "<<param<<" is required.."<<std::endl;
  exit ( 1 );
}

typedef arma::u8  tlabel ;
typedef qint8     qtlabel ;
typedef arma::u16 tlabelbranch ;
typedef qint16    qtlabelbranch ;

void set_branch( const QList< uint * > &touching, const BillonTpl< tlabel > *labelSkel, const BillonTpl< tlabelbranch > *labelBranch, QMap< tlabelbranch, tlabel > &NewLabelBranch, const QList< tlabel > &Labels, QMap< tlabel, QList<tlabel> > &edges ) {
	QList< uint * >::const_iterator iterVoxel = touching.begin(),
	                                iterVoxelEnd = touching.end() ;
	bool bDiscard ;
	QList< tlabelbranch > bridges ;
	while ( iterVoxel != iterVoxelEnd ) {
		tlabelbranch idBranch = (*labelBranch)( (*iterVoxel)[1], (*iterVoxel)[0], (*iterVoxel)[2] ) ;
		tlabel idComp = (*labelSkel)( (*iterVoxel)[1], (*iterVoxel)[0], (*iterVoxel)[2] ) ;
		bDiscard = false ;
		if ( !Labels.isEmpty() ) {
			bDiscard = ( !Labels.contains( idComp ) );
		}
		if ( !bDiscard ) {
			if ( !NewLabelBranch.contains( idBranch ) )
				NewLabelBranch.insert( idBranch, idComp ) ;
			else if (NewLabelBranch[idBranch] != idComp ) {
				bridges.append( idBranch ) ;
				if ( !edges.contains( idComp ) ) edges.insert( idComp, QList<tlabel>() ) ;
				if ( !edges.contains( NewLabelBranch[idBranch] ) ) edges.insert( NewLabelBranch[idBranch], QList<tlabel>() ) ;
				edges[ idComp ].append( NewLabelBranch[idBranch] ) ;
				//edges[ idComp ].append( idBranch ) ;
				edges[ NewLabelBranch[idBranch] ].append( idComp ) ;
				//edges[ NewLabelBranch[idBranch] ].append( idBranch ) ; /// it is this value that is the edge between the two
			}
		} else if ( NewLabelBranch.contains( idBranch ) ) {
			bridges.append( idBranch ) ;
		}
		iterVoxel++ ;
	}
	/// component that get their identifier in bridges are connected to at least two components
	qSort( bridges.begin(), bridges.end(), qLess< tlabelbranch >() ) ;
	while ( !bridges.isEmpty() ) {
		NewLabelBranch.take( bridges.takeFirst() ) ;
	}
	
	for ( QMap<tlabel,QList<tlabel> >::ConstIterator e_source_it = edges.begin() ; e_source_it != edges.end() ; e_source_it++ ) {
		std::cout<<(int)e_source_it.key()<<" connected to ";
		for ( QList<tlabel>::ConstIterator e_target_it = e_source_it.value().begin() ; e_target_it != e_source_it.value().end() ; e_target_it++ )
			std::cout<<(int)*e_target_it<<" " ;
		std::cout<<std::endl;
	}
	
}

void attach_branch( BillonTpl< tlabel > *extendedSkel, const BillonTpl< tlabelbranch > *labelBranch, const QMap< tlabelbranch, tlabel > &NewLabelBranch, const QList< tlabel > &Labels ) {
	BillonTpl< tlabelbranch >::const_iterator iterLabel = labelBranch->begin(),
	                                          iterLabelEnd = labelBranch->end() ;
	BillonTpl< tlabel >::iterator iterExtended = extendedSkel->begin() ;
	while ( iterLabel != iterLabelEnd ) {
		if ( NewLabelBranch.contains( *iterLabel ) ) {
			*iterExtended = NewLabelBranch[ *iterLabel ] ;
		}
		if ( !Labels.isEmpty() ) {
			if ( !Labels.contains( *iterExtended ) )
				*iterExtended = 0 ;
		}
		iterLabel++ ;
		iterExtended++ ;
	}
}

void make_branch( BillonTpl< arma::u8 > *initialSkel, const BillonTpl< arma::u8 > *reducedSkel, 
                  const BillonTpl< tlabel > *reducedLabeledSkel, QList< uint * > &touching ) {
	BillonTpl< arma::u8 >::iterator iterInitial = initialSkel->begin(),
	                                iterInitialEnd = initialSkel->end();
	                                
	BillonTpl< arma::u8 >::const_iterator iterReduced = reducedSkel->begin();
	
	/// step 1 : remove voxels that are activated in both images
	while( iterInitial != iterInitialEnd ) {
		if ( ( (*iterInitial) > 0 ) && ( (*iterReduced) > 0 ) ) {
			*iterInitial = 0 ;
		}
		iterInitial++ ;
		iterReduced++ ;
	}
	
	/// step 2 : identify voxels of the reduced label skeleton being adjacent to a voxel of a branch
	uint x,y,z,neighbor;
	for ( z = 0 ; z < reducedLabeledSkel->n_slices ; z++ )
		for ( y = 0 ; y < reducedLabeledSkel->n_rows ; y++ )
			for ( x = 0 ; x < reducedLabeledSkel->n_cols ; x++ ) {
				if ( (*reducedLabeledSkel)( y, x, z ) == 0 ) continue ;
				for ( neighbor = 0 ; neighbor < 27 ; neighbor++ )
					if ( ( y + (neighbor/9-1)) >= 0 		&& ( y + (neighbor/9-1)) < reducedSkel->n_rows &&
						 ( x + ( (neighbor%9)/3 -1 )) >= 0 	&& ( x + ( (neighbor%9)/3 -1 ))<reducedSkel->n_cols &&
						 ( z + ( neighbor % 3 -1 )) >= 0 	&& ( z + ( neighbor % 3 -1 ))<reducedSkel->n_slices ) {
						if ( (*initialSkel)( y + (neighbor/9-1), x + ( (neighbor%9)/3 -1 ), z + ( neighbor % 3 -1 ) ) ) {
							/// \note we do not update now initialSkel as we may get a side effect on the processing of the next voxels
							touching.append( new uint[3] ) ;
							touching.back()[ 0 ] = x ;
							touching.back()[ 1 ] = y ;
							touching.back()[ 2 ] = z ;
							assert( (*reducedLabeledSkel)( y + (neighbor/9-1), x + ( (neighbor%9)/3 -1 ), z + ( neighbor % 3 -1 ) ) == 0 ) ;
							break ;
						}
					}
			}
	/// step 3 : set active anew voxels being identified during the step 2
	QList< uint * >::ConstIterator voxelIter = touching.begin(),
	                               voxelEnd = touching.end() ;
	while ( voxelIter != voxelEnd ) {
		(*initialSkel)( (*voxelIter)[1], (*voxelIter)[0], (*voxelIter)[2] ) = 1 ;
		voxelIter++ ;
	}
}

int main ( int narg, char **argv ) {
	
	// parse command line ----------------------------------------------
	po::options_description general_opt ( "Allowed options are: " );
	general_opt.add_options()
		( "help,h", "display this message." )
		( "initial,i", po::value< std::string >(), "Initial skeleton pgm filename." )
		( "reduced,r", po::value< std::string >(), "Reduced skeleton pgm filename." )
		( "label,l", po::value< std::string >(), "(part of the) Reduced skeleton being labeled pgm filename." )
		( "selection,s", po::value< std::string >()->multitoken(), "Catch branch touching the given components\' labels of the reduced skeleton.")
		( "output,o", po::value< std::string >(), "Extended labeled skeleton pgm filename.");

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
	if ( ! vm.count("initial") ) missingParam( "initial" ) ;
	if ( ! vm.count("label") ) missingParam( "label" ) ;
	if ( ! vm.count("reduced") ) missingParam( "reduced" ) ;
	if ( ! vm.count("output") ) missingParam( "output" ) ;
	
	std::string initialSkelFile = vm["initial"].as<std::string>() ;
	std::string reducedSkelFile = vm["reduced"].as<std::string>() ;
	std::string reducedLabelSkelFile = vm["label"].as<std::string>() ;
	std::string outputSkelFile  = vm["output"].as<std::string>() ;
	
	QList< tlabel > Labels ;
	if ( vm.count( "selection" ) ) {
		QStringList selectedLabels = QString( "%1").arg( vm["selection"].as< std::string >().c_str() ).split( " ", QString::SkipEmptyParts) ;
		while ( !selectedLabels.isEmpty() )
			Labels.append( (tlabel)selectedLabels.takeAt(0).toInt() ) ;
		qSort( Labels.begin(), Labels.end(), qLess<tlabel>() ) ;
		
		std::cout<<"Selection : ";
		for ( QList< tlabel >::ConstIterator lbl = Labels.begin() ; lbl != Labels.end() ; lbl++ )
			std::cout<<"["<<(int) *lbl<<"] " ;
		std::cout<<std::endl;
	}
	
	BillonTpl< tlabel > *labelReducedSkel ;
	BillonTpl< arma::u8 > *initialSkel,
	                      *reducedSkel ;
	{
		Pgm3dFactory< tlabel > factory ;
		labelReducedSkel = factory.read( QString(reducedLabelSkelFile.c_str()) ) ;
	}
	{
		Pgm3dFactory< arma::u8 > factory ;
		initialSkel = factory.read( QString(initialSkelFile.c_str()) ) ;
		if ( initialSkel->max() > 1 ) *initialSkel /= initialSkel->max() ;
		reducedSkel = factory.read( QString(reducedSkelFile.c_str()) ) ;
		if ( reducedSkel->max() > 1 ) *reducedSkel /= reducedSkel->max() ;
	}
	
	QList< uint * > touching ;
	make_branch( initialSkel, reducedSkel, labelReducedSkel, touching ) ;
	delete reducedSkel ;
	
	ConnexComponentExtractor< arma::u8, tlabelbranch > CCE ;
	BillonTpl< tlabelbranch > * labelBranch = CCE.run( *initialSkel ) ;
	delete initialSkel ;
	
	QMap< tlabelbranch, tlabel > NewLabelBranch ;
	QMap< tlabel, QList<tlabel> > edges ;
	set_branch( touching, labelReducedSkel, labelBranch, NewLabelBranch, Labels,edges ) ;
	attach_branch( labelReducedSkel, labelBranch, NewLabelBranch, Labels ) ;
	
	IOPgm3d< tlabel, qtlabel, false >::write( *labelReducedSkel, QString(outputSkelFile.c_str() ) ) ;
	
	while ( !touching.isEmpty() ) {
		delete [] touching.takeFirst() ;
	}
	delete labelReducedSkel ;
	delete labelBranch ;
	
	return 0 ;
}
