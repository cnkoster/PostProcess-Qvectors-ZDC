iteration=$1
step=$2
run=$3

SUB_FILE=testjob1.sub

JOB_ID=$(condor_submit "$SUB_FILE" -append "PARAM1=${iteration}" -append "PARAM2=${run}" | grep -oP '(?<=submitted to cluster )\d+')
echo "Submitted job ID $JOB_ID"


# Wait for all jobs to complete
echo "Waiting for jobs to complete..."
while true; do
    ALL_DONE=true
    if condor_q | grep -q "$JOB_ID"; then
        ALL_DONE=false
    fi
    # Break the loop if all jobs are done
    if [ "$ALL_DONE" = true ]; then
        echo "All jobs are complete."
        break
    fi
    # Wait before checking again
    echo "..."
    sleep 3
done

# Merge output files
echo "Merging output files..."

source /cvmfs/alice.cern.ch/etc/login.sh

if [ "${step}" -eq 5 ]; then   
    /cvmfs/alice.cern.ch/bin/alienv setenv VO_ALICE@ROOT::v6-32-06-alice1-4 -c hadd -f  ${run}/outCorrelations_it${iteration}_step${step}.root ${run}/*/outCorrelations_it${iteration}_step${step}.root
    /cvmfs/alice.cern.ch/bin/alienv setenv VO_ALICE@ROOT::v6-32-06-alice1-4 -c hadd -f /dcache/alice/nkoster/PhD/q-vectors/LHC23zzh_pass4_small/step0/${run}/AO2D${iteration}.root ${run}/*/AO2D${iteration}.root
elif [ "${step}" -eq 0 ]; then  
    cp ${run}/0/Calibration_constants_it${iteration}_step$((${step} + 1)).root ${run}/
    cp ${run}/0/outCorrelations_it${iteration}_step${step}.root ${run}/
    cp ${run}/0/outCorrParams_it${iteration}_step${step}.root ${run}/
    
else
    /cvmfs/alice.cern.ch/bin/alienv setenv VO_ALICE@ROOT::v6-32-06-alice1-4 -c hadd -f ${run}/Calibration_constants_it${iteration}_step$((${step} + 1)).root ${run}/*/Calibration_constants_it${iteration}_step$((${step} + 1)).root
    /cvmfs/alice.cern.ch/bin/alienv setenv VO_ALICE@ROOT::v6-32-06-alice1-4 -c hadd -f ${run}/outCorrelations_it${iteration}_step${step}.root ${run}/*/outCorrelations_it${iteration}_step${step}.root
fi

echo "All jobs have completed and outputs have been merged into $MERGED_OUTPUT."

ls ${run}/0
echo "Removing job-output" 
rm ${run}/*/*.root
ls ${run}/0


# rm ${run}/*/*.root 
