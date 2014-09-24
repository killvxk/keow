@ECHO OFF
@REM Batch file to extract bootstrap tgz with keow to preferve unix symlinks and permissions etc
ECHO "Extracting linuz.tgz ..."
KeowKernel root=. init=busybox tar xzf linux.tgz
ECHO "Done."
PAUSE
