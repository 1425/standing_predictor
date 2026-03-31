# standing_predictor

Dependencies:
-sqlite3
-libcurl
-libgumbo (if you want to run the meta-analysis)
-libboost

Build instructions:
1) Download this project and https://github.com/1425/tba so that the two directories are in the same parent directory.
2) Build the "tba" library to talk to The Blue Alliance
3) Get an access key to talk to The Blue Alliance; this is available on their website and is pretty painless.
4) Check that your access key is working with the “demo” program.
5) Go to the standing_predictor directory and run “make”
6) Run the resulting executable “outline”

If you are on Ubuntu 25.10 you might do something like:

apt-get install -y g++ git make cmake wget
apt-get install -y libsqlite3-dev libcurl4-openssl-dev libboost-dev
apt-get install -y libgumbo-dev libtbb-dev

wget https://github.com/simdjson/simdjson/archive/refs/tags/v4.4.2.tar.gz
tar xzf v4.4.2.tar.gz
cd simdjson-4.4.2
cmake .
make install
cd ..

git clone https://1425@github.com/1425/tba.git
git clone https://1425@github.com/1425/standing_predictor.git
git clone https://1425@github.com/1425/frc_api.git
cd standing_predictor
make -j$(nproc)
./outline



