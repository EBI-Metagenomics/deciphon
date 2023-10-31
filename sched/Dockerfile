FROM python:3.11 AS sched
WORKDIR /app
COPY . .
RUN pip install --use-pep517 --no-cache-dir --upgrade -e .
