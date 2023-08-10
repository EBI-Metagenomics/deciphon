# Generate requirements.txt from pyproject.toml
FROM python:3.11 AS builder
RUN pip install --no-cache-dir --upgrade pip pip-tools
RUN mkdir -p deciphon_sched
RUN touch deciphon_sched/__init__.py
RUN touch README.md
COPY pyproject.toml pyproject.toml
RUN pip-compile -o requirements.txt pyproject.toml


FROM python:3.11
COPY --from=builder requirements.txt .
RUN pip install --no-cache-dir --upgrade -r requirements.txt
COPY . /app
WORKDIR /app
CMD [ "uvicorn", "--factory", "deciphon_sched.main:create_app" ]
