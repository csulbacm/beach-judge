#!/bin/bash
gcj $1 --main=aout -o $2 2>&1 | grep -B 3 -A 2 "error:" > $3
