#!/bin/bash

iteration=$1
step=$2
run=$3
finebins=${4}

SUB_FILE=testjob1.sub
if [ "${step}" -eq 0 ]; then  
    SUB_FILE=job_step0.sub
fi

# Submit the job array and capture the JOB_ID
JOB_ID=$(condor_submit "$SUB_FILE" -append "PARAM1=${iteration}" -append "PARAM2=${run}" -append "PARAM3=${finebins}" | grep -oP '(?<=submitted to cluster )\d+')
echo "Submitted job ID $JOB_ID"

# Total number of jobs submitted
TOTAL_JOBS=301

# Wait for all jobs with the given JOB_ID to complete
echo "Waiting for $TOTAL_JOBS jobs in cluster $JOB_ID to complete..."
while true; do
    # Count the jobs in the queue matching the JOB_ID
    JOBS_REMAINING=$(condor_q "$JOB_ID" | grep "$JOB_ID" | wc -l)

    if [ "$JOBS_REMAINING" -eq 0 ]; then
        echo "All jobs in cluster $JOB_ID are complete."
        break
    fi

    echo "Jobs remaining in cluster $JOB_ID..."
    sleep 3
done

# Merge output files
echo "Merging output files..."

source /cvmfs/alice.cern.ch/etc/login.sh

if [ "${step}" -eq 5 ]; then   
    /cvmfs/alice.cern.ch/bin/alienv setenv VO_ALICE@ROOT::v6-32-06-alice1-4 -c hadd -f  ${run}/outCorrelations_it${iteration}_step${step}.root ${run}/*/outCorrelations_it${iteration}_step${step}.root
    /cvmfs/alice.cern.ch/bin/alienv setenv VO_ALICE@ROOT::v6-32-06-alice1-4 -c hadd -f /dcache/alice/nkoster/PhD/q-vectors/LHC23zzh_pass4/${run}/AO2D${iteration}.root ${run}/*/AO2D${iteration}.root
elif [ "${step}" -eq 0 ]; then  
    cp ${run}/0/Calibration_constants_it${iteration}_step$((${step} + 1)).root ${run}/
    cp ${run}/0/outCorrelations_it${iteration}_step${step}.root ${run}/
    cp ${run}/0/outCorrParams_it${iteration}_step${step}.root ${run}/
    
else
    # /cvmfs/alice.cern.ch/bin/alienv setenv VO_ALICE@ROOT::v6-32-06-alice1-4 -c hadd -f ${run}/Calibration_constants_it${iteration}_step2.root ${run}/*/Calibration_constants_it${iteration}_step2.root
    # /cvmfs/alice.cern.ch/bin/alienv setenv VO_ALICE@ROOT::v6-32-06-alice1-4 -c hadd -f ${run}/Calibration_constants_it${iteration}_step3.root ${run}/*/Calibration_constants_it${iteration}_step3.root
    # /cvmfs/alice.cern.ch/bin/alienv setenv VO_ALICE@ROOT::v6-32-06-alice1-4 -c hadd -f ${run}/Calibration_constants_it${iteration}_step4.root ${run}/*/Calibration_constants_it${iteration}_step4.root
    # /cvmfs/alice.cern.ch/bin/alienv setenv VO_ALICE@ROOT::v6-32-06-alice1-4 -c hadd -f ${run}/Calibration_constants_it${iteration}_step5.root ${run}/*/Calibration_constants_it${iteration}_step5.root
    /cvmfs/alice.cern.ch/bin/alienv setenv VO_ALICE@ROOT::v6-32-06-alice1-4 -c hadd -f ${run}/Calibration_constants_it${iteration}_step$((${step} + 1)).root ${run}/*/Calibration_constants_it${iteration}_step$((${step} + 1)).root
    /cvmfs/alice.cern.ch/bin/alienv setenv VO_ALICE@ROOT::v6-32-06-alice1-4 -c hadd -f ${run}/outCorrelations_it${iteration}_step${step}.root ${run}/*/outCorrelations_it${iteration}_step${step}.root
fi

echo "All jobs have completed and outputs have been merged into $MERGED_OUTPUT."


ls ${run}/0
echo "remove unmerged output" 
rm ${run}/*/*.root
echo "Sould be nothing left! : " 
ls ${run}/0
echo " --------> BYE <-------- "
echo "" 