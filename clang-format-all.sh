#!/bin/bash
find src test examples -type f \( -name "*.hpp" -o -name "*.cpp" \) -exec clang-format -i {} \;