FROM python:3.11 AS base
WORKDIR /app
COPY . .
RUN pip install --use-pep517 --no-cache-dir --upgrade -e .
ENTRYPOINT [ "deciphon-worker" ]
