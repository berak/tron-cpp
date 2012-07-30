#include <stdio.h>
#include "db.h"

	void Sqlite::_clearResult()
	{
		this->ncols = this->nrows = 0;
		if ( this->cbuf )
			sqlite3_free_table( this->cbuf );
		this->cbuf = 0;
	}
	int Sqlite::_error( const char * fn, const char * txt )
	{
		const char * err = sqlite3_errmsg( this->sqlite );
		printf( "%s(%s) : error( %s )\n", fn, txt, err );
		return 0; 
	}

	Sqlite::Sqlite() 
		: sqlite(0)
		, cbuf(0)
		, nrows(0)
		, ncols(0)
	{
		// printf( "<sqlite %s>\n", sqlite3_libversion() );
	}

	Sqlite::~Sqlite() 
	{
		close();
	}


	int Sqlite::open( const char * db, const char * host, const char * user, const char * pw ) 
	{
		if ( this->sqlite )
		{
			return _error(__FUNCTION__, "close running connection first");
		}
		int r = sqlite3_open( db, &this->sqlite );
		if ( r != SQLITE_OK )
		{
			return _error(__FUNCTION__);
		}
		return 1; 
	}

	int Sqlite::query( const char * statement ) 
	{
		if ( ! this->sqlite ) 
		{
			return _error(__FUNCTION__,"DB_CLOSED");
		}

		Sqlite::_clearResult();

		char * msg = 0;
		int r = sqlite3_get_table( this->sqlite, statement, &this->cbuf, &this->nrows, &this->ncols, &msg );
		if ( r != SQLITE_OK )
		{
			return _error(__FUNCTION__);
		}
		return 1;
	}

	int Sqlite::numCols() const 
	{
		return this->ncols;
	}
	int Sqlite::numRows() const 
	{
		return this->nrows;
	}
	const char *Sqlite::colName( int col ) const 
	{
		if ( ! this->cbuf ) return 0;
		if ( (int)col >= this->ncols ) return 0;
		return this->cbuf[col];
	}
	const char *Sqlite::getItem( int row, int col ) const 
	{
		if ( ! this->cbuf ) return 0;
		if ( (int)row >= this->nrows ) return 0;
		if ( (int)col >= this->ncols ) return 0;
		return this->cbuf[(row+1)*this->ncols + col];
	}

	int Sqlite::close() 
	{
		Sqlite::_clearResult();

		if ( ! this->sqlite ) return 0;

		int r = sqlite3_close(this->sqlite);
		this->sqlite = 0;
		return 1;
	}


