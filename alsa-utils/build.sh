wget https://www.alsa-project.org/files/pub/lib/alsa-lib-1.2.9.tar.bz2

tar -xvf alsa-lib-1.2.9.tar.bz2

cd alsa-lib-1.2.9/

mkdir -p target

./configure --prefix=/root/alsa-lib-1.2.9/target/ --enable-static --disable-shared  CFLAGS="--static" LDFLAGS="--static"  CPPFLAGS="--static"

make -j24

make install

tar -cvf  alsa-lib-1.2.9.tar target/*

/********************************************************************************************/

wget https://www.alsa-project.org/files/pub/utils/alsa-utils-1.2.9.tar.bz2

tar -xvf alsa-utils-1.2.9.tar.bz2

cd alsa-utils-1.2.9

mkdir -p target

apt-get install libgpm-dev gettext  libncurses5-dev libncursesw5-dev

export LIBS="-lformw -lncursesw -ltinfo -lgpm -lasound -lm -ldl -lpthread"

./configure --prefix=/root/alsa-utils-1.2.9/target/ --enable-static  --disable-shared CFLAGS="--static" LDFLAGS="--static -L/root/alsa-lib-1.2.9/target/lib -L/usr/lib/aarch64-linux-gnu/ -lgpm "  CPPFLAGS="--static -I/root/alsa-lib-1.2.9/target/include/" PKG_CONFIG_PATH=/root/alsa-lib-1.2.9/target/lib/pkgconfig/:/usr/lib/aarch64-linux-gnu/pkgconfig/ --includedir=/root/alsa-lib-1.2.9/target/include/

make -j24

make install-stri

tar -cvf  alsa-utils-1.2.9.tar target/*

