FROM python:3.11 AS deciphon/sched
WORKDIR /app
COPY . .
RUN pip install --no-cache-dir --upgrade -e .
CMD [ "uvicorn", "--factory", "deciphon_sched.main:create_app" ]
