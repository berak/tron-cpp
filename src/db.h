#ifndef __db_onboard__
#define __db_onboard__


#include "sqlite3.h"


class Sqlite
{
private:
	sqlite3 * sqlite;

	int ncols;
	int nrows;
	char ** cbuf;

	void _clearResult();
	int  _error( const char * fn, const char * txt="" );
public:
	Sqlite() ;
	~Sqlite() ;

	int open( const char * db, const char * host, const char * user, const char * pw ) ;
	int query( const char * statement ) ;
	int numCols() const ;
	int numRows() const ;
	const char *colName( int col ) const ;
	const char *getItem( int row, int col ) const ;

	int close();

}; //Sqlite


#endif // __db_onboard__

