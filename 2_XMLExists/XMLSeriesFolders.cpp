//
// Check how images are distributed among the series folders
// of form 'V91005p2*'.
//
// Example command:
// >XMLExists xxx.xml -zmin=0 -zmax=10
//


#include	"Cmdline.h"
#include	"File.h"

#include	"tinyxml.h"

#include	<set>
#include	<string>
using namespace std;


/* --------------------------------------------------------------- */
/* Macros -------------------------------------------------------- */
/* --------------------------------------------------------------- */

/* --------------------------------------------------------------- */
/* Types --------------------------------------------------------- */
/* --------------------------------------------------------------- */

/* --------------------------------------------------------------- */
/* CArgs_xex ----------------------------------------------------- */
/* --------------------------------------------------------------- */

class CArgs_xex {

public:
	char	*infile;
	int		zmin, zmax;

public:
	CArgs_xex()
	{
		infile	= NULL;
		zmin	= 0;
		zmax	= 32768;
	};

	void SetCmdLine( int argc, char* argv[] );
};

/* --------------------------------------------------------------- */
/* Statics ------------------------------------------------------- */
/* --------------------------------------------------------------- */

static CArgs_xex		gArgs;
static FILE*			flog = NULL;






/* --------------------------------------------------------------- */
/* SetCmdLine ---------------------------------------------------- */
/* --------------------------------------------------------------- */

void CArgs_xex::SetCmdLine( int argc, char* argv[] )
{
// start log

	flog = FileOpenOrDie( "XMLFolders.log", "w" );

// parse command line args

	if( argc < 2 ) {
		printf( "Usage: XMLExists <xml-file> [options].\n" );
		exit( 42 );
	}

	for( int i = 1; i < argc; ++i ) {

		// echo to log
		fprintf( flog, "%s ", argv[i] );

		if( argv[i][0] != '-' ) {

			if( !infile )
				infile = argv[i];
		}
		else if( GetArg( &zmin, "-zmin=%d", argv[i] ) )
			;
		else if( GetArg( &zmax, "-zmax=%d", argv[i] ) )
			;
		else {
			printf( "Did not understand option '%s'.\n", argv[i] );
			exit( 42 );
		}
	}

	fprintf( flog, "\n\n" );
	fflush( flog );
}

/* --------------------------------------------------------------- */
/* ScanXML ------------------------------------------------------- */
/* --------------------------------------------------------------- */

static void ScanXML()
{
/* ------------- */
/* Load document */
/* ------------- */

	TiXmlDocument	doc( gArgs.infile );
	bool			loadOK = doc.LoadFile();

	if( !loadOK ) {
		fprintf( flog,
		"Could not open XML file [%s].\n", gArgs.infile );
		exit( 42 );
	}

/* ---------------- */
/* Verify <trakem2> */
/* ---------------- */

	TiXmlHandle		hDoc( &doc );
	TiXmlElement*	layer;

	if( !doc.FirstChild() ) {
		fprintf( flog, "No trakEM2 node.\n" );
		exit( 42 );
	}

	layer = hDoc.FirstChild( "trakem2" )
				.FirstChild( "t2_layer_set" )
				.FirstChild( "t2_layer" )
				.ToElement();

	if( !layer ) {
		fprintf( flog, "No first trakEM2 child.\n" );
		exit( 42 );
	}

/* ---- */
/* Scan */
/* ---- */

	set<string>	dirs;

	for( ; layer; layer = layer->NextSiblingElement() ) {

		int	z = atoi( layer->Attribute( "z" ) );

		if( z > gArgs.zmax )
			break;

		if( z < gArgs.zmin )
			continue;

		if( !(z % 100) )
			printf( "z=%6d\n", z );

		// for each tile in this layer...
		for(
			TiXmlElement* ptch =
			layer->FirstChildElement( "t2_patch" );
			ptch;
			ptch = ptch->NextSiblingElement() ) {

			char	buf[4096], sub[256];
			char	*start, *end;

			sprintf( buf, "%s", ptch->Attribute( "file_path" ) );
			end = strstr( buf, "/Plate1_0" );
			end[0] = 0;
			start = strrchr( buf, '/' ) + 1;
			sprintf( sub, "%.*s", end - start, start );
			dirs.insert( string( sub ) );
		}
	}

/* ----- */
/* Print */
/* ----- */

	set<string>::iterator	it = dirs.begin();
	int						nd = dirs.size();

	for( int i = 0; i < nd; ++i, ++it )
		fprintf( flog, "%s\n", it->c_str() );
}

/* --------------------------------------------------------------- */
/* main ---------------------------------------------------------- */
/* --------------------------------------------------------------- */

int main( int argc, char* argv[] )
{
/* ------------------ */
/* Parse command line */
/* ------------------ */

	gArgs.SetCmdLine( argc, argv );

/*----- */
/* Scan */
/*----- */

	ScanXML();

/* ---- */
/* Done */
/* ---- */

exit:
	fprintf( flog, "\n" );
	fclose( flog );

	return 0;
}


