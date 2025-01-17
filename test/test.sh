#!/bin/sh
cd "$(dirname "${0}")" || { echo "Test failed."; exit 1; }
../gunzip_demo sample.txt.gz sample_output || { echo "Test failed."; exit 1; }
md5sum -c --quiet MD5SUM || { rm -f sample_output; echo "Test failed."; exit 1; }
rm -f sample_output || { echo "Test failed."; exit 1; }
echo "Test succeeded."
exit 0