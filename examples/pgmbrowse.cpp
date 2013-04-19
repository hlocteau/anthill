#include <io/IOUtils.h>
/// see http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/index.html
#include <ncurses.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#define QUIT_COMMAND	1
#define MOVE_LEFT 		2
#define MOVE_RIGHT		4
#define MOVE_CLOSE		8
#define MOVE_FAR		16
#define MOVE_UP			32
#define MOVE_DOWN		64
#define XY_PLANE		128
#define YZ_PLANE		256
#define ZX_PLANE		512

#define IS_QUIT_COMMAND( cmd )  ( cmd & QUIT_COMMAND )

#define IS_MOVE_LEFT( cmd )  	( cmd & MOVE_LEFT )
#define IS_MOVE_RIGHT( cmd ) 	( cmd & MOVE_RIGHT )
#define IS_MOVE_CLOSE( cmd ) 	( cmd & MOVE_CLOSE )
#define IS_MOVE_FAR( cmd )   	( cmd & MOVE_FAR )
#define IS_MOVE_UP( cmd )    	( cmd & MOVE_UP )
#define IS_MOVE_DOWN( cmd )  	( cmd & MOVE_DOWN )

#define IS_XY_PLANE( cmd )   	( cmd & XY_PLANE )
#define IS_YZ_PLANE( cmd )   	( cmd & YZ_PLANE )
#define IS_ZX_PLANE( cmd )   	( cmd & ZX_PLANE )

typedef arma::u32 DistType ;

namespace PgmBrowse {
	namespace po = boost::program_options;
	typedef struct _Point { int x, y, z ; } Point ;
	
	typedef struct _TUIParam {
		Point 	lower ;
		Point 	upper ;
		int		plane ;
		int     n_rows ;
		int		n_cols ;
		int		n_slices ;
		std::string filename ;
		BillonTpl< DistType > *img ;
	} TUIParam ;

	void errorAndHelp( const po::options_description & general_opt ) {
		std::cerr 	<< "Push multiple times a 2d image in order to get a 3d image."<<std::endl
					<< general_opt << "\n";
	}

	bool missingParam ( std::string param ) {
		std::cerr <<" Parameter: "<<param<<" is required.."<<std::endl;
		return false ;
	}

	bool process_arg( int narg, char **argv, TUIParam &params ) {
		po::options_description general_opt ( "Allowed options are: " );
		general_opt.add_options()
			( "help,h", "display this message." )
			( "image,i", po::value<std::string>(), "Input binary pgm filename (3d)." )
			( "xmin,x", po::value<int>()->default_value(0), "xmin.")
			( "Xmax,X", po::value<int>()->default_value(-1), "xmax.")
			( "ymin,y", po::value<int>()->default_value(0), "ymin.")
			( "Ymax,Y", po::value<int>()->default_value(-1), "ymax.")
			( "zmin,z", po::value<int>()->default_value(0), "zmin.")
			( "Zmax,Z", po::value<int>()->default_value(-1), "zmax.");

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
		if ( ! ( vm.count ( "image" ) ) ) { missingParam ( "image" ); return false ; }
		params.filename = vm["image"].as<std::string>();
		
		params.lower.x = vm["xmin"].as<int>() ;
		params.lower.y = vm["ymin"].as<int>() ;
		params.lower.z = vm["zmin"].as<int>() ;
		params.upper.x = vm["Xmax"].as<int>() ;
		params.upper.y = vm["Ymax"].as<int>() ;
		params.upper.z = vm["Zmax"].as<int>() ;
		
		return true ;
	}

