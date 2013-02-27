/**
 * \brief min-max segmentation of a signal
 */
#ifndef SIGNAL_MIN_MAX_SEGMENTATION_HEADER
#define SIGNAL_MIN_MAX_SEGMENTATION_HEADER

#include <QList>
#include <QPair>
#include <QSet>
#include <iostream>

template <typename IN,typename OUT> void blur_signal( const QList<IN> &input, QList<OUT> &output, uint radius ) {
	uint n ;
	for ( uint i=0;i<input.size();i++ ) {
		OUT value = 0 ;
		n = radius*2+1 ;
		for ( uint j=(i>radius?i-radius:0);j<(i<input.size()-radius?i+radius+1:input.size());j++ ) {
			value += input.at(j) ;
			n-- ;
		}
		if ( i < radius ) {
			while ( n != 0 ) {
				value += input.at(0) ;
				n-- ;
			}
		} else {
			while ( n != 0 ) {
				value += input.back() ;
				n-- ;
			}
		}
		output.append( value / ( radius * 2 + 1 ) ) ;
	}
}

template <typename T, typename W,bool minmax=true,bool tracing=false>
class SignalMinMax {
public:
	static const bool _s_absolute_min_max_search_only = minmax ;
	static const bool _s_tracing = tracing ;
	SignalMinMax( const QList< T > &input, uint radius, T ) ;
	void result( QList< W > &component ) const {
		W i_cc = 1 ;
		uint end = 1 ;
		while ( end < _result.size() ) {
			do {
				component.append( i_cc ) ;
			} while ( component.size() < _result.at( end ) ) ;
			i_cc++ ;
			end += 2 ;
		}
		i_cc-- ;
		while ( component.size() < _signal.size() )
			component.append( i_cc ) ;
	}
	uint get_max( uint begin, uint end ) ;
	uint get_firstmax( uint begin, uint end ) ;
	uint get_lastmax( uint begin, uint end ) ;
	uint get_min( uint begin, uint end ) ;
	uint get_firstmin( uint begin, uint end ) ;
	uint get_lastmin( uint begin, uint end ) ;
	T gamma( const QPair<uint,uint> &, uint & ) ;
	T delta( uint, uint,uint ) ;
	void addResult( const QPair<uint,uint> & res ) {
		_result.append( res.first ) ;
		_result.append( res.second ) ;
	}
	void addJob( const QPair<uint,uint> & job ) ;
	void remove_consecutive_extrema( QList<uint> & L ) ;
private:
	QList< T >						_signal ;
	QList< uint > 					_minimum_index ;
	QList< uint > 					_maximum_index ;
	QList< QPair< uint, uint > > 	_job ;
	QList< uint > 					_result ;
} ;
/** \example test_minmaxseg.cpp
 * This is an basic example of how to use the SignalMinMax class.
 * \verbatim 
   yourshell$ test_minmax ../samples/signal1d.dat > minmax.csv
   \endverbatim
 *
 * Opening the file minmax.csv with your favorite spreadsheet (such as http://projects.gnome.org/gnumeric/), you may obtain this kind of graphic:
 * \image html minmax.png "Cuts (in red) for the input signal (in gray) that have been blurred (in blue)" 
 */
template <typename T, typename W,bool minmax,bool tracing> T SignalMinMax<T,W,minmax,tracing>::gamma( const QPair<uint,uint> &cur, uint & peak ) {
	peak = 0 ;
	while ( peak < _maximum_index.size() ) {
		if ( _maximum_index.at( peak ) > cur.first && _maximum_index.at( peak ) < cur.second ) {
			peak = _maximum_index.at( peak ) ;
			return delta( cur.first, peak, cur.second ) ;
		} else
			peak++ ;
	}
	peak = -1 ;
	return 0 ;
}

template <typename T, typename W,bool minmax,bool tracing> uint SignalMinMax<T,W,minmax,tracing>::get_max( uint begin, uint end ) {
	uint peak = 0 ;
	if ( begin != _signal.size() && end != _signal.size() )
		while ( peak < _maximum_index.size() ) {
			if ( _maximum_index.at(peak) > begin && _maximum_index.at(peak) < end )
				return _maximum_index.at(peak) ;
			peak++ ;
		}
	return _signal.size() ;
}

