README

These files together makeup a compiler used for compiling a dimished java language, 
called DJ, into a diminished machine language, called dism.

This compiler performs all the major steps of compilation from lexical analysis 
to code generation but does not include code optimizations and garbage collecion.

This compiler was a semester long project for my compilers class, eveyr file but 
symtbl.h and symtbl.o were written by us students. This compiler uses flex and 
bison for the front end of compilation and C directly for codegeneration.

This compiler was implemented at level which assumes that the source code does not include
classes and only a main block of code. 


ABOUT DJ
DJ is a turing complete language designed by our professor that is counted as an 
OOP languge. DJ includes classes and natural numbers as a primitive type. To see 
the entire DJ language please email me at ahrnndz360@gmail.com

