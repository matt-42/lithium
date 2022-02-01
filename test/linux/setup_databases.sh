#! /bin/sh


rm -rf /usr/local/var/postgres
mkdir -p /usr/local/var/postgres
chown postgres:postgres /usr/local/var/postgres

# docker build -t docker_linux_test .
#docker run -it -v $HOME/lithium:/lithium docker_linux_test
sudo -u postgres /usr/lib/postgresql/12/bin/initdb /usr/local/var/postgres;
sudo -u postgres /usr/lib/postgresql/12/bin/pg_ctl -D /usr/local/var/postgres -o "-p 32768" start;
sudo -u postgres /usr/lib/postgresql/12/bin/psql -p 32768 -c "ALTER ROLE postgres WITH LOGIN ENCRYPTED PASSWORD 'lithium_test';" postgres;


TMPDIR=/tmp/lithium_mariadb_test
rm -fr $TMPDIR
sudo -u mysql mkdir -p $TMPDIR

CONF=/lithium/libraries/sql/tests/test_mariadb.cnf
basedir="/usr"
sudo -u mysql mysql_install_db --auth-root-authentication-method=normal --defaults-file=$CONF --skip-test-db --basedir=$basedir
#sudo -u mysql mysqld --defaults-file=$CONF &
mysqld --defaults-file=$CONF --user=root &

sleep 2 # wait for server to start 

sudo -u mysql mysqladmin --defaults-file=$CONF -u root password 'lithium_test' # root password
sudo -u mysql mysqladmin --defaults-file=$CONF -u root --password=lithium_test create mysql_test # create test database
