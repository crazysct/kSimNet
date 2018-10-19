#environment: Ubuntu 16.04LTS, 18.04LTS

#install ns-3 essentials
#default
sudo apt-get install -y gcc g++ python python-dev python-setuptools
#visualization
sudo apt-get install -y qt5-default python-pygraphviz python-kiwi python-pygoocanvas libgoocanvas-dev ipython
#debug
sudo apt-get install -y gdb valgrind 

#configure ofswitch13 module
sudo apt-get install -y build-essential gcc g++ python git mercurial unzip cmake
sudo apt-get install -y libpcap-dev libxerces-c-dev libpcre3-dev flex bison
sudo apt-get install -y pkg-config autoconf libtool libboost-dev


git clone https://github.com/netgroup-polito/netbee.git
cd netbee/src/
cmake .
make

cd ../
SCRIPTPATH=$(cd "$(dirname "$0")" && pwd)

echo $SCRIPTPATH

sudo cp $SCRIPTPATH/bin/libn*.so /usr/local/lib
sudo ldconfig
sudo cp -R $SCRIPTPATH/include/* /usr/include/
cd ../

SCRIPTPATH=$(cd "$(dirname "$0")" && pwd)
echo $SCRIPTPATH

cd $SCRIPTPATH/src/ofswitch13/lib/ofsoftswitch13
#exec bash
./boot.sh
./configure --enable-ns3-lib
make

cd ../../../../

./waf configure
./waf configure --with-ofswitch13=src/ofswitch13/lib/ofsoftswitch13/
./waf

#sudo ./waf configure --with-ofswitch13=lib/ofsoftswitch13/
#sudo ,/waf
