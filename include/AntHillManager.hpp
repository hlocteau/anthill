#ifndef ANT_HILL_PROJECT_MANAGER_HEADER
#define ANT_HILL_PROJECT_MANAGER_HEADER
#include <stdint.h>
#include <io/AntHillFile.hpp>
#include <QVector>
#include <def_billon.h>
#include <billon.h>
#include <interval.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#define SIGNED_TINY_INT		1
#define UNSIGNED_TINY_INT	2
#define SIGNED_SHORT_INT	3
#define UNSIGNED_SHORT_INT	4
#define SIGNED_INT			5
#define UNSIGNED_INT		6

namespace fs = boost::filesystem ;

class AntHillManager {
public:
	enum { SLICING_ON_X, SLICING_ON_Y, SLICING_ON_Z };
	AntHillManager() {_currentSeries=-1;_project = 0;};
	~AntHillManager();
	void importDicom( const QString &folderName ) ;
	bool load( const QString &name ) ;
	void openInitialInput( Billon ** img ) ;
	bool binarization( const Interval<arma::s16> &range, int th) ;
	QVector< QString >::ConstIterator series_begin() const {
		return _series.begin() ;
	}
	QVector< QString >::ConstIterator series_end() const {
		return _series.end() ;
	}
	
	fs::path defaultProjectLocation( ) const ;
	fs::path projectLocation( ) const {
		return _projectLocation ;
	}
	void setFileName( const QString &filename ) ;
	AntHillFile * project() {
		return _project ;
	}
	template <typename T,typename U> bool draw( const QString &resname, arma::Mat<U> &image, uint8_t axis, uint16_t coordinate, const Interval<T> &range,bool normalize=true ) ;
	template <typename T,typename U> void draw( const BillonTpl< T > *data, arma::Mat<U> &image, uint8_t axis, uint16_t coordinate, const Interval<T> &range,bool normalize=true ) ;
	template <typename T> BillonTpl<T> * getRessourceByUID( const QString &uid, bool &valid ) ;
	void reset() ;
	
	bool isContentOnly           ( QMap< QString, QString >::ConstIterator &res ) const ;
	bool isColorSelectionAllowed ( QMap< QString, QString >::ConstIterator &res ) const ;
	bool isColor                 ( QMap< QString, QString >::ConstIterator &res ) const ;
	
	QString uid ( QMap< QString, QMap< QString, QString> >::ConstIterator process,
					QMap< QString, QString >::ConstIterator param ) const {
		return QString("%1:%2").arg(process.key()).arg(param.key()) ;
	}
	
	QString inputuid ( ) const {
		return QString("%1:%2").arg( ANTHILL_OVERALL_INPUT_PROCESS_NAME ).arg( ANTHILL_DEFAULT_OUTPUT_NAME ) ;
	}
	
	void getOffset( QMap< QString, QString >::ConstIterator &res, uint &row, uint &col, uint &slice ) const ;
	void getSize( QMap< QString, QString >::ConstIterator &res, uint &n_rows, uint &n_cols, uint &n_slices ) const ;
	
	bool getSize( uint &n_rows, uint &n_cols, uint &n_slices ) ;
	template <typename T> bool getRange( Interval< T > &range ) ;
	/**
	 * \todo we should not open ressources for this operation. This information has to be saved in the projec file
	 */
	template <typename T> bool getRange( const QString &resname, Interval< T > &range ) ;
	bool load_ressource( const QString &resname ) ;
private:
	typedef struct _TImageProperty {
		void*	_adr ;
		uint8_t _type ;
		uint8_t _meaning ;
		uint    *_size ;
		uint	*_offset ;
	} TImageProperty ;
	
	fs::path _projectLocation ;
	fs::path _filename ;
	QVector< QString > _series ;
	uint _currentSeries ;
	QList< BillonTpl< arma::s8 > * >  _im_8s ;
	QList< BillonTpl< arma::u8 > * >  _im_8u ;
	QList< BillonTpl< arma::s16 > * > _im_16s ;
	QList< BillonTpl< arma::u16 > * > _im_16u ;
	QList< BillonTpl< arma::s32 > * > _im_32s ;
	QList< BillonTpl< arma::u32 > * > _im_32u ;
	QMap< QString, TImageProperty >   _prop ;
    AntHillFile *_project ;
} ;

template <typename T> bool AntHillManager::getRange( Interval< T > &range ) {
	QString main_input_name = QString("%1:%2").arg( ANTHILL_OVERALL_INPUT_PROCESS_NAME ).arg( ANTHILL_DEFAULT_OUTPUT_NAME ) ;
	return getRange<T>( main_input_name, range ) ;
}

