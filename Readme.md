assimp2gltf
========

#### JSON exporter for Open Asset Import Library ####

Convert files in 40+ 3D file formats, including __Collada, 3DS, OBJ, LWO, FBX, Blender, X, STL, PLY, MS3D, B3D, MD3, MDL, DXF__ and __IFC__ to plain `json`.

[__Convert models online here__.](http://www.greentoken.de/onlineconv/) (Select JSON as output format)

[__Download Windows binaries here__.](https://github.com/acgessler/assimp2gltf/releases) (_v2.0, October 2013_)

![quak](http://s21.postimg.org/nu1bfiobr/dwarf22.png)

### Introduction ###

`assimp2gltf` is a command line tool designed to expose the import capabilities of `assimp`, the [Open Asset Import Library](http://assimp.sourceforge.net) to WebGl developers. The tool takes a single 3d model as input file, imports it using `assimp` and converts the result to `json`.

`assimp2gltf` is platform-independent, its only dependency is `assimp` itself.

### Output Format ###

The output format is a one-by-one translation of [Assimp's C datastructure](http://assimp.sourceforge.net/lib_html/structai_scene.html) , with a few changes to make the resulting `json` look more natural. All fields are lower-case and the prefixes (such as m, pc, ..) are omitted. Array lengths are not written as this information is implicitly given. Empty arrays are not written at all, i.e. a node without children doesn't have an empty `children:[]` field.

The `/samples` folder contains some sample `assimp.json` files.

`assimp.json` files carry a little format header in the JSON root element. This allows them to be disambiguated from other JSON-based 3D formats (such as Three.JS scenes). The numeric value specifies the assimp2gltf format version in (major, minor, revision) format. Currently, only 100 is defined.

     "__metadata__": {
	 	 "format" : "assimp2gltf"
	 	,"version": 100
	 }

### Build ###

The build system for assimp2gltf is CMake. To build, use either the CMake GUI or the CMake command line utility. __Note__: make sure you pulled the `assimp` submodule, i.e. with `git submodule init && git submodule update`

### Usage ###

``` 
$ assimp2gltf [flags] input_file [output_file] 
```

(omit the `output_file` argument to get the `json` string on stdout)

Invoke `assimp2gltf` with no arguments for detailed information.

### Future Plans ###

I plan to build a proper content pipeline for WebGl assets based on assimp, assimp2gltf, possibly OpenCTM, WebP and Crunch/DXT to be able to preprocess 3D assets and their textures for fast streaming. Stay tuned, or drop me a note if you want to help out.








