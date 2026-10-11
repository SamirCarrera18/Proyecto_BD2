FROM postgres:18.6-bookworm
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential postgresql-server-dev-18 python3 python3-psycopg2 \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /opt/ttree
COPY . .
RUN make PG_CONFIG=/usr/lib/postgresql/18/bin/pg_config && \
    make PG_CONFIG=/usr/lib/postgresql/18/bin/pg_config install
