# AO2D merging tool. Options: 
#   --input <inputfile.txt>      Contains path to files to be merged. Default: input.txt
#   --output <outputfile.root>   Target output ROOT file. Default: AO2D.root
#   --max-size <size in Bytes>   Target directory size. Default: 100000000. Set to 0 if file is not self-contained.
#   --skip-non-existing-files    Flag to allow skipping of non-existing files in the input list.
#   --skip-parent-files-list     Flag to allow skipping the merging of the parent files list.
#   --compression <root compression id>  Compression algorithm / level to use (default: 505)
#   --verbosity <flag>           Verbosity of output (default: 2).

#!/bin/bash
run=${1}
# Loop through directories and copy files
for dir in /dcache/alice/nkoster/PhD/q-vectors/LHC23zzh_pass4/${run}/AO2D*.root
do
    echo ${dir} >> ${run}.txt
done

o2-aod-merger --input ${run}.txt --output /dcache/alice/nkoster/PhD/q-vectors/LHC23zzh_pass4/${run}/AO2D0.root --max-size 10000000000

root -b -q "openFile.C(\"/dcache/alice/nkoster/PhD/q-vectors/LHC23zzh_pass4/${run}/AO2D0.root\")"