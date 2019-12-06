#! /bin/bash

if [ ! -d "./libraries" -o ! -f "README.md" ]; then
  echo "This must be run from the iod base directory."
fi

#TMPDIR=$(mktemp -d)
#
TMPDIR=$PWD/build/mysql_test_db
# Mariadb version.
if [ -d "/usr/share/mysql" ]; then
  basedir="/usr"
elif [ -d "/usr/local/share/mysql" ]; then
  basedir="/usr/local"
elif [ -d "/c/Program Files (x86)/MariaDB 10.4" ]; then
  basedir="/c/Program Files (x86)/MariaDB 10.4/"
  export PATH="${PATH}:/c/Program Files (x86)/MariaDB 10.4/bin"
fi

if [ $# -eq 0 ]; then
  echo "Start or stop the mysql test server."
  echo
  echo "Usage: " $0 "start|stop"
  exit 1
fi

if [ $1 == "start" ]; then

  rm -fr $TMPDIR
  mkdir -p $TMPDIR/data
  CONF=$TMPDIR/my.cnf

  if [[ "$OSTYPE" == "msys" ]]; then
    TMPDIR=$(echo $TMPDIR | sed 's/\/c\//C:\//')
    echo $TMPDIR
  fi

  cat > $CONF <<- EOF
[client]
port                           = 14550
socket                           = $TMPDIR/mysqld.sock

[mysql]
default_character_set          = utf8                                

[mysqld_safe]
log-error                      = $TMPDIR/error.log
socket                           = $TMPDIR/mysqld.sock
port                           = 14550

[mysqld]
socket                           = $TMPDIR/mysqld.sock
port                           = 14550
datadir                        = $TMPDIR/data
log_error                      = $TMPDIR/error.log
EOF


  if [[ "$OSTYPE" == "msys" ]]; then
    TMPDIR=$(echo $TMPDIR | sed 's/\/c\//C:\//')
    echo $TMPDIR
    mysql_install_db --datadir=$TMPDIR/data --allow-remote-root-access --port=14550
  else
    mysql_install_db --auth-root-authentication-method=normal --defaults-file=$CONF --skip-test-db --basedir=$basedir
  fi
  mysqld --defaults-file=$CONF &
  sleep 2 # wait for server to start 
  mysqladmin --defaults-file=$CONF -u root password 'sl_test_password' # root password
  mysqladmin --defaults-file=$CONF -u root --password=sl_test_password create silicon_test # create test database

elif [ $1 == "stop" ]; then
  mysqladmin -P 14550 -h "127.0.0.1" -u root --password=sl_test_password shutdown
  rm -fr $TMPDIR
fi

# run test.
#sleep 5
#mysqladmin  --defaults-file=$CONF -u root --password=sl_test_password shutdown

exit 0
