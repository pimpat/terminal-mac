#include "stdio.h"
#include "stdlib.h"
#include "sqlite3.h"
#include "string"

#include "Database.h"
#include "Query.h"

int main(int argc, char* argv[])
{
	Database db( "testdata.db" );
	Query q(db);

	q.execute("insert into testdata values('5','boom')");
	q.execute("insert into testdata values('6','pluem')");
	q.execute("insert into testdata values('7','arm')");
	q.execute("insert into testdata values('8','oey')");

	q.get_result("select id,name from testdata");
	while (q.fetch_row())
	{
		std::int id = q.getstr();
		std::string name = q.getstr();
		printf("=%s,%s\n", id.c_str(),name.c_str());
	}
	q.free_result();
	return 0;
	return 0;
}