template <typename T, typename W,bool minmax,bool tracing> uint SignalMinMax<T,W,minmax,tracing>::get_lastmax( uint begin, uint end ) {
	uint peak = 0 ;
	uint closest_peak = begin ;
	if ( begin != _signal.size() && end != _signal.size() ) {
		while ( peak < _maximum_index.size() ) {
			if ( _maximum_index.at(peak) > begin && _maximum_index.at(peak) < end )
				if ( end-_maximum_index.at(peak) < end-closest_peak )
					closest_peak = _maximum_index.at(peak) ;
			peak++ ;
		}
		if ( closest_peak == begin ) return _signal.size() ;
		return closest_peak ;
	}
	return _signal.size() ;
}

template <typename T, typename W,bool minmax,bool tracing> uint SignalMinMax<T,W,minmax,tracing>::get_firstmax( uint begin, uint end ) {
	uint peak = 0 ;
	uint closest_peak = end ;
	if ( begin != _signal.size() && end != _signal.size() ) {
		while ( peak < _maximum_index.size() ) {
			if ( _maximum_index.at(peak) > begin && _maximum_index.at(peak) < end )
				if ( _maximum_index.at(peak)-begin < closest_peak-begin )
					closest_peak = _maximum_index.at(peak) ;
			peak++ ;
		}
		if ( closest_peak == end ) return _signal.size() ;
		return closest_peak ;
	}
	return _signal.size() ;
}

template <typename T, typename W,bool minmax,bool tracing> uint SignalMinMax<T,W,minmax,tracing>::get_lastmin( uint begin, uint end ) {
	uint peak = 0 ;
	uint closest_peak = begin ;
	if ( begin != _signal.size() && end != _signal.size() ) {
		while ( peak < _minimum_index.size() ) {
			if ( _minimum_index.at(peak) > begin && _minimum_index.at(peak) < end )
				if ( end-_minimum_index.at(peak) < end-closest_peak )
					closest_peak = _minimum_index.at(peak) ;
			peak++ ;
		}
		if ( closest_peak == begin ) return _signal.size() ;
		return closest_peak ;
	}
	return _signal.size() ;
}

template <typename T, typename W,bool minmax,bool tracing> uint SignalMinMax<T,W,minmax,tracing>::get_firstmin( uint begin, uint end ) {
	uint peak = 0 ;
	uint closest_peak = end ;
	if ( begin != _signal.size() && end != _signal.size() ) {
		while ( peak < _minimum_index.size() ) {
			if ( _minimum_index.at(peak) > begin && _minimum_index.at(peak) < end )
				if ( _minimum_index.at(peak)-begin < closest_peak-begin )
					closest_peak = _minimum_index.at(peak) ;
			peak++ ;
		}
		if ( closest_peak == end ) return _signal.size() ;
		return closest_peak ;
	}
	return _signal.size() ;
}


template <typename T, typename W,bool minmax,bool tracing> uint SignalMinMax<T,W,minmax,tracing>::get_min( uint begin, uint end ) {
	uint peak = 0 ;
	if ( begin != _signal.size() && end != _signal.size() )
		while ( peak < _minimum_index.size() ) {
			if ( _minimum_index.at(peak) > begin && _minimum_index.at(peak) < end )
				return _minimum_index.at(peak) ;
			peak++ ;
		}
	return _signal.size() ;
}

template <typename T, typename W,bool minmax,bool tracing> T SignalMinMax<T,W,minmax,tracing>::delta ( uint mL, uint M, uint mR ) {
	if ( M != _signal.size() && mL != _signal.size() && mR != _signal.size() )
		return _signal.at(M) - std::min( _signal.at(mL), _signal.at(mR) ) ;
	return 0 ;
}

template <typename T, typename W,bool minmax,bool tracing> void SignalMinMax<T,W,minmax,tracing>::addJob( const QPair<uint,uint> & job ) {
	/**
	 * from the current implementation viewpoint, is is not necessary to sort this list
	 */
	uint 	position = 0 ;
	uint 	cur_peak,
			pos_peak ;
	T cur_gamma = gamma( job, cur_peak ) ;
	T pos_gamma ;
	if ( _s_tracing ) std::cout<<"Info : add job ["<<job.first<<" : "<<cur_peak<<" : "<<job.second<<"] gamma="<< cur_gamma<<std::endl;
	while ( position < _job.size() ) {
		pos_gamma = gamma( _job.at( position ), pos_peak ) ;
		if ( pos_gamma < cur_gamma ) break ;
		position++ ;
	}
	_job.insert( position, job ) ;
}

