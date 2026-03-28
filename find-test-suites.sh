#!/bin/bash

find test/unit -type d -name "test_*" | sed 's|^test/||' | sort | sed 's/.*/"&"/' | paste -sd, | sed 's/.*/[&]/'