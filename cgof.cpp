#include <getopt.h>
#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <ncurses.h>
#include <unistd.h>
#include <cstring>
#include <sstream>

using namespace std;

//TODO: safe checking for the colors and speed

#define RED "\033[1;31m"
#define WHT "\033[1;37m"
#define GRN "\033[32m"
#define BLU "\033[34m"
#define RST "\033[0m"
 
// PRINT FUNCTION
void Printgrid( const char* blk, int WIDTH, int HEIGHT, const vector< vector<bool> >& grid ){

	for ( int i = 0; i < WIDTH;  i++ ){
		for ( int j = 0; j < HEIGHT;  j++ ){

			const char* print =  ( grid[i][j] == 1 ) ? blk : " " ;
			move( j,i );
			printw( "%s", print );

		}
	}
	refresh();
};

// CHECK NEIGHBORS FUNCTION
int check_neighbors( int i, int j, int WIDTH, int HEIGHT, const vector< vector<bool> >& grid ){
	// to check the neighbors I identify the opposits hedges of the grid (toroidal warp)
	int count{};
	int ia{};
	int jb{};

	for( int a = -1;  a < 2 ; a++ ){
		for( int b = -1;  b < 2; b++ ){
			ia = i+a;
			jb = j+b;

			// toroidal identification block
			if ( jb == HEIGHT ){ jb=0; }
			if ( jb == -1 ){ jb=HEIGHT-1; }
			if ( ia == WIDTH ){ ia=0; }
			if ( ia == -1 ){ ia=WIDTH-1; }

			// counting
			if ( grid[ia][jb] == 1 and ( a!=0 or b!=0 ) ){count+=1;}
		}
	}
	return count;
};

// APPLY RULES FUNCTION
int apply_rules( int i, int j, int WIDTH, int HEIGHT, const vector< vector<bool> >& grid ){
	// to check the neighbors I identify the opposites hedges of the grid (toroidal wrap)

	bool alive = grid[i][j];
	int count = check_neighbors(i,j, WIDTH, HEIGHT, grid);

	if (count < 2 and alive == 1){
		alive = 0;
	}else if( ( count == 2 or count == 3 ) and alive == 1){
		alive = 1;
	}else if(count > 3 and alive == 1){
		alive = 0;
	}else if(count == 3 and alive == 0){
		alive = 1;
	}else{alive = 0;}

	return alive;
};

// MODIFY GRID: THIS FUNCTION MODIFIES THE WHOLE GRID THEN WE PRINT
vector< vector<bool> > PrintgridUpdate( int WIDTH, int HEIGHT, const vector< vector<bool> >& grid ){

	vector< vector<bool> > appo( WIDTH, vector<bool>( HEIGHT, false ) );
	for ( int i = 0; i < WIDTH;  i++ ){
		for ( int j = 0; j < HEIGHT;  j++ ){
			appo[i][j] = apply_rules(i,j, WIDTH, HEIGHT, grid);
		}
	}
	return appo;
};

// VARIOUS INITIALIZERS
void GliderInit( int WIDTH, int HEIGHT, vector< vector<bool> >& grid ){
	// initialize the cells

	for ( int i = 0; i < WIDTH;  i++ ){
		for ( int j = 0; j < HEIGHT;  j++ ){

			// glider init
			if( (i == 1 and j == 2) or (i == 0 and j == 1) or (i == 2 and j == 0) or (i == 2 and j == 1) or (i == 2 and j == 2) ){
				grid[i][j] = 1;
			}else{ grid[i][j] = 0; }

		}
	}
};

void BlinkerInit( int WIDTH, int HEIGHT, vector< vector<bool> >& grid ){
	// initialize the cells

	for ( int i = 0; i < WIDTH;  i++ ){
		for ( int j = 0; j < HEIGHT;  j++ ){

			// glider init
			if( (i == 5 and j == 5) or (i == 6 and j == 5) or (i == 7 and j == 5) ){
				grid[i][j] = 1;
			}else{ grid[i][j] = 0; }

		}
	}
};

void RandomInit( int WIDTH, int HEIGHT, vector< vector<bool> >& grid ){
	// initialize the cells
	default_random_engine generator;
	discrete_distribution<int> distribution {1,1}; //weighted function that assigns 0,1 with probability proportional to the weight 
	
	for ( int i = 0; i < WIDTH;  i++ ){
		for ( int j = 0; j < HEIGHT;  j++ ){
			grid[i][j] = distribution(generator);
		}
	}
};

