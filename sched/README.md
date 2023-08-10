```
docker-compose up -d --build --force-recreate --remove-orphans
docker-compose down -v
```

```
docker run -p 9000:9000 -p 9001:9001 quay.io/minio/minio server /data --console-address ":9001"
docker run -p 1883:1883 -v /Users/horta/tmp/mosquitto/config:/mosquitto/config eclipse-mosquitto
uvicorn --factory deciphon_sched.main:create_app
```
