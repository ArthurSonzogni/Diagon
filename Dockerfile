FROM ubuntu:24.04 AS builder

ARG ANTLR_VERSION=4.13.2
ENV ANTLR_VERSION=$ANTLR_VERSION

RUN apt update && \
    apt install -y make libboost-graph-dev cmake default-jdk git g++ wget
COPY . /Diagon
RUN mkdir -p Diagon/build && \
    cd Diagon/build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    wget -O antlr.jar https://www.antlr.org/download/antlr-$ANTLR_VERSION-complete.jar && \
    make -j install

FROM ubuntu:24.04 AS runner
COPY --from=builder /usr/local/bin/diagon /usr/local/bin/diagon
RUN useradd -u 1101 diagon
USER 1101
ENTRYPOINT [ "diagon" ]
