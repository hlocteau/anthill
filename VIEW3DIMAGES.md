Viewing 3D images {#view3d}
==================

Two solutions are available to visualize the 3D images issued from the different tools.
The first one is a real 3D viewer - named view3dPgm - while the second one is a 2D viewer - named antHouse_gui - for which user may tune the parameters.

# 1. view3dPgm
## 1. basic usage
Given a file named _filename.pgm3d_, the basic usage is :
~~~
view3dPgm --input filename.pgm3d
~~~
or, using the short option name for __input__ (in the forthcoming commands, we will use the full option names):
~~~
view3dPgm -i filename.pgm3d
~~~
This gives you an interface that just display voxels that get a positive value. The viewer lets you interact with the object using the mouse (further options, owned by the QGLViewer, are available, press the key 'H' to display them).

Lets consider the output file named _test_rag1.pgm3d_ issued from the example test_rag. You get a window like the following one:
![Viewing test_rag1.pgm3d](view3dPgm_vasic "Viewing test_rag1.pgm3d (1)")

This image is not a binary one. Using an hexadecimal viewer, the header of this file is :
~~~
P5
60 60 60
5
~~~
that you should understand as : a gray-level image (because of P5), whose dimensions are 60 x 60 x 60, and whose highest value is 5.
Of course, the highest value is usually not equals to the number of colors. Nevertheless, for the specific case, values are equals and, as the header reads a gray-level image, you may expect to see distincts colors.

## 2. using labels
To distinguish the different labels, you should activate the labels:
~~~
view3dPgm -i test_rag1.pgm3d --label 1
~~~
By default, a single channel is used. To get more discriminative colors:
~~~
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0
~~~
When mono is 0, a predefined gradient is used to convert scalar values in a given range into a color.
~~~
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0
~~~
In the last command line, we use a rgb generator. To use a hsv generator, lets type :
~~~
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --golden 1
~~~
## 3. filtering labels
You may display only voxels having a specific scalar value:
~~~
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --golden 1 --selection 2
~~~
You may enable the selection of several values (possibly using intervals __min__:__max__) :
~~~
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --golden 1 --selection "2 4 5"
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --golden 1 --selection "2 4:5"
~~~

