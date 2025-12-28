chmod -R +x ./*
echo "-------------------- Starting float_test_aarch64-linux-gnu test --------------------"
./float_test_aarch64-linux-gnu.bin

echo "-------------------- Starting float_test_neon_arm-linux-gnueabihf test --------------------"
./float_test_neon_arm-linux-gnueabihf.bin

echo "-------------------- Starting float_test_neon_armv8l-linux-gnueabihf test --------------------"
./float_test_neon_armv8l-linux-gnueabihf.bin

echo "-------------------- Starting float_test_sf_arm-linux-gnueabi test --------------------"
./float_test_sf_arm-linux-gnueabi.bin

echo "-------------------- Starting float_test_vfpv3_arm-linux-gnueabihf test --------------------"
./float_test_vfpv3_arm-linux-gnueabihf.bin

echo "-------------------- Starting float_test_vfpv3_armv8l-linux-gnueabihf test --------------------"
./float_test_vfpv3_armv8l-linux-gnueabihf.bin