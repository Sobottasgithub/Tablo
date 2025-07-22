# Use the official Ubuntu image as the base image
FROM ubuntu:latest

WORKDIR /app

RUN apt-get update && apt-get install -y \
    g++ \
    libcpprest-dev \
    libboost-all-dev \
    libssl-dev \
    cmake
COPY utils/* utils/.
COPY tabloNode.cpp .

RUN g++ -o tabloNode utils/*.cpp tabloNode.cpp -lcpprest -lboost_system -lboost_thread -lboost_chrono -lboost_random -lssl -lcrypto

EXPOSE 8080

CMD ["./tabloNode"]