template <typename T, typename W,bool minmax,bool tracing> void SignalMinMax<T,W,minmax,tracing>::remove_consecutive_extrema( QList<uint> & L ) {
	QList<uint>::iterator cur=L.begin(), next ;
	uint shift;
	while ( cur != L.end() ) {
		next = cur ;
		next++ ;
		if ( next == L.end() ) break ;
		shift = 1 ;
		if ( *next != *cur+shift ) {
			cur++ ;
		} else {
			while ( *next == *cur+shift ) {
				next++ ;
				shift++ ;
				if ( next == L.end() ) break ;
			}
			shift-- ;
			*cur = *cur + shift/2 ;
			cur++ ;
			for ( ; shift != 0 ; shift-- )
				cur = L.erase( cur ) ;
		}
	}
}

template <typename T, typename W,bool minmax,bool tracing> SignalMinMax<T,W,minmax,tracing>::SignalMinMax( const QList< T > &signal, uint radius, T th_gamma ) {
	uint i,j ;
	for ( i = 0 ; i < signal.size() ; i++ ) {
		bool is_min = true ;
		bool is_max = true ;
		for ( j = (i>radius?i-radius:0) ; ( is_min || is_max ) && j < ( i < signal.size()-radius ? i+radius+1 : signal.size() ) ; j++ ) {
			if ( is_min ) is_min = ( signal.at(i) <= signal.at(j) ) ;
			if ( is_max ) is_max = ( signal.at(i) >= signal.at(j) ) ;
		}
		if ( is_min ) {
			for ( j = 0 ; j < _minimum_index.size() ; j++ )
				if ( signal.at( _minimum_index.at(j) ) > signal.at( i ) ) break ;
			_minimum_index.insert( j, i ) ;
		}
		if ( is_max ) {
			for ( j = 0 ; j < _maximum_index.size() ; j++ )
				if ( signal.at( _maximum_index.at(j) ) < signal.at( i ) ) break ;
			_maximum_index.insert( j, i ) ;

		}
		_signal.append( signal.at( i ) ) ;
	}
	
	if ( _s_tracing ) {
		std::cout<<"== Minimum =="<<std::endl;
		for ( int i = 0 ; i < _minimum_index.size() ; i++ )
			std::cout<<_minimum_index.at(i)<<" ["<<signal.at(_minimum_index.at(i))<<"] " ;
		std::cout<<std::endl;

		std::cout<<"== Maximum =="<<std::endl;
		for ( int i = 0 ; i < _maximum_index.size() ; i++ )
			std::cout<<_maximum_index.at(i)<<" ["<<signal.at(_maximum_index.at(i))<<"] ";
		std::cout<<std::endl;
	}
	/// remove consecutive extrema...
	{
		remove_consecutive_extrema( _minimum_index ) ;
		remove_consecutive_extrema( _maximum_index ) ;
	}
	if ( _s_tracing ) {
		std::cout<<"== Minimum =="<<std::endl;
		for ( int i = 0 ; i < _minimum_index.size() ; i++ )
			std::cout<<_minimum_index.at(i)<<" ["<<signal.at(_minimum_index.at(i))<<"] " ;
		std::cout<<std::endl;

		std::cout<<"== Maximum =="<<std::endl;
		for ( int i = 0 ; i < _maximum_index.size() ; i++ )
			std::cout<<_maximum_index.at(i)<<" ["<<signal.at(_maximum_index.at(i))<<"] ";
		std::cout<<std::endl;
	}
	uint firstmin=_signal.size(), lastmin=0;
	if ( !_minimum_index.isEmpty() ) {
		/// interval has to be defined such as we can find before the next(firstmin) index a max value big enought wrt the threshold
		/// and so on between lastmin and previous(lastmin)
		
		firstmin = get_firstmin( 0, _signal.size()-1 ) ;
		while ( true ) {
			uint firstmax = get_firstmax( firstmin, _signal.size()-1 ) ;
			if ( delta( firstmin,firstmax,firstmin ) > th_gamma ) break ;
			if ( firstmax == _signal.size() ) {
				firstmin = _signal.size() ;
				break ;
			}
			firstmin = get_firstmin( firstmin+1, _signal.size()-1 ) ;
		}
		if ( firstmin != _signal.size() ) {
			lastmin = get_lastmin( firstmin+1, _signal.size()-1 ) ;
			while ( true ) {
				uint lastmax = get_lastmax( firstmin, lastmin ) ;
				if ( delta( lastmin,lastmax,lastmin ) > th_gamma ) break ;
				if ( lastmax == _signal.size() ) {
					lastmin = _signal.size() ;
					break ;
				}
				lastmin = get_lastmin( firstmin+1, lastmin-1 ) ;
			}
			if ( lastmin != _signal.size() ) {
				_job.append( QPair<uint,uint>( firstmin, lastmin ) ) ;
			} else {
				addResult( QPair<uint,uint>( 0, _signal.size() ) ) ;
			}
		}
	}
	
	uint 	maxi_left, mini_left,
			maxi_right, mini_right,
			maxi_mid_left, mini_mid_left,
			maxi_mid_right, mini_mid_right ;
	T		delta_left, delta_right,
			delta_mid_left = 0,
			delta_mid_right = 0 ;
	while ( ! _job.isEmpty() ) {
		QPair<uint,uint> cur_job = _job.takeAt(0) ;
		uint peak_idx ;
		T cur_gamma = gamma( cur_job, peak_idx ) ;
		if ( cur_gamma < th_gamma ) {
			if ( _s_tracing ) std::cout<<"Info : insert (1) "<<cur_job.first<<"/"<<cur_job.second<<"( peak "<<(peak_idx<_signal.size()?_signal.at(peak_idx):-1)<<" | gamma = "<<cur_gamma<<" )"<<std::endl;
			addResult( cur_job ) ;
		} else {
			if ( _s_tracing ) std::cout<<"Info : work on job ["<<cur_job.first<<" : "<<peak_idx<<" : "<<cur_job.second<<"] gamma="<< cur_gamma<<std::endl;
			
			if ( _s_absolute_min_max_search_only ) {
				maxi_left = get_max( cur_job.first, peak_idx ) ;
				mini_left = get_min( maxi_left, peak_idx ) ;
				delta_left = std::max( delta( cur_job.first, maxi_left, mini_left ) , delta( mini_left,peak_idx,cur_job.second ) ) ;
				if ( _s_tracing ) std::cout<<"       think about ["<<cur_job.first<<","<<maxi_left<<","<<mini_left<<"] ("<<delta( cur_job.first, maxi_left, mini_left )<<") union "
											<<"["<<mini_left<<","<<peak_idx<<","<<cur_job.second<<"] ("<<delta( mini_left,peak_idx,cur_job.second )<<")"<<std::endl;
				
				mini_right = get_min( peak_idx, cur_job.second ) ;
				maxi_right = get_max( mini_right, cur_job.second ) ;
				delta_right = std::max( delta( cur_job.first, peak_idx, mini_right ), delta(mini_right,maxi_right,cur_job.second) ) ;
				if ( _s_tracing ) std::cout<<"       think about ["<<cur_job.first<<","<<peak_idx<<","<<mini_right<<"] ("<<delta( cur_job.first, peak_idx, mini_right )<<") union "
											<<"["<<mini_right<<","<<maxi_right<<","<<cur_job.second<<"] ("<<delta( mini_right,maxi_right,cur_job.second )<<")"<<std::endl;
			} else {
				maxi_left = get_firstmax( cur_job.first, peak_idx ) ;
				mini_left = get_min( maxi_left, peak_idx ) ;
				delta_left = std::max( delta( cur_job.first, maxi_left, mini_left ) , delta( mini_left,peak_idx,cur_job.second ) ) ;
				if ( _s_tracing ) std::cout<<"       think about ["<<cur_job.first<<","<<maxi_left<<","<<mini_left<<"] ("<<delta( cur_job.first, maxi_left, mini_left )<<") union "
											<<"["<<mini_left<<","<<peak_idx<<","<<cur_job.second<<"] ("<<delta( mini_left,peak_idx,cur_job.second )<<")"<<std::endl;
				
				maxi_right = get_lastmax( peak_idx, cur_job.second ) ;
				mini_right = get_min( peak_idx, maxi_right ) ;
				delta_right = std::max( delta( cur_job.first, peak_idx, mini_right ), delta(mini_right,maxi_right,cur_job.second) ) ;
				if ( _s_tracing ) std::cout<<"       think about ["<<cur_job.first<<","<<peak_idx<<","<<mini_right<<"] ("<<delta( cur_job.first, peak_idx, mini_right )<<") union "
											<<"["<<mini_right<<","<<maxi_right<<","<<cur_job.second<<"] ("<<delta( mini_right,maxi_right,cur_job.second )<<")"<<std::endl;
				
				mini_mid_left = get_lastmin( cur_job.first, peak_idx ) ;
				maxi_mid_left = get_max( cur_job.first, mini_mid_left ) ;
				delta_mid_left = std::max( delta(cur_job.first, maxi_mid_left,mini_mid_left ), delta(mini_mid_left,peak_idx,cur_job.second) ) ;
				if ( _s_tracing ) std::cout<<"       think about ["<<cur_job.first<<","<<maxi_mid_left<<","<<mini_mid_left<<"] ("<<delta( cur_job.first, maxi_mid_left, mini_mid_left )<<") union "
											<<"["<<mini_mid_left<<","<<peak_idx<<","<<cur_job.second<<"] ("<<delta( mini_mid_left,peak_idx,cur_job.second )<<")"<<std::endl;
				
				mini_mid_right = get_firstmin( peak_idx,cur_job.second ) ;
				maxi_mid_right = get_max( mini_mid_right,cur_job.second ) ;
				delta_mid_right = std::max( delta(cur_job.first, peak_idx,mini_mid_right), delta(mini_mid_right,maxi_mid_right,cur_job.second) ) ;
				if ( _s_tracing ) std::cout<<"       think about ["<<cur_job.first<<","<<peak_idx<<","<<mini_mid_right<<"] ("<<delta( cur_job.first, peak_idx, mini_mid_right )<<") union "
											<<"["<<mini_mid_right<<","<<maxi_mid_right<<","<<cur_job.second<<"] ("<<delta( mini_mid_right,maxi_mid_right,cur_job.second )<<")"<<std::endl;
			}
			if ( delta_left >= std::max( delta_right, std::max( delta_mid_left, delta_mid_right ) ) ) {
				if ( delta_left > th_gamma ) {
					addJob( QPair<uint,uint>( cur_job.first, mini_left ) ) ;
					addJob( QPair<uint,uint>( mini_left, cur_job.second ) ) ;
				} else {
					addResult( cur_job ) ;
					if ( _s_tracing ) std::cout<<"Info : insert (2) "<<cur_job.first<<"/"<<cur_job.second<<std::endl;
				}
			} else if ( delta_right >= std::max( delta_left, std::max( delta_mid_left, delta_mid_right ) ) ) {
				if ( delta_right > th_gamma ) {
					addJob( QPair<uint,uint>( cur_job.first, mini_right ) ) ;
					addJob( QPair<uint,uint>( mini_right, cur_job.second ) ) ;
				} else {
					addResult( cur_job ) ;
					if ( _s_tracing ) std::cout<<"Info : insert (2) "<<cur_job.first<<"/"<<cur_job.second<<std::endl;
				}
			} else if ( delta_mid_left >= std::max( delta_left, std::max( delta_right, delta_mid_right ) ) ) {
				if ( delta_mid_left > th_gamma ) {
					addJob( QPair<uint,uint>( cur_job.first, mini_mid_left ) ) ;
					addJob( QPair<uint,uint>( mini_mid_left, cur_job.second ) ) ;
				} else {
					addResult( cur_job ) ;
					if ( _s_tracing ) std::cout<<"Info : insert (2) "<<cur_job.first<<"/"<<cur_job.second<<std::endl;
				}
			} else {
				if ( delta_mid_right > th_gamma ) {
					addJob( QPair<uint,uint>( cur_job.first, mini_mid_right ) ) ;
					addJob( QPair<uint,uint>( mini_mid_right, cur_job.second ) ) ;
				} else {
					addResult( cur_job ) ;
					if ( _s_tracing ) std::cout<<"Info : insert (2) "<<cur_job.first<<"/"<<cur_job.second<<std::endl;
				}
			}
		}
	}
	if ( _s_tracing ) {
		std::cout<<"Cuts : " ;
		for ( int k=0;k<_result.size() ;k++) {
			std::cout<<_result.at(k)<<" " ;
		}
		std::cout<<std::endl;
	}
	qSort( _result.begin(), _result.end(), qLess<uint>() ) ;
	if ( _s_tracing ) {
		std::cout<<"Cuts (order): " ;
		for ( int k=0;k<_result.size() ;k++) {
			std::cout<<_result.at(k)<<" " ;
		}
		std::cout<<std::endl;
	}
}

#endif
