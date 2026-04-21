#! /bin/sh

set -eu


rm -rf /usr/local/var/postgres
mkdir -p /usr/local/var/postgres
chown postgres:postgres /usr/local/var/postgres

# docker build -t docker_linux_test .
#docker run -it -v $HOME/lithium:/lithium docker_linux_test
PG_BINDIR="/usr/lib/postgresql/14/bin"
if [ ! -x "$PG_BINDIR/pg_ctl" ]; then
	PG_BINDIR="$(dirname "$(command -v pg_ctl)")"
fi

sudo -u postgres "$PG_BINDIR/initdb" /usr/local/var/postgres
sudo -u postgres "$PG_BINDIR/pg_ctl" -D /usr/local/var/postgres -o "-p 32768" start

i=0
while [ $i -lt 30 ]; do
	if sudo -u postgres "$PG_BINDIR/pg_isready" -h 127.0.0.1 -p 32768 >/dev/null 2>&1; then
		break
	fi
	i=$((i + 1))
	sleep 1
done

sudo -u postgres "$PG_BINDIR/pg_isready" -h 127.0.0.1 -p 32768
sudo -u postgres "$PG_BINDIR/psql" -p 32768 -c "ALTER ROLE postgres WITH LOGIN ENCRYPTED PASSWORD 'lithium_test';" postgres


TMPDIR=/tmp/lithium_mariadb_test
rm -fr $TMPDIR
sudo -u mysql mkdir -p $TMPDIR

CONF=/lithium/libraries/sql/tests/test_mariadb.cnf
basedir="/usr"
sudo -u mysql mysql_install_db --auth-root-authentication-method=normal --defaults-file=$CONF --skip-test-db --basedir=$basedir
if command -v mariadbd >/dev/null 2>&1; then
	MYSQLD_BIN="$(command -v mariadbd)"
else
	MYSQLD_BIN="$(command -v mysqld)"
fi

MYSQL_SSL_ARGS=""
if [ "${LI_MYSQL_SSL:-0}" = "1" ]; then
	SSL_DIR="$TMPDIR/ssl"
	mkdir -p "$SSL_DIR"

	# Self-signed CA and server certs for local test usage.
	openssl req -newkey rsa:2048 -nodes -x509 -days 3650 \
		-subj "/CN=lithium-test-ca" \
		-keyout "$SSL_DIR/ca-key.pem" -out "$SSL_DIR/ca.pem" >/dev/null 2>&1
	openssl req -newkey rsa:2048 -nodes -days 3650 \
		-subj "/CN=127.0.0.1" \
		-keyout "$SSL_DIR/server-key.pem" -out "$SSL_DIR/server-req.pem" >/dev/null 2>&1
	openssl x509 -req -in "$SSL_DIR/server-req.pem" -days 3650 \
		-CA "$SSL_DIR/ca.pem" -CAkey "$SSL_DIR/ca-key.pem" -set_serial 01 \
		-out "$SSL_DIR/server-cert.pem" >/dev/null 2>&1

	chmod 600 "$SSL_DIR/"*.pem
	chown -R mysql:mysql "$SSL_DIR"

	MYSQL_SSL_ARGS="--ssl-ca=$SSL_DIR/ca.pem --ssl-cert=$SSL_DIR/server-cert.pem --ssl-key=$SSL_DIR/server-key.pem"
fi

# Keep server alive after this setup script exits.
if [ -n "$MYSQL_SSL_ARGS" ]; then
	# shellcheck disable=SC2086
	nohup "$MYSQLD_BIN" --defaults-file=$CONF --user=mysql $MYSQL_SSL_ARGS >/tmp/lithium_mariadb_test/mysqld.stdout.log 2>&1 &
else
	nohup "$MYSQLD_BIN" --defaults-file=$CONF --user=mysql >/tmp/lithium_mariadb_test/mysqld.stdout.log 2>&1 &
fi

i=0
while [ $i -lt 30 ]; do
	if sudo -u mysql mysqladmin --defaults-file=$CONF --protocol=tcp -h 127.0.0.1 -P 14550 -u root ping >/dev/null 2>&1; then
		break
	fi
	i=$((i + 1))
	sleep 1
done

sudo -u mysql mysqladmin --defaults-file=$CONF --protocol=tcp -h 127.0.0.1 -P 14550 -u root ping
sudo -u mysql mysqladmin --defaults-file=$CONF --protocol=tcp -h 127.0.0.1 -P 14550 -u root password 'lithium_test' # root password
sudo -u mysql mysqladmin --defaults-file=$CONF --protocol=tcp -h 127.0.0.1 -P 14550 -u root --password=lithium_test ping
sudo -u mysql mysqladmin --defaults-file=$CONF --protocol=tcp -h 127.0.0.1 -P 14550 -u root --password=lithium_test create mysql_test # create test database
