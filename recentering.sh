run=${1}

for it in 4 5
do
    for step in 0 1 2 3 4 5 
    do
        source run_and_merge.sh ${it} ${step} ${run}
    done
    echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - "
done