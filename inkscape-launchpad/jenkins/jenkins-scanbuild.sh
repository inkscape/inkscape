./autogen.sh
./configure
make clean

# do not exit immediately if any command fails
set +e
     
# directory to use for storing the scan-build report, this is where the Publish Clang Scan-Build Results plugin checks
SCAN_BUILD_OUTPUTDIR="${WORKSPACE}/clangScanBuildReports"

# generate the scan-build report
scan-build -k -o ${SCAN_BUILD_OUTPUTDIR} make
     
