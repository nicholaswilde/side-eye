#!/bin/bash
# Test for Dockerfile existence and basic multi-stage structure

if [ ! -f "Dockerfile" ]; then
    echo "FAIL: Dockerfile does not exist"
    exit 1
fi

grep -q "AS chef" Dockerfile || { echo "FAIL: Stage 'chef' missing"; exit 1; }
grep -q "AS planner" Dockerfile || { echo "FAIL: Stage 'planner' missing"; exit 1; }
grep -q "AS builder" Dockerfile || { echo "FAIL: Stage 'builder' missing"; exit 1; }
grep -q "AS runtime" Dockerfile || { echo "FAIL: Stage 'runtime' missing"; exit 1; }

echo "PASS: Dockerfile has basic multi-stage structure"
exit 0
