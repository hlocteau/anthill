#include "geom2d.h"

#include <iomanip>
using std::setw ;

Geom2D::Geom2D()
{
}

bool Geom2D::comp_points_location( const Z2i::Point &a, const Z2i::Point &b, const Z2i::Point &ref )
{
    int value = vectorial_product( ref, a, b ) ;
    if ( value > 0 ) return true ;
    if ( value < 0 ) return false ;
    return ( a - ref ).norm1() > ( b - ref ).norm1() ;
}

void Geom2D::sub_sort_per_angle( Z2i::Point * tab, uint id, uint max_position, const Z2i::Point ref )
{
    uint k = id ;
    uint j = 2 * k ;
    while ( j <= max_position )
    {
        if ( j < max_position && comp_points_location( tab[j], tab[j+1], ref ) )
            j++ ;
        if ( comp_points_location( tab[k], tab[j], ref ) )
        {
            Z2i::Point buf = tab[k] ;
            tab[k] = tab[j] ;
            tab[j] = buf ;
            k = j ;
            j = 2 * k ;
        }
        else
            break ;
    }
}

void Geom2D::sort_per_angle( Z2i::Point * tab, int size, const Z2i::Point ref )
{
    for ( uint i = size/2 ; i > 0 ; i-- )
        sub_sort_per_angle( tab, i, size-1, ref );
    sub_sort_per_angle( tab, 0, size-1, ref );
    for ( uint i = size-1 ; i >= 1 ; i-- )
    {
        Z2i::Point buf = tab[i] ;
        tab[i] = tab[0] ;
        tab[0] = buf ;
        sub_sort_per_angle( tab, 0, i-1, ref );
    }
}

int Geom2D::vectorial_product( const Z2i::Point &a, const Z2i::Point &b, const Z2i::Point &c )
{
    return (c.at(0)-a.at(0) )*( b.at(1) - a.at(1) ) - ( b.at(0) - a.at(0) )*(c.at(1)-a.at(1) );
}

void Geom2D::ConvexHull( Z2i::DigitalSet &input, std::list<Z2i::Point> &output )
{
    assert( !input.empty() ) ;

    Z2i::Point pivot = * input.begin();
    Z2i::Point *tab = new Z2i::Point[ input.size() ] ;
    uint position = 0 ;
    /** select pivot */
    for ( Z2i::DigitalSet::ConstIterator pt = input.begin() ; pt != input.end() ; pt++ )
    {
        if ( pt->at(0) < pivot.at(0) )
            pivot = *pt ;
        else if ( pt->at(0) == pivot.at(0) && pt->at(1) < pivot.at(1) )
            pivot = *pt ;
        tab[ position ] = *pt ;
        position++ ;
    }
	if ( input.size() > 1 ) {
		/** sort table of points according to x-axis and pivot */
		sort_per_angle( tab, input.size(), pivot ) ;
    
		vector< Z2i::Point > queue_ ;
		queue_.reserve( input.size());
		queue_.push_back( tab[input.size()-1] );
		queue_.push_back( tab[input.size()-2] );
		/** selection of remaining points */
		for ( uint i = 3 ; i <= input.size() ; i++ )
		{
			while ( queue_.size() >= 2 )
				if ( vectorial_product( queue_[queue_.size()-2], queue_.back(), tab[input.size()-i]) >= 0 )
					queue_.pop_back();
				else
					break ;
			queue_.push_back( tab[input.size()-i]) ;
		}
		output.clear();
		for ( uint i = 0 ; i < queue_.size() ; i++ )
			output.push_back( queue_[i] ) ;
	} else {
		output.push_back( pivot ) ;
	}
}

