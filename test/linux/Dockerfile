FROM buildpack-deps:jammy
RUN apt-get update && apt-get install -yqq bison flex libboost-context-dev libboost-dev wget libmariadb-dev\
            postgresql-server-dev-14 postgresql-14 libpq-dev cmake sudo mariadb-server clang \
            curl zip unzip tar

RUN clang++  --version
RUN rm -rf /usr/local/var/postgres
RUN mkdir -p /usr/local/var/postgres
RUN chown postgres:postgres /usr/local/var/postgres
