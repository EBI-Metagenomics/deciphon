#!/bin/bash

HMM=minifam.hmm
SCHED_HOST=127.0.0.1
SCHED_PORT=1515
S3_HOST=127.0.0.1
S3_PORT=9000

cleanup() {
  rv=$?
  if [ $rv -eq 0 ]
  then
    echo 🎉 Success! 🎉
  else
    echo 🔥 Failure! 🔥
  fi
  rm -rf actual.txt || true
  exit $rv
}
trap "cleanup" EXIT
cd "$(dirname "$0")"

what() {
  echo -n "$1... "
}

ok() {
  echo "done. ✅"
}

fail() {
  exit 1
}

what "Checking scheduler status"
./wait-for http://$SCHED_HOST:$SCHED_PORT -t 30 || fail
ok

what "Fetching presigned-url"
data=$(
curl --no-progress-meter \
  -X 'GET' "http://$SCHED_HOST:$SCHED_PORT/hmms/presigned-upload/$HMM" \
  -H 'accept: application/json'
) || fail
ok

what "Parsing presigned response"
form=$(echo $data | jq .fields | jq -r '[to_entries[] | ("-F " + "\(.key)" + "=" + "\(.value)")] | join(" ")') || fail
ok

what "Uploading HMM file"
curl --no-progress-meter -X POST $form -F file=@$HMM http://$S3_HOST:$S3_PORT/deciphon || fail
ok

what "Posting HMM file"
data=$(
curl --no-progress-meter \
  -X 'POST' "http://$SCHED_HOST:$SCHED_PORT/hmms/?gencode=1&epsilon=0.01" \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d "{\"name\": \"$HMM\"}"
) || fail
ok

job_state()
{
  curl --no-progress-meter \
    -X 'GET' "http://$1:$2/jobs/$3" \
    -H 'accept: application/json' | jq -r .state
}
export -f job_state

wait_job_done()
{
  while [ "$(job_state $@)" != "done" ]
  do
    sleep 1
  done
}
export -f wait_job_done

what "Waiting for job of pressing HMM"
timeout 10s bash -c "wait_job_done $SCHED_HOST $SCHED_PORT 1" || fail
ok


what "Submitting a scan request"
curl --no-progress-meter \
  -X 'POST' "http://$SCHED_HOST:$SCHED_PORT/scans/" \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d @scan.json > /dev/null || fail
ok

what "Waiting for job of scanning to be done"
timeout 10s bash -c "wait_job_done $SCHED_HOST $SCHED_PORT 2" || fail
ok

what "Viewing scan alignment"
curl --no-progress-meter \
  -X 'GET' "http://$SCHED_HOST:$SCHED_PORT/scans/1/snap.dcs/view" \
  -H 'accept: application/json' > actual.txt || fail
ok

what "Diffing desired and actual output from alignment"
diff desired.txt actual.txt || fail
ok