template <typename T> bool AntHillManager::getRange( const QString &resname, Interval< T > &range ) {
	if ( !load_ressource( resname ) ) return false ;
	switch ( _prop[ resname ]._type ) {
	case SIGNED_TINY_INT :
		{
			BillonTpl< arma::s8 > *img = (BillonTpl< arma::s8 >*) _prop[ resname ]._adr ;
			range.setBounds( img->min(), img->max() ) ;
		}
		break;
	case UNSIGNED_TINY_INT :
		{
			BillonTpl< arma::u8 >  *img = (BillonTpl< arma::u8>*) _prop[ resname ]._adr ;
			range.setBounds( img->min(), img->max() ) ;
		}
		break;
	case SIGNED_SHORT_INT :
		{
			BillonTpl< arma::s16 > *img = (BillonTpl< arma::s16>*) _prop[ resname ]._adr ;
			range.setBounds( img->min(), img->max() ) ;
		}
		break;
	case UNSIGNED_SHORT_INT :
		{
			BillonTpl< arma::u16 >  *img = (BillonTpl< arma::u16>*) _prop[ resname ]._adr ;
			range.setBounds( img->min(), img->max() ) ;
		}
		break;
	case SIGNED_INT :
		{
			BillonTpl< arma::s32 >  *img = (BillonTpl< arma::s32>*) _prop[ resname ]._adr ;
			range.setBounds( img->min(), img->max() ) ;
		}
		break;
	case UNSIGNED_INT :
		{
			BillonTpl< arma::u32 >  *img = (BillonTpl< arma::u32>*) _prop[ resname ]._adr ;
			range.setBounds( img->min(), img->max() ) ;
		}
		break;
	default:
		std::cerr<<"[ Error ] : unmanaged type "<< _prop[ resname ]._type <<" for the project file wrt the ressource "<<resname.toStdString()<<" Function "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
		return false;
	}
	return true ;
}

template <typename T> BillonTpl<T> * AntHillManager::getRessourceByUID( const QString &uid, bool &valid ) {
	if ( !_prop.contains( uid ) ) {
		if (!load_ressource( uid ) ) {
			valid = false ;
			return 0 ;
		}
	}
	valid = true ;
	return (BillonTpl<T>*) _prop[ uid ]._adr ;
}

