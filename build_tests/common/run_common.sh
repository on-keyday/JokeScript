. ../setting/common_setting.sh
if [ -e built/src/$RUNEXECUTABLE ]; then
  cd built/src
  ./$RUNEXECUTABLE %RUNCMDLINE%
  cd ../..
fi
