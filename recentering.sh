run=${1}

for it in 2
do
    for step in 2 3 4 5
    do
        source run_and_merge.sh ${it} ${step} ${run} 50
    done
    echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - "
done

for it in 3 
do
    for step in 0 1 2 3 4 5
    do
        source run_and_merge.sh ${it} ${step} ${run} 50
    done
    echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - "
done

for it in 4 5 
do
    for step in 0 1 2 3 4 5
    do
        source run_and_merge.sh ${it} ${step} ${run} 100
    done
    echo " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - "
done


source run_and_merge.sh 6 0 ${run} 100