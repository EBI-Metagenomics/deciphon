FROM python:3.11 AS base
WORKDIR /app
COPY . .
RUN pip install --no-cache-dir --upgrade -e .

FROM base AS presser
CMD [ "python", "-m", "deciphonctl", "presser", "run" ]

FROM base AS scanner
CMD [ "python", "-m", "deciphonctl", "scanner", "run" ]
