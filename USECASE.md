Use case {#usecase}
========
This page describe a use case.
~~~
cd ~/outputData/MeMo0068/serie_3
ls
	binary.pgm3d  input.pgm3d  serie_3.xml

buildScene ./binary.pgm3d
	BoundingBox #1 : [PointVector] {11, 112, 15} [PointVector] {480, 501, 598} volume 23412102
ls
	anthillallcontent.pgm3d  	anthillcontent_v3d.pgm3d  	anthilllabelinner.pgm3d
	anthill.scene.pgm3d  		depthObjHist.txt  		serie_3.xml
	anthillcontent.pgm3d     	anthill.dthull.pgm3d      	anthill.mask.pgm3d
	binary.pgm3d         		input.pgm3d

do3Dskel.sh anthillcontent.pgm3d anthillcontent.dist.pgm3d anthillcontent.skeleucl.pgm3d
ls
	anthillallcontent.pgm3d    	anthillcontent.pgm3d           	anthillcontent_v3d.pgm3d
	anthilllabelinner.pgm3d  	anthill.scene.pgm3d  		depthObjHist.txt  
	serie_3.xml			anthillcontent.dist.pgm3d  	anthillcontent.skeleucl.pgm3d
	anthill.dthull.pgm3d      	anthill.mask.pgm3d       	binary.pgm3d
	input.pgm3d

innerSkelOnly -i . --loop 4
ls
	anthillallcontent.pgm3d    	anthillcontent.pgm3d           	anthillcontent_v3d.pgm3d
	anthill.innerskel.pgm3d  	anthill.mask.pgm3d   		binary.pgm3d
	input.pgm3d			anthillcontent.dist.pgm3d  	anthillcontent.skeleucl.pgm3d
	anthill.dthull.pgm3d      	anthilllabelinner.pgm3d  	anthill.scene.pgm3d
	depthObjHist.txt  		serie_3.xml

labelingcc -i anthill.innerskel.pgm3d -o anthill.innerskel.main.pgm3d -t 1
ls
	anthillallcontent.pgm3d    	anthillcontent.skeleucl.pgm3d  	anthill.innerskel.main.pgm3d
	anthill.mask.pgm3d   		depthObjHist.txt		anthillcontent.dist.pgm3d
	anthillcontent_v3d.pgm3d       	anthill.innerskel.pgm3d       	anthill.scene.pgm3d
	input.pgm3d			anthillcontent.pgm3d		anthill.dthull.pgm3d
	anthilllabelinner.pgm3d		binary.pgm3d			serie_3.xml

innerScene -s anthill.innerskel.main.pgm3d -d anthillcontent.dist.pgm3d -o anthill.inner.main.pgm3d -g 256x256x256
ls
	anthillallcontent.pgm3d		anthillcontent.skeleucl.pgm3d	anthill.inner.main.pgm3d
	anthilllabelinner.pgm3d		binary.pgm3d			serie_3.xml
	anthillcontent.dist.pgm3d	anthillcontent_v3d.pgm3d	anthill.innerskel.main.pgm3d
	anthill.mask.pgm3d		depthObjHist.txt		anthillcontent.pgm3d
	anthill.dthull.pgm3d		anthill.innerskel.pgm3d		anthill.scene.pgm3d
	input.pgm3d

segSkelMinDepth -s anthill.innerskel.main.pgm3d -i anthill.inner.main.pgm3d -d anthillcontent.dist.pgm3d -o seg80_60.pgm3d --high 1 -t 80 -m 0.6
cp /tmp/stepFinal.pgm3d seg80_60.pgm3d
~~~