Z2i::Point * Geom2D::computeD8Segment( const Z2i::Point &p, const Z2i::Point &q, unsigned int &size ) {
	int a = q.at(1)-p.at(1) ;
	int b = q.at(0)-p.at(0) ;
	
	size = max( abs( a ), abs( b ) )+1 ;
	int i ;
	Z2i::Point *points = new Z2i::Point [ size ] ;

	if ( a * b == 0 ) {
		if ( a == 0 ) {
			int inc = ( b > 0 ? 1 : -1 ) ;
			for ( i = 0 ; i < size ; i++ )
				points[i] = Z2i::Point( p.at(0) + i*inc, p.at(1) ) ;
		} else {
			int inc = ( a > 0 ? 1 : -1 ) ;
			for ( i = 0 ; i < size ; i++ )
				points[i] = Z2i::Point( p.at(0), p.at(1)+inc*i ) ;		
		}
	} else if ( abs(a) == abs(b) ) {
		Z2i::Point inc = Z2i::Point( ( b > 0 ? 1 : -1 ),( a > 0 ? 1 : -1 ) ) ;
		points[ 0 ] = p ;
		for ( i = 1 ; i < size ; i++ )
			points[ i ] = points[ i-1 ] + inc ;
	} else if ( abs(a) > abs(b) ) {
		for ( i = 0 ; i < size ; i++ )
			points[ i ] = p + Z2i::Point( (int)floor(  ( b * i ) / (double)abs(a)+0.5), i*( a>0?1:-1) ) ;
	} else {
		for ( i = 0 ; i < size ; i++ )
			points[ i ] = p + Z2i::Point(i*( b>0?1:-1), (int)floor( ( a * i ) / (double)abs(b)+0.5 ) ) ;
	}
	if ( points[size-1] != q ) {
		std::cerr<<"Generation of segment pixels "<<p<<" to "<<q<<"\t"<<" | a,b is "<<a<<","<<b<<std::endl;
		for ( i = 0;i<size;i++ ) std::cerr<<"("<<points[i].at(0)<<","<<points[i].at(1)<<") " ;
		std::cerr<<std::endl;
	}
	assert( points[size-1] == q ) ;
	return points ;
}

