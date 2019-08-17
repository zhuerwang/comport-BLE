#include <stdio.h>
#include <sqlite3.h>
#include <unistd.h>
#include <fcntl.h>
#include "database_init.h"



int database_init(void)
{
    int             rv = -1;
    int             ret = -1;

    rv = access("./ble.db", F_OK);

	ret = sqlite3_open("ble.db", &db);
	if( ret )
	{	
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1;
	}
	else
	{
		fprintf(stdout, "Opened database successfully.\n");
	}
    
    if(rv == -1)
    {
	    char *sql_s =   "CREATE TABLE IF NOT EXISTS STUDENT(" \
			    "NUMBER  INTEGER PRIMARY KEY     AUTOINCREMENT,"\
			    "SN      CHAR(64)    NOT NULL,"\
			    "TEMPER    CHAR(64)    NOT NULL,"\
			    "HUMIDITY  CHAR(64)    NOT NULL,"
                "TIME CHAR(64)   NOT NULL);";		   

	    rv = sqlite3_exec(db, sql_s, 0, 0, &zErrMsg);
	    if( rv != SQLITE_OK )
	    {
		    fprintf(stderr, "SQL error: %s\n", zErrMsg);
		    sqlite3_free(zErrMsg);
            return -1;
	    }
	    else
	    {
		    fprintf(stdout, "Table create successfully\n");
	    }
    }

    return 1;
}
