if [ ! -e ./.config ];then
  cp oem7628_config .config
fi
cd toolchain
if [ ! -e ./buildroot-gcc342 ];then
  tar -xf buildroot-gcc342.tar.bz2
fi
cd ..
make
cp uboot.bin oem7628_uboot.bin 
 