void Geom2D::init_edges_polygon( EdgePolygon &edges, const VertexPolygon &vertices ) {
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


void Geom2D::pca( const Z3i::DigitalSet & cloud, arma::Mat< double >  &base, arma::colvec  &centroid ) {
	/// compute gravity center
	centroid = arma::zeros< arma::colvec >(3) ;
	for ( Z3i::DigitalSet::ConstIterator pt = cloud.begin() ; pt != cloud.end() ; pt++ ) {
		centroid(0) += (*pt).at(0) ;
		centroid(1) += (*pt).at(1) ;
		centroid(2) += (*pt).at(2) ;
	}
	centroid /= cloud.size() ;

	/// compute covariance matrice
	arma::Mat< double > A = arma::zeros< arma::Mat<double > >(3,3) ;
	register unsigned int i,j ;
	for ( Z3i::DigitalSet::ConstIterator pt = cloud.begin() ; pt != cloud.end() ; pt++ ) {
		arma::colvec p(3);
		for ( i=0;i<3;i++) p(i) = (*pt).at(i) - centroid(i) ;
		
		for ( i = 0 ; i < 3 ; i++ )
			for ( j = 0 ; j < 3 ; j++ )
				A(i,j) += p(i)  * p(j) ;
	}
	A /= (double) cloud.size() ;
	arma::colvec eigval= arma::zeros< arma::colvec >(3) ;
	base = arma::zeros< arma::Mat<double > >(3,3) ;
	/// get eigen vectors
	eig_sym( eigval, base, A ) ;
	std::cout<<"eigen values : "<<eigval(0)<<" "<<eigval(1)<<" "<<eigval(2)<<std::endl;
	base.swap_cols(0,2) ;
	std::cout<<base<<std::endl;
}

void Geom2D::paramEnclosingDomain( const Z3i::DigitalSet & cloud, arma::colvec  &X, arma::colvec  &centroid, double &thickness, double &radius ) {
	/// we minimize hte thickness on the z axis
	arma::Mat< double > A = arma::zeros< arma::Mat<double > >(3,3) ;
	arma::colvec b = arma::zeros< arma::colvec >(3) ;
	centroid = arma::zeros< arma::colvec >(3) ;
		
	for ( Z3i::DigitalSet::ConstIterator pt = cloud.begin() ; pt != cloud.end() ; pt++ ) {
		centroid(0) += (*pt).at(0) ;
		centroid(1) += (*pt).at(1) ;
		centroid(2) += (*pt).at(2) ;
	}
	centroid /= cloud.size() ;
	
	for ( Z3i::DigitalSet::ConstIterator pt = cloud.begin() ; pt != cloud.end() ; pt++ ) {
		double x = (*pt).at(0);//-centroid(0) ;
		double y = (*pt).at(1);//-centroid(1) ;
		double z = (*pt).at(2);//-centroid(2) ;
		A(0,0) +=  x * x  ;		A(0,1) +=  x * y  ;		A(0,2) +=  x  ;
		A(1,0) +=  x * y  ;		A(1,1) +=  y * y  ;		A(1,2) +=  y  ;
		A(2,0) +=  x  ;			A(2,1) +=  y  ;
		
		b(0) += x * z ;
		b(1) += y * z ;
		b(2) += z ;
		
	}
	A(2,2) = cloud.size() ;
	
	X = arma::zeros< arma::colvec >(3) ;
	
	
	std::cout	<<"Try to solve system A X = b having b = "<<b(0)<<" "<<b(1)<<" "<<b(2)<<" and A being "
				<<setw(8)<<A(0,0)<<" "<<setw(8)<<A(0,1)<<" "<<setw(8)<<A(0,2)<<std::endl
				<<setw(8)<<A(1,0)<<" "<<setw(8)<<A(1,1)<<" "<<setw(8)<<A(1,2)<<std::endl
				<<setw(8)<<A(2,0)<<" "<<setw(8)<<A(2,1)<<" "<<setw(8)<<A(2,2)<<std::endl;
	
	arma::solve( X, A, b ) ;
	std::cout<<" X = "<<X(0)<<" "<<X(1)<<" "<<X(2)<<std::endl;
	
	radius = -1 ;
	thickness = -1 ;
	for ( Z3i::DigitalSet::ConstIterator pt = cloud.begin() ; pt != cloud.begin() ; pt++ ) {
		arma::colvec P(3);
		P(0) = (*pt).at(0) ; P(1) = (*pt).at(1) ; P(2) = (*pt).at(2) ;
		
		/// projection if x,y,z on the plan
		double k = ( X(0) * ( P(0) - centroid(0) ) + X(1) * ( P(1) - centroid(1) ) + X(2) * ( P(2) - centroid(2) ) ) / ( X(0)*X(0)+X(1)*X(1)+X(2)*X(2) ) ;
		
		arma::colvec projection(3);
		projection(0) = k * X(0) + P(0);
		projection(1) = k * X(1) + P(1);
		projection(2) = k * X(2) + P(2);
		 
		/// check output (formula) with gnumeric
		//std::cout<<P(0)<<";"<<P(1)<<";"<<P(2)<<";";
		//std::cout<<projection(0)<<";"<<projection(1)<<";"<<projection(2)<<std::endl;
		
		/// max distance of a projection to the centroid
		arma::colvec dep = projection - centroid ;
		radius = max( radius, arma::norm( dep, 2 ) ) ;
		
		/// max distance of a projection to the point
		dep = projection - P ;
		thickness = max ( thickness, arma::norm( dep, 2 ) ) ;
	}
	thickness = sqrt( thickness ) ;
	radius = sqrt( radius ) ;
}

void Geom2D::projection_on( const Z3i::Point &pt, const arma::Mat< double > &base, const arma::colvec &centroid, arma::colvec &h ) {
	arma::colvec p(3);
	p(0) = pt.at(0) - centroid(0) ;		p(1) = pt.at(1) - centroid(1) ;		p(2) = pt.at(2) - centroid(2) ;
	h = base * p ;
	h(0) += centroid(0) ;				h(1) += centroid(1) ;				h(2) += centroid(2) ;

}

void Geom2D::projection_on( const Z3i::Point &pt, const arma::colvec & eq, const arma::colvec &centroid, arma::colvec &h ) {
	arma::colvec P(3);
	P(0) = (pt).at(0);//-centroid(0) ;
	P(1) = (pt).at(1);//-centroid(1) ;
	P(2) = (pt).at(2);//-centroid(2);
	
	/// projection if x,y,z on the plan
	//double_t k = ( eq(0) * ( P(0) - centroid(0) ) + eq(1) * ( P(1) - centroid(1) ) + eq(2) * ( P(2) - centroid(2) ) ) / ( eq(0)*eq(0)+eq(1)*eq(1)+eq(2)*eq(2) ) ;
	
	//h(0) = k * eq(0) + P(0);
	//h(1) = k * eq(1) + P(1);
	//h(2) = k * eq(2) + P(2);
	
	
	double d = - ( eq(0) * centroid(0) + eq(1) * centroid(1) + eq(2) * centroid(2) ) ;
	double k = -( eq(0)*P(0)+eq(1)*P(1)+eq(2)*P(2) + d ) / ( eq(0)*eq(0) + eq(1)*eq(1) + eq(2)*eq(2) ) ;
	
	h(0) = eq(0) * k + P(0);		h(1) = eq(1) * k+ P(1) ;		h(2) = eq(2) * k+P(2) ;
	
	//h(0) += centroid(0) ;			h(1) += centroid(1) ;			h(2) += centroid(2) ;
}