template <typename T, typename U> bool AntHillManager::draw( const QString &resname, arma::Mat<U> &image, uint8_t axis, uint16_t coordinate, const Interval<T> &range,bool normalize ) {
	QStringList info ;
	if ( !_prop.contains( resname ) ) {
		if ( !load_ressource( resname ) ) return false ;
	}
	
	TImageProperty storage = _prop.find( resname ).value() ;
	arma::span  cols = arma::span( storage._offset[ (axis+1)%3 ],storage._offset[ (axis+1)%3 ]+storage._size[ (axis+1)%3 ]-1 ),
				rows = arma::span( storage._offset[ (axis+2)%3 ],storage._offset[ (axis+2)%3 ]+storage._size[ (axis+2)%3 ]-1 ) ;
	if ( true || rows.b >= image.n_rows || cols.b >= image.n_cols ) {
		std::cerr<<"[ Info ] : offset "<<storage._offset[0]<<","<<storage._offset[1]<<","<<storage._offset[2]
		                      <<" size "<<storage._size[0]<<","<<storage._size[1]<<","<<storage._size[2]<<" wrt axis "<<(int)axis<<" leading to "<<rows.a<<":"<<rows.b<<" x "<<cols.a<<":"<<cols.b
		                      <<" on a 2d image's size "<<image.n_rows<<" "<<image.n_cols<<std::endl;
		std::cerr<<"[ Info ] : coordinate = "<<coordinate<<" - "<<storage._offset[axis]<<std::endl;
	}
	arma::Mat<U> subimage = image(rows,cols);
	if ( coordinate < storage._offset[axis] ) return true ;
	if ( coordinate-storage._offset[axis] >= storage._size[axis] ) return true ;
	coordinate -= storage._offset[axis] ;
	std::cerr<<"[ Info ] : coordinate = "<<coordinate<<std::endl;
	if ( storage._type == SIGNED_TINY_INT ) {
std::cerr<<"[ debug ] : draw @line "<<__LINE__<<std::endl;
		Interval<arma::s8> cast_range( range.min(), range.max() ) ;
		draw( (BillonTpl<arma::s8>*) storage._adr, subimage, axis, coordinate, cast_range, normalize ) ;
	} else if ( storage._type == UNSIGNED_TINY_INT ) {
std::cerr<<"[ debug ] : draw @line "<<__LINE__<<std::endl;
		Interval<arma::u8> cast_range( range.min(), range.max() ) ;
		draw( (BillonTpl<arma::u8>*) storage._adr, subimage, axis, coordinate, cast_range, normalize ) ;
	} else if ( storage._type == SIGNED_SHORT_INT ) {
std::cerr<<"[ debug ] : draw @line "<<__LINE__<<std::endl;
		Interval<arma::s16> cast_range( range.min(), range.max() ) ;
		draw( (BillonTpl<arma::s16>*) storage._adr, subimage, axis, coordinate, cast_range, normalize ) ;
	} else if ( storage._type == UNSIGNED_SHORT_INT ) {
std::cerr<<"[ debug ] : draw @line "<<__LINE__<<std::endl;
		Interval<arma::u16> cast_range( range.min(), range.max() ) ;
		draw( (BillonTpl<arma::u16>*) storage._adr, subimage, axis, coordinate, cast_range, normalize ) ;
	} else if ( storage._type == SIGNED_INT ) {
std::cerr<<"[ debug ] : draw @line "<<__LINE__<<std::endl;
		Interval<arma::s32> cast_range( range.min(), range.max() ) ;
		draw( (BillonTpl<arma::s32>*) storage._adr, subimage, axis, coordinate, cast_range, normalize ) ;
	} else if ( storage._type == UNSIGNED_INT ) {
std::cerr<<"[ debug ] : draw @line "<<__LINE__<<std::endl;
		Interval<arma::u32> cast_range( range.min(), range.max() ) ;
		draw( (BillonTpl<arma::u32>*) storage._adr, subimage, axis, coordinate, cast_range, normalize ) ;
	}
	image(rows,cols) = subimage ;
	//std::cerr<<"[ Info ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
	return true ;
}

/**
 * \bug drawing a subimage on axis x is ill defined (the dimensions of the rectangle seems to be swapped)
 */
template <typename T,typename U>
void AntHillManager::draw( const BillonTpl< T > *data, arma::Mat<U> &image, uint8_t axis, uint16_t coordinate, const Interval<T> &range,bool normalize ) {
	arma::span rows, cols, slices ;
	//std::cout<<"[ Debug ] : "<<__FUNCTION__<<" on "<<(void*)data<<std::endl;
	if ( axis == SLICING_ON_Y ) {
		rows = arma::span(coordinate) ;
		cols = arma::span(0,data->n_cols-1) ;
		slices = arma::span(0,data->n_slices-1) ;
	} else if ( axis == SLICING_ON_X ) {
		rows = arma::span(0,data->n_rows-1) ;
		cols = arma::span(coordinate) ;
		slices = arma::span(0,data->n_slices-1) ;
	} else {
		rows = arma::span(0,data->n_rows-1) ;
		cols = arma::span(0,data->n_cols-1) ;
		slices = arma::span(coordinate) ;
	}
std::cout<<"[ Info ] define view "<<rows.a<<":"<<rows.b<<" x "<<cols.a<<":"<<cols.b<<" x "<<slices.a<<":"<<slices.b<<" on cube "<<data->n_rows<<" x "<<data->n_cols<<" x "<<data->n_slices<<std::endl;
	const typename arma::Cube< T > v_data = (*data)( rows,cols,slices ) ;
	typename arma::Cube< T >::const_iterator readIter = v_data.begin(),
									readEnd = v_data.end() ;
	typename arma::Mat<U>::iterator writeIter = image.begin();
	//
	// QImage's data are stored row by row while cube's data are stored column by column!
	// that is why we draw on a matrice that we can next transpose for display
	//
	while ( readIter != readEnd ) {
		if ( range.containsClosed( *readIter ) ) {
			T value = *readIter ;
			if ( normalize )
				value = floor( ( ( value - range.min() ) * 255. ) / range.size() ) ;
			*writeIter = (U)value ;
		} else {
			*writeIter = range.max() < *readIter ? 255 : 0 ;
		}
		readIter++ ;
		writeIter++;
	}
	//std::cout<<"[ Info ] : finish drawing view "<<coordinate<<" on axis "<<(axis==0?"x":(axis==1?"y":"z"))<<std::endl;
}
#endif
