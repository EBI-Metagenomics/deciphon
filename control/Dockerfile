FROM python:3.11 AS base
WORKDIR /app
COPY . .
RUN pip install --no-cache-dir --upgrade -e .

FROM base AS deciphon/presser
CMD [ "python", "-m", "deciphonctl", "presser", "run" ]

FROM base AS deciphon/scanner
CMD [ "python", "-m", "deciphonctl", "scanner", "run" ]
