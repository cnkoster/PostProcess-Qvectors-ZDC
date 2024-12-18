#!/bin/bash

#Define the input file
#runs uploaded: 544116 544122 (13-12-2024)
#runs not finished yet: 544098 544121 544123 544124 (13-12-2024)
for run in 544098 544121 544123 544124 
do
    input_file="run_duration_${run}.txt"

    if [[ ! -f $input_file ]]; then
        echo "Error: Input file '${input_file}' not found!"
        exit 1
    fi

    host="alice-ccdb.cern.ch"
    # "StandardCut" "SmallCut" "Occupancy1000" (al gedaan)

    for it in 1 2 3 4 5
    do
        for step in 1 2 3 4 5
        do
        # Skip the first line (header)
        tail -n +2 "$input_file" | while read -r run start end; do
            # Print the values or process them as needed
            echo "Run: $run, Start Time: $start, End Time: $end"
            o2-ccdb-upload --host ${host} -p Users/c/ckoster/ZDC/LHC23_zzh_pass4/it${it}_step${step}/ -f ${run}/Calibration_constants_it${it}_step${step}.root -k "ccdb_object" --starttimestamp ${start} --endtimestamp ${end}
            done
        done
    done
done

# For energy and vmean histos!! All runs available in histos! 
#LHC23zzh 544095-544124: 1696527167467 1696579335154
# for mode in "Energy" "vmean"
# do
# o2-ccdb-upload --host ${host} -p Users/c/ckoster/ZDC/LHC23_zzh_pass4/${mode}/ -f Calibration_constants_${mode}.root -k "ccdb_object" --starttimestamp 1696527167467 --endtimestamp 1696579335154
# done
