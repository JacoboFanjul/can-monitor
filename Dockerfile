FROM alpine:3.11 AS builder

# Install required dependencies 
RUN apk add --update --no-cache build-base wget git gcc cmake make yaml-dev libcurl curl-dev libmicrohttpd-dev util-linux-dev jsoncpp-dev openssl-dev ninja linux-headers musl-dev && mkdir -p /adms-build/build

RUN git clone https://github.com/eclipse/paho.mqtt.c.git && \
    cd paho.mqtt.c && \
    mkdir build && \
    cd build && \
    cmake -GNinja -DPAHO_WITH_SSL=TRUE -DPAHO_BUILD_DOCUMENTATION=FALSE -DPAHO_BUILD_SAMPLES=TRUE -DPAHO_HIGH_PERFORMANCE=TRUE .. && \
    ninja install && \
    ldconfig /etc/ld.so.conf.d 
# RUN git clone https://github.com/eclipse/paho.mqtt.c.git && \
#    cd paho.mqtt.c && \
#    make && \
#    make install && \
#    ldconfig

# Set 
COPY VERSION /adms-build
COPY LICENSE /adms-build
COPY Attribution.txt /adms-build
COPY src /adms-build/src
COPY build.sh /adms-build
WORKDIR /adms-build
RUN ./build.sh $*
# ENTRYPOINT ["/bin/sh"]

FROM alpine:3.11 

# Install required dependencies
RUN apk add --update --no-cache yaml libcurl libmicrohttpd util-linux openssl && mkdir -p /adeptness

# Copy project directories
COPY --from=builder /adms-build/build /adeptness/bin/build
COPY --from=builder /usr/local/lib*/libpaho* /usr/local/lib/

COPY conf /adeptness/conf

# Define the working directory
WORKDIR /adeptness/bin/build/release

# Expose container port
EXPOSE 48890

# Run executable parsing the configuration file (this file can be modified when running the docker image)
#ENTRYPOINT ["/bin/sh"]
ENTRYPOINT ["./CanMonitor"]
