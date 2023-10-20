# Using Docker Compose to run Deciphon server ⚙️

Deciphon server requires five different services to work:
- [scheduler](https://github.com/EBI-Metagenomics/deciphon/tree/main/sched) to receive requests via its REST API;
- [presser](https://github.com/EBI-Metagenomics/deciphon/tree/main/control) to generate protein databases from HMM files;
- [scanner](https://github.com/EBI-Metagenomics/deciphon/tree/main/control) to scan sequences against protein databases;
- [s3](https://min.io) server to store protein databases;
- [mqtt](https://mosquitto.org) server to orchestrate message sharing.

## Quick start ⚡

Enter

```sh
docker compose --env-file compose.cfg up
```

to start the services with configuration set by [compose.cfg](compose.cfg).
It will expose the ports `8000` and `9000` for the REST API and S3 server by default.
The REST API documentation can then be access via [http://127.0.0.1:8000/docs](http://127.0.0.1:8000/docs).

One can use [deciphonctl](https://pypi.org/project/deciphonctl/) to interact with it.
Install it first:

```sh
pip install deciphonctl
```

Then configure and run it:

```
# Configure the controller
export DECIPHONCTL_SCHED_URL=http://127.0.0.1:8000
export DECIPHONCTL_S3_URL=http://127.0.0.1:9000/deciphon

# Upload example.hmm file and process its proteins according to
# The Bacterial, Archaeal and Plant Plastid Code (NCBI code 11)
deciphonctl hmm add example.hmm 11

# List HMM files
deciphonctl hmm ls
```

## Web page

A web application will only need to interact with the REST API via port `8000` by default.
One must take care however of setting the variable `DECIPHON_SCHED_ALLOW_ORIGINS` in `compose.cfg`
property for Cross-Origin Resource Sharing.

## 👤 Author

- [Danilo Horta](https://github.com/horta)

## Show your support

Give a ⭐️ if this project helped you!