// help function for the case -h
void helpfun(const char* prog){
 
	cout << WHT "usage: " RST << prog << " [" GRN "options" RST "] [" BLU "glyph" RST "]\n\n"
		"Conway's Game of Life on a toroidal grid.\n"
		BLU "glyph" RST " is the string drawn for a live cell (default: " BLU "█" RST "). Any UTF-8 string works, e.g. 🐉 or a Nerd Font icon.\n\n"
 
		WHT "options:\n" RST
		"  " GRN "--version" RST  "     check version\n"
		"  " GRN "-c" BLU " COLOR" RST "      foreground color of live cells (default: " BLU "red" RST ")\n"
		"  " GRN "-b" BLU " COLOR" RST "      background color (default: " BLU "black" RST ")\n"
		"  " GRN "-v" BLU " MS" RST "         milliseconds between generations, integer > 0 (default: " BLU "100" RST ")\n"
		"  " GRN "-s" BLU " N" RST "          restart every N generations, 0 = never (default: " BLU "0" RST ")\n"
		"  " GRN "-t" BLU " HH:MM:SS" RST "   restart after this much run time (default: never)\n"
		"  " GRN "-m" BLU " MODE" RST "       initial pattern: " BLU "random" RST ", " BLU "glider" RST ", " BLU "blinker" RST " (default: " BLU "random" RST ")\n"
		"  " GRN "-r" RST "            rainbow mode: change the foreground color at every generation\n"
		"  " GRN "-h" RST "            show this help and exit\n\n"
		WHT "colors: " RST "\033[30;47mblack" RST " \033[31mred" RST " \033[32mgreen" RST " \033[33myellow" RST " \033[34mblue" RST " \033[35mmagenta" RST " \033[36mcyan" RST " \033[37;40mwhite" RST "\n\n"
		WHT "keys while running:\n" RST
		"  " GRN "q" RST "  quit\n"
		"  " GRN "r" RST "  restart from the initial pattern\n"
		"  " GRN "c" RST "  next foreground color\n"
		"  " GRN "b" RST "  next background color\n\n"
		"see '" GRN "man cppautomata" RST "' for details\n";
};

// usage error function for the case of wrong input
int usage_error(const char* prog, const string& msg){
 
	cerr << RED "error: " RST << msg << "\n";
	cerr << WHT "usage: " RST << prog << " [-c color] [-b color] [-v ms] [-s n] [-t hh:mm:ss] [-m mode] [-r] [-h] [glyph]\n";
	cerr << "try '" GRN << prog << " -h" RST "' for more information\n";
 
	return 1;
}

// version function
void version(){
 
	cout << RED << R"(

 ░▒▓██████▓▒░░▒▓███████▓▒░░▒▓███████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓████████▓▒░▒▓██████▓▒░░▒▓██████████████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░▒▓██████▓▒░  
░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░  ░▒▓█▓▒░  ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░  ░▒▓█▓▒░░▒▓█▓▒░ 
░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░  ░▒▓█▓▒░  ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░  ░▒▓█▓▒░░▒▓█▓▒░ 
░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░  ░▒▓█▓▒░  ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓████████▓▒░ ░▒▓█▓▒░  ░▒▓████████▓▒░ 
░▒▓█▓▒░      ░▒▓█▓▒░      ░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░  ░▒▓█▓▒░  ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░  ░▒▓█▓▒░░▒▓█▓▒░ 
░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░  ░▒▓█▓▒░  ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░  ░▒▓█▓▒░░▒▓█▓▒░ 
 ░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░   ░▒▓█▓▒░   ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░  ░▒▓█▓▒░░▒▓█▓▒░ 
)" << RST "\nversion 1.0\n";
	cout << "Copyright (C) 2026 RICCARDO MARTELLI\n";
};



