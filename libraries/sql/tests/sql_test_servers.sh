
#! /bin/bash

if [ ! -d "./libraries" -o ! -f "README.md" ]; then
  echo "This must be run from the iod base directory."
fi


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

  docker start lithium-postgres-test 2>&1 > /dev/null
  if [ $? -ne 0 ]; then 
    docker pull postgres
    docker run -p=32768:5432 --name lithium-postgres-test -e POSTGRES_PASSWORD=lithium_test -d postgres
  fi

  # docker start lithium-mysql-test 2>&1 > /dev/null
  # # echo $?
  # if [ $? -ne 0 ]; then 
  #   docker pull mysql:latest
  #   docker run -p=14550:3306 --name lithium-mysql-test --health-cmd='mysqladmin ping --silent' -e MYSQL_ROOT_HOST=% -e MYSQL_DATABASE=mysql_test -e MYSQL_ROOT_PASSWORD=lithium_test -d mysql:latest

  #   echo "Waiting for MySQL to start"
  #   while [ $(docker inspect --format "{{json .State.Health.Status }}" lithium-mysql-test) != "\"healthy\"" ]; do 
  #       printf "."; sleep 1;
  #     done
  #   echo "MySQL is now ready."
  # fi 


  rm -fr $TMPDIR
  mkdir -p $TMPDIR/data
  CONF=$TMPDIR/my.cnf

  if [[ "$OSTYPE" == "msys" ]]; then
    TMPDIR=$(echo $TMPDIR | sed 's/\/c\//C:\//')
    echo $TMPDIR
  fi

  cat > $CONF <<- EOF
[Service]
LimitNOFILE=8000
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
open_files_limit =        4294967295
socket                           = $TMPDIR/mysqld.sock
port                           = 14550
datadir                        = $TMPDIR/data
log_error                      = $TMPDIR/error.log
max_connections                = 99999
max_allowed_packet            = 512M
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

  mysqladmin --defaults-file=$CONF -u root password 'lithium_test' # root password
  mysqladmin --defaults-file=$CONF -u root --password=lithium_test create mysql_test # create test database


  echo "SQL test servers ready for testing."

elif [ $1 == "stop" ]; then

  #docker stop lithium-mysql-test
  docker stop lithium-postgres-test

  mysqladmin -P 14550 -h "127.0.0.1" -u root --password=lithium_test shutdown
  rm -fr $TMPDIR

fi
