Use case {#usecase}
========
This page describe a use case.

# starting point
Once having importing dicom folder MeMo0013 using the gui, open the project serie_ 2.xml and apply a 
binarization (checkbox Threshold) using for example parameters -600:1200 for the selection range (in the ressource table) of the import,
and a threshold value 11 (using spinbox). Click on "run" button. You should get a new ressource named "binarisation:result".
Now, you may quit the gui and open a terminal:
~~~
cd ~/outputData/MeMo0013/serie_2
ls
	binary.pgm3d  input.pgm3d  serie_3.xml
~~~

# Initializing the scene
We aim to decompose the anthill into a set of rooms and corridors, we have to bound the scene, and fill it.
~~~
buildScene ./binary.pgm3d
	"Nombre de composantes = 465" 
	BoundingBox #1 : [PointVector] {108, 163, 13} [PointVector] {396, 394, 506} volume 7920172
	"Nombre de composantes = 1292" 

~~~
We have created a serie of new ressources located at voxel (108 163 13) in the initial 3D image.
~~~
ls
	binary.pgm3d  content.pgm3d  hull.dt.pgm3d  input.pgm3d  mask.pgm  premask.pgm  serie_2.xml
~~~
All the following ressources are computed in the domain (108, 163, 13) (396, 394, 506).

# Computing the skeleton of the object
## Extracting Euclidean Skeleton
We may extract both the skeleton content.skeleucl.pgm3d, and a distance map content.dist.pgm3d of the object saved in content.pgm3d:
~~~
do3Dskel.sh content.pgm3d content.dt.pgm3d content.skeleucl.pgm3d
ls
	binary.pgm3d  content.dt.pgm3d  content.pgm3d  content.skeleucl.pgm3d  hull.dt.pgm3d  input.pgm3d  
	mask.pgm      premask.pgm       serie_2.xml
~~~

## Filtering the skeleton
Because of the definition of the hull, some branches of the skeleton are meaningless as they correspond to concavities.
~~~
innerSkelOnly -i . --loop 4
	Info : size of the input skeleton's image    232 x 289 x 494
	       size of the distance hull's image     232 x 289 x 494
	       size of the distance skeleton's image 232 x 289 x 494
	1389039 after the first loop
	Info : number of skeleton's voxels
	                    - input  : 1629746
	                    - output : 1389039 after 2 loop(s)

ls
	binary.pgm3d      content.pgm3d           hull.dt.pgm3d     input.pgm3d  premask.pgm  serie_2.xml
	content.dt.pgm3d  content.skeleucl.pgm3d  inner.skel.pgm3d  mask.pgm

~~~
We have filtered out the skeleton and possibly created several disconnected components. We just select the biggest one:
~~~
labelingcc -i inner.skel.pgm3d -o inner.skel.main.pgm3d -t 1
	"Nombre de composantes = 2954"
ls
	binary.pgm3d      content.pgm3d           hull.dt.pgm3d          inner.skel.pgm3d  mask.pgm		serie_2.xml
	content.dt.pgm3d  content.skeleucl.pgm3d  inner.skel.main.pgm3d  input.pgm3d       premask.pgm

~~~
# Defining the scene wrt preprocessing
We can now reconstruct the scene wrt preprocessing.
~~~
innerScene -s inner.skel.main.pgm3d -d content.dt.pgm3d -o inner.main.pgm3d -g 256x256x256
ls
	binary.pgm3d      content.pgm3d           hull.dt.pgm3d          inner.skel.pgm3d  mask.pgm3d   serie_2.xml
	content.dt.pgm3d  content.skeleucl.pgm3d  inner.skel.main.pgm3d	 input.pgm3d       premask.pgm  inner.main.pgm3d
~~~
# Segmenting the scene into rooms and corridors
We first have to define some rooms seeds wrt a minimum distance to the boundary (24).
Voxels not been selected as seeds are regarded as voxels belonging to any corridor.
Some merging operations are applied wrt the threshold value 60%.
~~~
segSkelMinDepth -s inner.skel.main.pgm3d -i inner.main.pgm3d -d content.dt.pgm3d -o seg24_60.pgm3d --high 1 -t 24 -m 0.6
cp /tmp/stepFinal.pgm3d seg24_60.pgm3d
~~~
The output result seg24_ 60.pgm3d can be visualized through the view3dPgm tool or the gui, adding the ressource to the project file.
