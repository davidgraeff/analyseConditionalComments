Analyse Conditional Comments (#ifdefs)
======================================
Heavy usage of conditional comments in c/c++ software often implies a difficult
to read source code and raises the maintanence burden. That is bound to happen sooner or later.
The induction process for new programmers is accompanied by a high learning curve, due to
mostly no available documentation for all the used C-Macros and conditional comments. 
Introducing a new feature may cause another conditional comment all over the codebase.

This software will give you a quantitative graphical overview of all used conditional comments,
their spreading and a first understanding of how much code is affected by each one of the #if/#ifdef uses.
The input is your codebase. Options for an output are vector based image types like svg and pdf,
but also the raster based image type png.

You may use this software for scientific reseaches or just for understanding a codebase.

Dependencies:
=============
* Qt4 for the gui
* librsvg for svg->pdf,png conversion.

Compile/Download:
=================
You may download a precompiled version for windows and linux.
If you want to compile the software yourself, install cmake (www.cmake.org), run it to generate
Makefiles, VisualStudio project files or whatever and compile it.

How it works:
=============
This software statically analysis given files by explicitly looking for #if/#ifdef/#else/#endif
and other constructs. No parse tree or abstract syntax tree is generated or necessary for this
task. Nested conditional comments are supported, #ifdef-guards are recogniced and ignored.
The parser is multi-thread capable and depending on your cpu cores even a large code base is
analysed very fast.

License:
========
Not decided yet. Probably dual-licensed as GPLv3 and Apache License, Version 2.0.
