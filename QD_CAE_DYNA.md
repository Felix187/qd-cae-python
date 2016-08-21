
# Module qd.cae.dyna

The module contains functions and classes for the FEM-Solver LS-DYNA. This project is not affiliated in any way with the creators or distributors of LS-Dyna and thus is totally unofficial.

The core-code is written entirely in C++ with a python wrapper. Even though the code is being validated with big effort, there always may be mistakes and bugs. The reader is very touchy in terms of checks, so if anything will go wrong during reading, an exception will be thrown.

-----------
# Overview:

[Example](#example)

Classes:
- [D3plot](#d3plot)
- [KeyFile](#keyfile)
- [FEMFile (D3plot & KeyFile)](#femfile(d3plot,keyfile))
- [Node](#node)
- [Element](#element)
- [Part](#part)

Functions:
(None)

[FAQ](#faq)


---------
# Example

```python
from codie import D3plot

d3plot = D3plot("filepath/to/d3plot",read_states="disp")
timesteps = d3plot.get_timesteps()
d3plot.read_states(["plastic_strain max","history 2 shell max"])

node = d3plot.get_nodeByID(7)
node_displacement = node.get_disp()

element = d3plot.get_elementByID("shell",11)
elem_plastic_strain = element.get_plastic_strain()
for node in element.get_nodes():
  print("Node:"+str(node.get_id()))

part = d3plot.get_partByID(13)
part_elems = part.get_elements()

```

---------
# D3plot

This class can read binary result files and give access to it. It has the following limits:

- Single Precision only (in case you use Double Precision, output in Single Precision)
- no SPH or fluid dynamics planned!
- no thick shells

**D3plot(filepath,use_femzip=False,read_states=None)**

*return: instance of the d3plot class.*

Read a d3plot with basic geometry into the memory. The second option is meant to be used for femzipped result files. use_femzip is optional and False by default. The pre-compiled .whl is compiled with femzip. The option read_states works the same as the function d3plot.read_states and is meant to save time to already load state variables on first time loading.

**d3plot.get_timesteps()**

*return: (list of floats) output time*

Get a list of time-steps at which the data was written to the d3plot.

**d3plot.read_states(arg)**

*return: None*

Argument arg may be a string or a list of strings.

Read a variable from the state files. If this is not done, the nodes and elements return empty lists when requesting a result. The variables available are:
- disp = displacement
- vel = velocity
- accel = acceleration
- strain [(optional) mode]
- stress [(optional) mode]
- plastic_strain [(optional) mode]
- history [id1] [id2] ... [shell or solid] [(optional) mode]

Additionally one has to keep in mind that shells contain multiple output layers with results. Therefore additional modes where introduced in order to specify the handling of multiple outputs.
- in/mid/out
- max/min/mean

In order to read the plastic-strain considering only the maximum over all integration layers use: "plastic_strain max". Default is mean, if no mode is given. When reading history variables, one MUST specify type (shell/solid) and at least one index starting at 1.


----------
# KeyFile

**KeyFile(filepath)**

*return instance of KeyFile*

Constructor for a KeyFile, which is a LS-Dyna input file. The parsing is currently limited to mesh data only:

Features:
- Nodes
- Elements
- Parts

-----------------------------
# FEMFile (D3plot, KeyFile)

**femfile.get_filepath()**

*return: (string) filepath*

Get the path of the file.

**femfile.get_nodeByID(arg)**

*return: node or list of nodes*

The a node or a list of nodes, depending on the argument. One can either use just a node id (thus an integer) or a list of ids (thus a list of int). In the second case the function returns a list of nodes.

**femfile.get_elementByID(element_Type,arg)**

*return: element or list of elements*

This function takes two arguments. The first one is the element type. It may be a string "beam", "shell" or "solid". This is necessary due to the reason that ls-dyna is the sole solver which can use the same id for two different element types. The second argument may be either an id or a list of ids.

**femfile.get_partByID(id)**

*return: part*

Get a part instance by it's id.

**femfile.get_parts()**

*return: list of parts*

Get all the parts in the femfile.

-------
# Node

The **Node** class handles all node related data. In case that the **Node** is owned by a D3plot, time series data may be requested too, must be loaded first.

**node.get_id()**

*return: int id.*

Returns the id of the node.

**node.get_coords(int iTimestep = 0)**

*return: (list of floats) 3D-coordinates*

The geometrical coordinates of the node. In a d3plot, coordinates can also be loaded from different timesteps, in which case displacements must be loaded though (see d3plot.read_states). iTimestep may also be negative to access coordinates backwards (e.g. -1 for last timestep), similar to python array syntax.

**node.get_disp()**

*return (list of list of floats) time series of displacement*

The time series of the displacement of the node. The first index is the time state and the second index the space coordinate index.

**node.get_vel()**

*return (list of list of floats) time series of the velocity vector*

Get the time series of the velocity vector.

**node.get_accel()**

*return (list of list of floats) time series of the acceleration vector*

Get the time series of the acceleration vector.

**node.get_elements()**

*return (list of elements) elements of the node.*

Get all element instances, which reference this node.

---------
# Element

The **Element** function works the same as the node function. In case it is owned by a D3plot, it may contain time series data, if loaded.


**element.get_id()**

*return: int id*

Get the element id.

**element.get_plastic_strain()**

*return: (list of floats) time series of plastic strain values*

Get the time series of the elements plastic strain.

**element.get_energy()**

*return: (list of floats) time series of element energy*

Get the element energy.

**element.get_strain()**

*return: (list of list of floats) time series of the strain vector*

This function returns a time series of the strain vector. The vector contains the 6 values of the strain vector [exx,eyy,ezz,exy,eyz,exz].

**element.get_stress()**

*return: (list of list of floats) time series of the stress vector*

This function returns a time series of the stress vector. The vector contains the 6 values of the stress vector [sxx,syy,szz,sxy,syz,sxz].

**element.get_nodes()**

*return: (list of nodes) nodes of the elements*

**element.get_coords(iTimestep=0)**

*return: (list of flaot) Get the coordinates of the element.*

You can get the coordinates of the element, which is the mean of it's nodes coordinates. If the optional flag iTimtestep != 0 then the displacements must be read in the D3plot. One also may use negative numbers like -1 for the last timestep.

**element.get_history()**

*return: (list of list of floats) time series of the history variable vector*

This function returns the time series of the history variables. The first index is the timestep and the second index the loaded variable. The history variables loaded in shells and solids may be different!

**element.get_estimated_size()**

*return: estimated element size*

Calculates an average element edge size for the element. The size is not highly accurate for performance reasons, but is a good indicator for the dimensional size though.

**element.get_type()**

*return: str element_type*

Get the type of the element.

-------
# Part

The **Part** class has the following functions:


**part.get_id()**

*return: (int) get the id of the part*

Get the id of the part.

**part.get_name()**

*return: (string) part get_name*

Get the name of the part, if present.

**part.get_nodes()**

*return: (list) nodes of the part*

Get all nodes, which belong to the part. A node may belong to more than one part, since only elements are uniquely assigned to parts.

**part.get_elements()**

*return: (list) elements of the part*

Get all elements, which belong to the part.


-----
# FAQ

## Wrong Filetype

*RuntimeError: Wrong filetype [NUMBER] != 1 in header of d3plot*

The file might be written in double precision (1), or the files endian is different to the machines endian (2). In case (1) you can simply tell LS-Dyna to output the file in single-precision, despite calculating in double precision. No one usually cares about those digits anyways for numeric reasons. 3 ways to make dyna output in 32bit format:

- Inputfile: *DATABASE_FORMAT, IBINARY=1
- Environment-Variable: export LSTC_BINARY=32ieee
- Command-line: e. g. ls971 i=input 32ieee=yes


## nsrh != nsort + numnp

Your file might be compressed with FEMZIP. Use the flag: use_fezmip=True in the constructor of the D3plot.