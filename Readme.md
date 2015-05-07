assimp2gltf
========

#### gltf exporter for Open Asset Import Library ####

Convert files in 40+ 3D file formats, including __Collada, 3DS, OBJ, LWO, FBX, Blender, X, STL, PLY, MS3D, B3D, MD3, MDL, DXF__ and __IFC__ to `gltf`.

### Introduction ###

`assimp2gltf` is a command line tool designed to expose the import capabilities of `assimp`, the [Open Asset Import Library](http://assimp.sourceforge.net) to WebGl developers. The tool takes a single 3d model as input file, imports it using `assimp` and converts the result to `gltf`.

`assimp2gltf` is platform-independent, its only dependency is `assimp` itself.

### Usage ###

``` 
$ assimp2gltf [flags] input_file [output_file] 
```

Invoke `assimp2gltf` with no arguments for detailed information.