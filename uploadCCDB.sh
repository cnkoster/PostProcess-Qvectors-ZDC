#!/bin/bash

# Define the input file
run=${1}
input_file="run_duration_${run}.txt"

if [[ ! -f $input_file ]]; then
    echo "Error: Input file '${input_file}' not found!"
    exit 1
fi

host="alice-ccdb.cern.ch"
# "StandardCut" "SmallCut" "Occupancy1000" (al gedaan)

for it in 1
do
for step in 1 2 3 4 5
do
wagon="it${it}_step${step}"
# Skip the first line (header)
tail -n +2 "$input_file" | while read -r run start end; do
    # Print the values or process them as needed
    echo "Run: $run, Start Time: $start, End Time: $end"
    o2-ccdb-upload --host ${host} -p Users/c/ckoster/ZDC/LHC23_zzh_pass4/${wagon}/ -f ${run}/Calibration_constants_${wagon}.root -k "ccdb_object" --starttimestamp 1696527167467 --endtimestamp 1696579335154
    done
 done
done