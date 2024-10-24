FROM python:3.11 AS base
WORKDIR /app
COPY . .
RUN pip install --use-pep517 --no-cache-dir --upgrade -e .

FROM base AS presser
ENTRYPOINT [ "deciphon-worker", "presser" ]
CMD [ "http://127.0.0.1:80", "127.0.0.1", "1883" ]

FROM base AS scanner
ENTRYPOINT [ "deciphon-worker", "scanner" ]
CMD [ "http://127.0.0.1:80", "127.0.0.1", "1883" ]
