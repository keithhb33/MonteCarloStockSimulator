rm -rf build
mkdir build
cp fetch_data.py build/fetch_data.py
cd build
cmake ..
make