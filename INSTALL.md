Installation guide {#installation}
==================
This page described the installation guide for Anthill on Ubuntu 12.04 (expected to work too - tuning those commands - for your own os/distribution).

1. Third party libraries
2. Third party programs
3. Anthill programs

Third party libraries
---------------------

| Library        | Used version |
|:--------------:|:------------:|
| Qt             | 4.8.1        |
| Boost          | 1.48.0.2     |
| Armadillo      | 3.4.5        |
| InsightToolkit | 4.2.1        |
| QGLViewer      | 2.3.17       |
| DGtal          | 0.5.1        |

### Requirement
You may need to install "cmake".
~~~
  sudo apt-get install cmake
~~~

### 1. Qt
Installing "qtcreator" should install all the required packages (e.g. libqt4-dev, qt4-qmake, libqtcore4, libqtgui4)
~~~
  sudo apt-get install qtcreator
~~~

### 2. Boost
Installing "libboost-all-dev" should install all the required packages (e.g. libboost-program-options-dev, libboost-system-dev, libboost-graph-dev)
~~~
  sudo apt-get install libboost-all-dev
~~~

### 3. Armadillo
Armadillo is a C++ linear algebra library (matrix maths) aiming towards a good balance between speed and ease of use.
The library is open-source software, and is distributed under a license that is useful in both open-source and commercial/proprietary contexts.
You may download the latest version at http://arma.sourceforge.net/download.html, e.g. :
~~~
cd ~/3rdParty
wget http://sourceforge.net/projects/arma/files/armadillo-3.4.5.tar.gz
gzip -d armadillo-3.4.5.tar.gz
tar -xvf armadillo-3.4.5.tar
cd armadillo-3.4.5
./configure
make
sudo make install
~~~

### 4. InsightToolkit
ITK is an open-source software toolkit for performing registration and segmentation.
We **just use it to read DICOM folders**.
You may download the latest version at http://www.itk.org/ITK/resources/software.html, e.g. :
~~~
cd ~/3rdParty
wget http://sourceforge.net/projects/itk/files/itk/4.2/InsightToolkit-4.2.1.tar.gz/download
gzip -d InsightToolkit-4.2.1.tar.gz
tar -xvf InsightToolkit-4.2.1.tar
cd InsightToolkit-4.2.1
mkdir build
cd build
cmake .. -DBUILD_EXAMPLES:string=false -DBUILD_TESTING:string=false -DITK_BUILD_ALL_MODULES:string=false -DITKGroup_Core:string=true -DITKGroup_IO:string=true
sudo make install
~~~
You may tune the ITK installation to **compile more modules of the library**.

### 5. QGLViewer

libQGLViewer is a free C++ library based on Qt that enables the quick creation of OpenGL 3D viewers.
You may download the latest version at http://www.libqglviewer.com/download.html.
On Ubuntu, you may install it using :
~~~
cd ~/3rdParty
wget http://www.libqglviewer.com/src/libQGLViewer-2.3.17.tar.gz
gzip -d libQGLViewer-2.3.17.tar.gz
tar -xvf libQGLViewer-2.3.17.tar
cd libQGLViewer-2.3.17
qmake
make
sudo make install
~~~

### 6. DGtal

The collaborative project DGtal aims at developing generic, efficient and reliable digital geometry data structures, algorithms and tools. 
It takes the form of an open-source C++ library DGtal and a set of tools and binaries DGtalTools.
We mainly used it for the display 3D object and for the Reverse Euclidean Distance Transform.
~~~
cd ~/3rdParty
wget http://libdgtal.org/releases/DGtal-0.5.1-Source.tar.gz
gzip -d DGtal-0.5.1-Source.tar.gz
tar -xvf DGtal-0.5.1-Source.tar
cd DGtal-0.5.1-Source
mkdir build
cd build
cmake .. -DWITH_GMP:string=true -DWITH_ITK:string=true -DWITH_QGLVIEWER:string=true
make
sudo make install
~~~

Third party programs
--------------------

### 1. Euclidean skeletons http://www.esiee.fr/~coupriem/es/
C source code for Euclidean skeletons, both for 2D and 3D binary images.
A version of Linux is needed for an easy installation.
~~~
cd ~/3rdParty
wget http://www.esiee.fr/~coupriem/es/ES_programs.tgz
tar zxvf ES_programs.tgz
cd ES_programs
make
export PINK=`pwd`
export PATH=${PATH}:${PINK}/bin
~~~

### 2. Critical kernels http://www.esiee.fr/~info/ck/
Critical kernels contitute a unifying framework to study and design topology-preserving parallel thinning algorithms.
A version of Linux is needed for an easy installation.
~~~
cd ~/3rdParty
wget http://www.esiee.fr/~info/ck/CK_programs.tgz
tar zxvf CK_programs.tgz
cd CK_programs
make
export PATH=${PATH}:`pwd`/bin
~~~

Anthill programs
----------------
~~~
cd
wget https://github.com/hlocteau/anthill/archive/master.zip
unzip master.zip
cd anthill
cd scripts
for e in `ls *.sh.txt`; do mv "${e}" "${e%.*}"; done
chmod u+x *.sh
export PATH=${PATH}:${HOME}/bin:`pwd`
cd ../tools
mkdir ~/bin
mkdir ../obj
make ~/bin/buildScene ~/bin/innerScene ~/bin/labelingcc ~/bin/minpath ~/bin/segSkelMinDepth ~/bin/catchskel ~/bin/keepInnerSkelOnly ~/bin/rebuild ~/bin/view3dPgm
cd ../gui
qmake -o Makefile_gui
make -f Makefile_gui
~~~
