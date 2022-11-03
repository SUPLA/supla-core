FROM debian:11.3

RUN apt-get update 
RUN apt-get upgrade -y
RUN apt-get -y install git vim wget procps build-essential libssl-dev libmariadb-dev-compat mariadb-client net-tools valgrind cppcheck cmake clang-format python3-pip rsyslog libgtest-dev libgmock-dev
RUN pip install cpplint  
RUN mkdir -p /etc/supla-server

WORKDIR /CProjects/supla-core/supla-server

COPY supla.cfg /etc/supla-server/
COPY supla-test.cfg /etc/supla-server/
COPY private.key /etc/supla-server/
COPY cert.crt /etc/supla-server/
