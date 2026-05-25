FROM gcc:latest

WORKDIR /app

COPY oops.cpp .

RUN g++ oops.cpp -o smartbank

CMD ["./smartbank"]
