#!/bin/bash
source /cvmfs/alice.cern.ch/etc/login.sh

# Set up the environment using alienv
eval $(alienv printenv VO_ALICE@ROOT::v6-32-06-alice1-4)

# Define run number
part=${1}
iteration=$((${2} - 1))
run=${3}
outdir=/data/alice/nkoster/alice/Analysis/ZDC/runCent/postProcess/${run}/${part}

aod_file=/dcache/alice/nkoster/PhD/q-vectors/LHC23zzh_pass4_small/step0/${run}/AO2D${iteration}.root

cd ${TMPDIR}
echo ${TMPDIR}

export INPUT_FILES_DIR=/data/alice/nkoster/alice/Analysis/ZDC/runCent/postProcess

if [ ! -d ${outdir} ]
    then
  mkdir -p ${outdir}
fi

cp ${INPUT_FILES_DIR}/recentering.C ${TMPDIR}

# First iteration
root -b -q "recentering.C(\"${aod_file}\", ${run}, 100, ${part})"
echo "First step complete!" 

pwd
ls 

echo ${outdir}

mv *.root ${outdir}

ls
