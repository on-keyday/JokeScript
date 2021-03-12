. ../setting/common_setting.sh
mkdir pre_src
cp -p -r $PROJECT_BASE_PLUS_WILD_CARD ./pre_src
rm -r src/
mv pre_src/ src/
