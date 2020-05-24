#! /bin/bash

if [ $# -eq 0 ]; then
  echo "Start or stop the mysql test server."
  echo
  echo "Usage: " $0 "start|stop"
  exit 1
fi

if [ $1 == "start" ]; then

  docker pull mysql:latest
  docker run -p=14550:3306 --name lithium-mysql-test --health-cmd='mysqladmin ping --silent' -e MYSQL_ROOT_HOST=% -e MYSQL_DATABASE=mysql_test -e MYSQL_ROOT_PASSWORD=lithium_test -d mysql:latest

  while [ $(docker inspect --format "{{json .State.Health.Status }}" lithium-mysql-test) != "\"healthy\"" ]; do 
    printf "."; sleep 1;
  done
fi

