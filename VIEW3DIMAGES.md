Viewing 3D images {#view3d}
==================
Two solutions are available to visualize the 3D images issued from the different tools.
The first one is a real 3D viewer - named view3dPgm - while the second one is a 2D viewer - named antHouse_gui - for which user may tune the parameters.

# 1. Viewing a 3D image using the view3dPgm tool
## 1. Invoking (basic usage)
Given a file named _filename.pgm3d_, the basic usage is :
~~~
view3dPgm --input filename.pgm3d
~~~
or, using the short option name for __input__ (in the forthcoming commands, we will use the full option names):
~~~
view3dPgm -i filename.pgm3d
~~~
This gives you an interface that just display voxels that get a positive value. The viewer lets you interact with the object using the mouse (further options, owned by the QGLViewer, are available, press the key 'H' to display them).

Lets consider the output file named _test_rag1.pgm3d_ issued from the example [test_rag](@ref test_rag.cpp). You get a window like the following one:
![Viewing test_rag1.pgm3d](view3dPgmbasic.png "Viewing test_rag1.pgm3d")

This image is not a binary one. Using an hexadecimal viewer, the header of this file is :
~~~
P5
60 60 60
5
~~~
that you should understand as : a gray-level image (because of P5), whose dimensions are 60 x 60 x 60, and whose highest value is 5.
Of course, the highest value is usually not equals to the number of colors. Nevertheless, for the specific case, values are equals and, as the header reads a gray-level image, you may expect to see distincts colors.

## 2. Displaying labels
To distinguish the different labels, you should activate the labels:
~~~
view3dPgm -i test_rag1.pgm3d --label 1
~~~
By default, a single channel is used. To get more discriminative colors:
~~~
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0
~~~
When __mono__ is 0, a predefined gradient is used to convert scalar values in a given range into a color.
~~~
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0
~~~
In the last command line, we use a rgb generator. To use a hsv generator, lets type :
~~~
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --golden 1
~~~

![Viewing test_rag1.pgm3d using colors](view3dPgmlabel.png "Viewing test_rag1.pgm3d")
![Viewing test_rag1.pgm3d using a gradient](view3dPgmlabelcolor.png "Viewing test_rag1.pgm3d")
![Viewing test_rag1.pgm3d using the rgb generator for colors](view3dPgmlabelcolornogradient.png "Viewing test_rag1.pgm3d")
![Viewing test_rag1.pgm3d using the hsv generator for colors](view3dPgmlabelcolornomaphsv.png "Viewing test_rag1.pgm3d")

## 3. Filtering labels
You may display only voxels having a specific scalar value:
~~~
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --golden 1 --selection 2
~~~
You may enable the selection of several values (possibly using intervals as __min__ : __max__) :
~~~
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --golden 1 --selection "2 4 5"
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --golden 1 --selection "2 4:5"
~~~
![Viewing test_rag1.pgm3d using a specific value for selection](view3dPgmSelection1.png "Viewing test_rag1.pgm3d using selection")
![Viewing test_rag1.pgm3d using a specific values and/or intervals for selection](view3dPgmSelection2.png "Viewing test_rag1.pgm3d using selection")
To easier visualizing a filtered scene (and comparing different selection schemes), the color associated to a given scalar value is preserved, no matter the selected values.
As example, for the given file, the color of voxels labelled as 4 remains constant while displaying all labels or selecting only "2 4 5".
If you want to only use 3 distinct colors wrt the previous selection scheme, lets invoke view3dPgm with option __preserve=0__.
![Viewing test_rag1.pgm3d using a specific values and/or intervals for selection without preserving initial labels](view3dPgmSelection3.png "Viewing test_rag1.pgm3d using selection without preserving initial labels")

## 4. Cropping the scene
You may display only a subpart of the image using options __xmin Xmax ymin Ymax zmin Zmax__, the number of explicit bounds being free :
~~~
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --xmin 30
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --xmin 30 --Ymax 30
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --xmin 30 --Xmax 31
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --xmin 30 --Xmax 31 --boundary 0
~~~
For the latest command line, we introduce the option __boundary__. By default, the viewer just display boundaries' voxels. To change this preference, lets set boundary as 0.
![Viewing a subpart of test_rag1.pgm3d using a partial crop on x](view3dPgmCrop1.png "Viewing a subpart of test_rag1.pgm3d using a partial crop on x")
![Viewing a subpart of test_rag1.pgm3d using a partial crop both on x and y](view3dPgmCrop2.png "Viewing a subpart of test_rag1.pgm3d partial crop both on x and y")
![Viewing a subpart of test_rag1.pgm3d using a full crop on x](view3dPgmCrop3.png "Viewing a subpart of test_rag1.pgm3d using a full crop on x")
![Viewing a subpart of test_rag1.pgm3d using a full crop on x, displaying the content](view3dPgmCrop4.png "Viewing a subpart of test_rag1.pgm3d using a full crop on x, displaying the content")

As an alternative, you may also use clipping providing the parameters of a/severals plane(s) :
~~~
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --clipping "0 -1 0 8" --boundary=0
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --clipping "0 1 0 -5" --boundary=0
view3dPgm -i test_rag1.pgm3d --label 1 --mono 0 --colormap 0 --clipping "0 -1 0 8 0 1 0 -5" --boundary=0
~~~
![Initial viewpoint](view3dPgmNoClipping.png "Initial viewpoint")
![Applying a clipping A](view3dPgmClipping1.png "Applying a clipping A")
![Applying a clipping B](view3dPgmClipping2.png "Applying a clipping B")
![Applying both clippings A and B](view3dPgmClipping3.png "Applying both clippings A and B")

# 2. Viewing a 3D image using the antHouse_gui
The gui use a xml file to define a project. You can find below the main structure of the xml file:
~~~{.xml}
<?xml version="1.0" encoding="UTF-8"?>
<AntHillProject>
    <Dicom folder="/path/to/the/dicom/folder" serie-uid="1.3.12.2.1107.5.1.4.50115.30000006061006032379600001426"/>
    <DicomMetaData>
        <Info name="0008|0005" value="ISO_IR 100"/>
        ...
    </DicomMetaData>
    <Process>
        <Algorithm name="binarization">
            <param name="maximum" value="1200"/>
            <param name="minimum" value="-600"/>
            <param name="opgm" value="binary.pgm3d;bilevel;8u;512 512 531"/>
            <param name="threshold" value="11"/>
        </Algorithm>
        <Algorithm name="import">
            <param name="opgm" value="input.pgm3d;feature;16s;512 512 531"/>
        </Algorithm>
        <Algorithm name="buildscene">
            <param name="opgm" value="content.pgm3d;bilevel;8u;289 232 494;108 163 13"/>
        </Algorithm>
    </Process>
</AntHillProject>
~~~
Importing a dicom folder's content leads to the creation of a xml file that contains :
  - a reference to the input dicom folder (tag __Dicom__) and a dictionary (tag __DicomMetaData__);
  - a list of available ressouces of size 1.
The distinct ressources are issued from the processing methods. From scratch, a single process called _import_ is enumerated (tag **Algorithm** as a child of **Process**).
A single 3D pgm file named _input.pgm3d_ defines the displayable ressource (__o__ uput __pgm__ file or __opgm__ as an abbreviation) and may be identified as the _main ressource_.
For a displayable ressource, we get a serie of informations gathered in the __value__ field : the filename, its meaning, the voxels' type, the image's dimensions.
Whenever a displayable ressource's dimension is smaller than the one of the main ressource, e.g. the ressource attached to the _buildscene_ process, an additional offset is specified.

To keep track of the parameters' values used during a process call, we recommend to save them in the project file (e.g. the _binarization_ process).
If a single process generates _n_ displayable ressources, _n_ __opgm__ lines have to be inserted.
In fact, __opgm__ is a prefix. You may create __opgm1__, __opgm2__, ... lines, appending any string to this prefix (using numbers, letters).

Syntax of the __value__ field of a __opgm__ :
~~~
<filename> ; <bilevel|feature|label> ; <8u|8s|16u|16s|32u|32s> ; <rows> <cols> <slices> ; <offset row> <offset col> <offset slice>
~~~

Some comments :
  - _bilevel_ will be used for binary images (throught the gui, user may change the associated color);
  - _feature_ will be used for grayscale images (user can not change the associated color) when it makes no sence to use colors;
  - _label_ will be used for grayscale images (user can not change the associated color) when we explicitly want to use colors to distinguish scalar values;
  - the voxels' size is an information that can be retrieved from the pgm3d file. Nevertheless, the flag signed/unsigned has to be recorded.
