#!/bin/bash
# find-test-suites.sh – finds all test suite directories and outputs a JSON array

set -e

find test/unit -type d -name "test_*" | sort | sed 's/.*/"&"/' | paste -sd, | sed 's/.*/[&]/'