	bool interaction( TUIParam &params ) {
		
		int key = getch();
		int cmd =0;
		
		// encoding
		if /* ESC */ ( key == 27 ) cmd |= QUIT_COMMAND ;
		
		if ( key == KEY_UP || key == 'H' ) cmd |= MOVE_UP ;
		if ( key == KEY_DOWN || key == 'H' ) cmd |= MOVE_DOWN ;
		if ( key == KEY_LEFT || key == 'W') cmd |= MOVE_LEFT ;
		if ( key == KEY_RIGHT || key == 'W') cmd |= MOVE_RIGHT ;
		if /* next-page key */ ( key == KEY_NPAGE || key == 'D') cmd |= MOVE_FAR ;
		if /* previous-page key */ ( key == KEY_PPAGE || key == 'D') cmd |= MOVE_CLOSE ;
		
		if ( key == 'z' ) cmd |= XY_PLANE ;
		if ( key == 'x' ) cmd |= YZ_PLANE ;
		if ( key == 'y' ) cmd |= ZX_PLANE ;
		
		
		// interpreting
		if ( IS_QUIT_COMMAND( cmd ) ) return false ;
		if ( IS_MOVE_LEFT( cmd ) ) {
			if ( params.lower.x != 0 ) params.lower.x -- ;
			if ( params.upper.x != 1 ) params.upper.x -- ;
			
		}
		if ( IS_MOVE_UP( cmd ) ) {
			if ( params.lower.y != 0 ) params.lower.y -- ;
			if ( params.upper.y != 1 ) params.upper.y -- ;
			
		}
		if ( IS_MOVE_CLOSE( cmd ) ) {
			if ( params.lower.z != 0 ) params.lower.z -- ;
			if ( params.upper.z != 1 ) params.upper.z -- ;
			
		}
		if ( IS_MOVE_RIGHT( cmd ) ) {
			if ( params.upper.x != params.n_cols-1 ) params.upper.x++ ;
			if ( params.lower.x != params.n_cols-2 ) params.lower.x++ ;
		}
		if ( IS_MOVE_DOWN( cmd ) ) {
			if ( params.upper.y != params.n_rows-1 ) params.upper.y++ ;
			if ( params.lower.y != params.n_rows-2 ) params.lower.y++ ;
		}
		if ( IS_MOVE_FAR( cmd ) ) {
			if ( params.upper.z != params.n_slices-1 ) params.upper.z++ ;
			if ( params.lower.z != params.n_slices-2 ) params.lower.z++ ;
		}
		if ( IS_XY_PLANE( cmd ) ) params.plane = XY_PLANE ;
		if ( IS_YZ_PLANE( cmd ) ) params.plane = YZ_PLANE ;
		if ( IS_ZX_PLANE( cmd ) ) params.plane = ZX_PLANE ;
		return true ;
	}
	
	void display( TUIParam &params ) {
		int bounds[4] ;
		int idx_loop[3] ;
		clear();
		clrtoeol();
		int code_loop ;
		char name_axis[]= "zxy";
		if ( params.plane == XY_PLANE ) { code_loop=0; bounds[0] = params.lower.x ; bounds[1] = params.upper.x ; bounds[2] = params.lower.y ; bounds[3] = params.upper.y ; idx_loop[2]=(params.lower.z+params.upper.z)/2; }
		if ( params.plane == YZ_PLANE ) { code_loop=1; bounds[0] = params.lower.y ; bounds[1] = params.upper.y ; bounds[2] = params.lower.z ; bounds[3] = params.upper.z ; idx_loop[0]=(params.lower.x+params.upper.x)/2; }
		if ( params.plane == ZX_PLANE ) { code_loop=2; bounds[0] = params.lower.z ; bounds[1] = params.upper.z ; bounds[2] = params.lower.x ; bounds[3] = params.upper.x ; idx_loop[1]=(params.lower.y+params.upper.y)/2; }

mvprintw(0, 0, "Window %d:%d x%d:%d at %c=%d",bounds[2],bounds[3], bounds[0],bounds[1], name_axis[code_loop], idx_loop[ (code_loop+2)%3]);
move(1,0);
		for ( idx_loop[ code_loop ] = bounds[2] ; idx_loop[ code_loop ] != bounds[3] ; idx_loop[ code_loop ]++ ) {
			for ( idx_loop[ (code_loop+1)%3 ] = bounds[0] ; idx_loop[ (code_loop+1)%3 ] != bounds[1] ; idx_loop[ (code_loop+1)%3 ]++ )
				printw( " %3d", (*params.img)( idx_loop[0], idx_loop[1], idx_loop[2] ) ) ;
			printw("\n");
		}
		refresh() ;
	}
}
using namespace PgmBrowse ;

int main( int narg, char **argv ) {
	TUIParam params ;
	if ( !process_arg( narg, argv, params ) ) return 0 ;
	
	Pgm3dFactory< DistType > factory ;
	
	params.img = factory.read( params.filename.c_str() ) ;
	if ( !params.img ) return 0 ;
	factory.correctEncoding( params.img ) ;
	
	if ( params.upper.x == -1 ) params.upper.x = params.img->n_cols ;
	if ( params.upper.y == -1 ) params.upper.y = params.img->n_rows ;
	if ( params.upper.z == -1 ) params.upper.z = params.img->n_slices ;
	
	params.upper.x = std::min( params.upper.x, (int)params.img->n_cols ) ;
	params.upper.y = std::min( params.upper.y, (int)params.img->n_rows ) ;
	params.upper.z = std::min( params.upper.z, (int)params.img->n_slices ) ;
	
	params.lower.x = std::min( std::max(params.lower.x,0), params.upper.x-1 ) ;
	params.lower.y = std::min( std::max(params.lower.y,0), params.upper.y-1 ) ;
	params.lower.z = std::min( std::max(params.lower.z,0), params.upper.z-1 ) ;

	params.plane = XY_PLANE ;
	
	// start curses mode
	initscr() ;
	// disable line buffering
	raw() ;
	
	keypad(stdscr, TRUE);
	do
		display( params ) ;
	while ( interaction(params) ) ;
	delete params.img ;
	
	// end curses mode	
	endwin() ;
	
	return 1 ;
}