int color_from_name(const char* name){

	if(!strcmp(name, "black")) return COLOR_BLACK;
	else if(!strcmp(name, "red")) return COLOR_RED;
	else if(!strcmp(name, "green")) return COLOR_GREEN;
	else if(!strcmp(name, "yellow")) return COLOR_YELLOW;
	else if(!strcmp(name, "blue")) return COLOR_BLUE;
	else if(!strcmp(name, "magenta")) return COLOR_MAGENTA;
	else if(!strcmp(name, "cyan")) return COLOR_CYAN;
	else if(!strcmp(name, "white")) return COLOR_WHITE;
	else{
		cerr << RED "error:" RST " unknown color '" << name << "'\n";
		cerr << "available colors: black, red, green, yellow, blue, magenta, cyan, white\n";
		exit(1);
	}
}
// MAIN LOOP
int main( int argc, char **argv ){

	for (int i = 0; i < argc; i++){
			if(strcmp(argv[i], "--version") == 0){ version(); return 0; }
	}

	const char *blk;
	int opt;
	
	// We can define here the values on the colors and the bkg, and the default ones
	int	DEFAULT_COLOR = 1;
	const char* def_col = nullptr;
	int DEFAULT_BKG_COLOR = 0;
	const char* def_bkg_col = nullptr;
	// velocity of the updates in ms
	int update_time = 100;	
	const char* vel_char = nullptr;

	// cycles for the screensaver to revert to the beginning
	int cycles = 0;
	const char* cycle_char = nullptr;
	//set the local mode to true
	bool color_mode = false;

	// name of the mode
	const char* mod_name = nullptr;
	// timer for the screensaver to revert to the beginning
	const char* timer = nullptr;

	setlocale(LC_ALL, "");
	
		while((opt = getopt(argc, argv, "c:s:t:m:b:v:r h")) != -1){
		
		switch(opt){
			case 'r': color_mode = true; break;
			case 's': cycle_char = optarg; cycles = atoi(cycle_char);  break;
			case 'c': def_col = optarg; DEFAULT_COLOR = color_from_name(def_col); break;
			case 'b': def_bkg_col = optarg; DEFAULT_BKG_COLOR = color_from_name(def_bkg_col); break;
			case 'h': helpfun(argv[0]); return 0;
			case 'v': vel_char = optarg; update_time = atoi(vel_char); break;
			case 'm': mod_name = optarg; break;
			case 't': timer = optarg; break;

			default:
				cerr << "usage: " << argv[0] << " [glyph] [-tscbvrh] \n -h for help \n -r for a mode that changes color at each step";

				return 1;
		}
		
	}

	if ( update_time <= 0 ){
		cerr << RED "error:" RST " update time must be a positive integer\n";
		exit(1);
	}

	// unistd return the index of the first non-option arg as optind
	if ( optind < argc ){
		blk = argv[optind];
	}else{ blk = "█"; }
	//        COLOR_BLACK   0
	//        COLOR_RED     1
	//        COLOR_GREEN   2
	//        COLOR_YELLOW  3
	//        COLOR_BLUE    4
	//        COLOR_MAGENTA 5
	//        COLOR_CYAN    6
	//        COLOR_WHITE   7

	int hh,mm,ss;
	char duep1, duep2;
	
	float tot_sec = 0;
	float new_sec = 0;

	if ( timer != nullptr ){
	
	stringstream ss_in(timer);

	ss_in >> hh >> duep1 >> mm >> duep2 >> ss; // the stream is separeg the quantities

	tot_sec = hh*3600 + mm*60 + ss;
	new_sec = tot_sec;
	
	}

	initscr();
	curs_set( 0 );
	cbreak();
	nodelay( stdscr, TRUE ); // need to switch getch from blocking to non blocking

	int HEIGHT, WIDTH;
	getmaxyx( stdscr, HEIGHT, WIDTH );
	vector< vector<bool> > grid( WIDTH, vector<bool>( HEIGHT, false ) );

	restart:
	if( mod_name != nullptr and strcmp(mod_name, "glider") == 0 ){
		GliderInit( WIDTH, HEIGHT, grid );
	}else if(mod_name != nullptr and strcmp(mod_name, "blinker") == 0){ 
		BlinkerInit( WIDTH, HEIGHT, grid );
	}else{ RandomInit( WIDTH, HEIGHT, grid ); }

	start_color(); // lets give some color

	init_pair( 1, DEFAULT_COLOR, DEFAULT_BKG_COLOR );
	attron( COLOR_PAIR( 1 ) ); //starts the attribute function
	// prints the cells to scren after the update.
	Printgrid( blk, WIDTH, HEIGHT, grid );
	this_thread::sleep_for( chrono::milliseconds( update_time ) );

	if ( timer != nullptr ){ new_sec -= update_time/1000.0 ; }

	int counter = 0;
	while( true ){ 

		int ch = getch();
	  if( ch == 'q'){ break; } // on q  exits
		if( ch == 'r'){ goto restart; } // on r reload
		if( ch == 'c'){ 
			DEFAULT_COLOR = (DEFAULT_COLOR + 1) % COLORS;
			init_pair( 1, DEFAULT_COLOR, DEFAULT_BKG_COLOR );
		}
		if( ch == 'b'){ 
			DEFAULT_BKG_COLOR = (DEFAULT_BKG_COLOR + 1) % COLORS;
			init_pair( 1, DEFAULT_COLOR, DEFAULT_BKG_COLOR );
		}
		if( color_mode ){		
			DEFAULT_COLOR = (DEFAULT_COLOR + 1) % COLORS;
			init_pair( 1, DEFAULT_COLOR, DEFAULT_BKG_COLOR );
		}

		grid = PrintgridUpdate( WIDTH, HEIGHT, grid );
		Printgrid( blk, WIDTH, HEIGHT, grid );
		this_thread::sleep_for( chrono::milliseconds( update_time ) );
		new_sec -=  update_time/1000.0; 

		if(timer != nullptr and new_sec <= 0 ){
			new_sec = tot_sec;
			goto restart;
		}
		
		if( cycles != 0 && counter == cycles ) goto restart;		
		if( cycles != 0 ) counter++;

		};

	endwin();

 return 0;

};
