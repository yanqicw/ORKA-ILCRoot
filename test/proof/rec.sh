#!/bin/bash

[ -z $1 ] && { echo "Usage: rec.sh <run_number>"; exit 1; }

ilcroot -q run.C\($1\)
