FROM postgres:18

# Instalar herramientas de compilación para C y desarrollo de PostgreSQL
RUN apt-get update && apt-get install -y \
    gcc \
    make \
    postgresql-server-dev-18 \
    valgrind \
    && rm -rf /var/lib/apt-lists/*

WORKDIR /usr/src/ttree