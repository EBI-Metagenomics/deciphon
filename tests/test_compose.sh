#!/bin/bash
set -e

HMM=minifam.hmm

data=$(
curl -s -X 'GET' \
  "http://localhost:8000/hmms/presigned-upload/$HMM" \
  -H 'accept: application/json'
)

url=$(echo $data | jq '.url' -r)
form=$(echo $data | jq .fields | jq -r '[to_entries[] | ("-F " + "\(.key)" + "=" + "\(.value)")] | join(" ")')

curl -s -X POST $form -F file=@$HMM $url

curl -s -X 'POST' \
  'http://localhost:8000/hmms/?gencode=1&epsilon=0.01' \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d "{\"name\": \"$HMM\"}"


function job_state()
{
  curl -s -X 'GET' \
    "http://localhost:8000/jobs/$1" \
    -H 'accept: application/json' | jq -r .state
}
export -f job_state

function wait_job_done()
{
  while [ "$(job_state $1)" != "done" ]
  do
    sleep 1
  done
}
export -f wait_job_done

timeout 5s bash -c "wait_job_done 1"

curl -s -X 'POST' \
  'http://localhost:8000/scans/' \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d @scan_post.json

timeout 5s bash -c "wait_job_done 2"

curl -s -X 'GET' \
  'http://localhost:8000/scans/1/snap.dcs/view' \
  -H 'accept: application/json' > actual.txt

diff desired.txt actual.txt
