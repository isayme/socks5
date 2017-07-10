FROM debian:latest

RUN apt-get update
RUN apt-get install build-essential -y
RUN apt-get install gdb -y
RUN apt-get install procps -y

RUN apt-get install tzdata
ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN dpkg-reconfigure -f noninteractive tzdata

WORKDIR /app
