
#include <iod/mysql/mysql.hh>


int main()
{
  auto c = mysql_connection(s::host = "localhost",
                            s::database = "iod_test",
                            s::user = "iod",
                            s::password = "iod",
                            s::charset = "utf8");

  int test;
  c("SELECT 1 + 2") >> test;

  assert(test == 3);
}
