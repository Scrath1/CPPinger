FROM gcc:11.2.0 AS builder
RUN mkdir -p /app/
COPY ./ /app/
WORKDIR /app/

RUN apt-get update && apt-get install -y --no-install-recommends cmake gdb ssh rsync libpq-dev postgresql-server-dev-all
RUN cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
RUN cmake --build build || :
RUN ls -a /app/build
CMD ["/app/build/CPPinger"]

#FROM alpine:latest
#RUN mkdir -p /app/
#WORKDIR /app/
#COPY --from=builder /app/build/CPPinger ./
#RUN ls -a
#CMD ["./CPPinger"]
