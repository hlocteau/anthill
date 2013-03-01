Use case {#usecase}
========
This page describe a use case.

# starting point
Once having importing dicom folder MeMo0068 using the gui, open the project serie_ 3.xml and apply a 
binarization (checkbox Threshold) using for example parameters -600:1200 for the selection range (in the ressource table) of the import,
and a threshold value 11 (using spinbox). Click on "run" button. You should get a new ressource named "binarisation:result".
Now, you may quit the gui and open a terminal:
~~~
cd ~/outputData/MeMo0068/serie_3
ls
	binary.pgm3d  input.pgm3d  serie_3.xml
~~~

# Initializing the scene
We aim to decompose the anthill into a set of rooms and corridors, we have to bound the scene, and fill it.
~~~
buildScene ./binary.pgm3d
	BoundingBox #1 : [PointVector] {108, 163, 13} [PointVector] {396, 394, 506}
~~~
We have created a serie of new ressources located at voxel (108 163 13) in the initial 3D image.
~~~
ls
	anthillallcontent.pgm3d  	anthillcontent_v3d.pgm3d  	anthilllabelinner.pgm3d
	anthill.scene.pgm3d  		depthObjHist.txt  		serie_3.xml
	anthillcontent.pgm3d     	anthill.dthull.pgm3d      	anthill.mask.pgm3d
	binary.pgm3d         		input.pgm3d
~~~
All the following ressources are computed in the domain (108, 163, 13) (396, 394, 506).

# Computing the skeleton of the object
## Extracting Euclidean Skeleton
We may extract both the skeleton anthillcontent.skeleucl.pgm3d, and a distance map anthillcontent.dist.pgm3d of the object saved in anthillcontent.pgm3d:
~~~
do3Dskel.sh anthillcontent.pgm3d anthillcontent.dist.pgm3d anthillcontent.skeleucl.pgm3d
ls
	anthillallcontent.pgm3d    	anthillcontent.pgm3d           	anthillcontent_v3d.pgm3d
	anthilllabelinner.pgm3d  	anthill.scene.pgm3d  		depthObjHist.txt  
	serie_3.xml			anthillcontent.dist.pgm3d  	anthillcontent.skeleucl.pgm3d
	anthill.dthull.pgm3d      	anthill.mask.pgm3d       	binary.pgm3d
	input.pgm3d
~~~

## Filtering the skeleton
Because of the definition of the hull, some branches of the skeleton are meaningless as they correspond to concavities.
~~~
innerSkelOnly -i . --loop 4
ls
	anthillallcontent.pgm3d    	anthillcontent.pgm3d           	anthillcontent_v3d.pgm3d
	anthill.innerskel.pgm3d  	anthill.mask.pgm3d   		binary.pgm3d
	input.pgm3d			anthillcontent.dist.pgm3d  	anthillcontent.skeleucl.pgm3d
	anthill.dthull.pgm3d      	anthilllabelinner.pgm3d  	anthill.scene.pgm3d
	depthObjHist.txt  		serie_3.xml
~~~
We have filtered out the skeleton and possibly created several disconnected components. We just select the biggest one:
~~~
labelingcc -i anthill.innerskel.pgm3d -o anthill.innerskel.main.pgm3d -t 1
ls
	anthillallcontent.pgm3d    	anthillcontent.skeleucl.pgm3d  	anthill.innerskel.main.pgm3d
	anthill.mask.pgm3d   		depthObjHist.txt		anthillcontent.dist.pgm3d
	anthillcontent_v3d.pgm3d       	anthill.innerskel.pgm3d       	anthill.scene.pgm3d
	input.pgm3d			anthillcontent.pgm3d		anthill.dthull.pgm3d
	anthilllabelinner.pgm3d		binary.pgm3d			serie_3.xml
~~~
# Defining the scene wrt preprocessing
We can now reconstruct the scene wrt preprocessing.
~~~
innerScene -s anthill.innerskel.main.pgm3d -d anthillcontent.dist.pgm3d -o anthill.inner.main.pgm3d -g 256x256x256
ls
	anthillallcontent.pgm3d		anthillcontent.skeleucl.pgm3d	anthill.inner.main.pgm3d
	anthilllabelinner.pgm3d		binary.pgm3d			serie_3.xml
	anthillcontent.dist.pgm3d	anthillcontent_v3d.pgm3d	anthill.innerskel.main.pgm3d
	anthill.mask.pgm3d		depthObjHist.txt		anthillcontent.pgm3d
	anthill.dthull.pgm3d		anthill.innerskel.pgm3d		anthill.scene.pgm3d
	input.pgm3d
~~~
# Segmenting the scene into rooms and corridors
We first have to define some rooms seeds wrt a minimum distance to the boundary (24).
Voxels not been selected as seeds are regarded as voxels belonging to any corridor.
Some merging operations are applied wrt the threshold value 60%.
~~~
segSkelMinDepth -s anthill.innerskel.main.pgm3d -i anthill.inner.main.pgm3d -d anthillcontent.dist.pgm3d -o seg24_60.pgm3d --high 1 -t 24 -m 0.6
cp /tmp/stepFinal.pgm3d seg24_60.pgm3d
~~~
The output result seg24_ 60.pgm3d can be visualized through the view3dPgm tool or the gui, adding the ressource to the project file.
