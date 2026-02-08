#!/bin/bash
# Test for Docker runtime stage and .dockerignore

if [ ! -f ".dockerignore" ]; then
    echo "FAIL: .dockerignore does not exist"
    exit 1
fi

grep -q "target/" .dockerignore || { echo "FAIL: target/ missing from .dockerignore"; exit 1; }
grep -q "firmware/" .dockerignore || { echo "FAIL: firmware/ missing from .dockerignore"; exit 1; }

grep -q "FROM gcr.io/distroless/cc-debian12" Dockerfile || { echo "FAIL: Runtime base image incorrect"; exit 1; }

echo "PASS: Runtime optimization and .dockerignore verified"
exit 0
