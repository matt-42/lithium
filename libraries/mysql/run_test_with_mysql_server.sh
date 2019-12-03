
TMPDIR=$(mktemp -d)
#
mkdir $TMPDIR/data
#
mkdir $TMPDIR/base
CONF=$TMPDIR/my.cnf
cat > $CONF <<- EOF
[client]
port                           = 14550
socket                           = $TMPDIR/mysqld.sock

[mysql]
default_character_set          = utf8                                # Possibly this setting is correct for most recent Linux systems

[mysqld_safe]                                                        # Becomes sooner or later obsolete with systemd
open_files_limit               = 8192                                # You possibly have to adapt your O/S settings as well
user                           = matthieugarrigues
log-error                      = $TMPDIR/error.log   # Adjust AppArmor configuration: /etc/apparmor.d/local/usr.sbin.mysqld
socket                           = $TMPDIR/mysqld.sock
port                           = 14550

[mysqld]
# Connection and Thread variables
socket                           = $TMPDIR/mysqld.sock
port                           = 14550
basedir                        = $TMPDIR/base
datadir                        = $TMPDIR/data
lc_messages_dir                = /usr/local/share/mysql
lc_messages = en_US

max_allowed_packet             = 16M
default_storage_engine         = InnoDB

log_error                      = $TMPDIR/errord.log
pid_file                       = $TMPDIR/server.pid
EOF

mysqld --initialize-insecure --initialize --datadir=$TMPDIR/data 
mysqld_safe --defaults-file=$CONF --pid-file=$TMPDIR/server.pid &
sleep 1
mysqladmin  --defaults-file=$CONF -u root password sl_test_password
#echo $CONF
cat $TMPDIR/error.log

#PID=$(cat $TMPDIR/server.pid)
#kill -9 $PID
mysqladmin  --defaults-file=$CONF -u root -p=sl_test_password shutdown
