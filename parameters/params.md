# Parameter System

## Parameter definitions

Parameters are defined in a text file from which the code of the parameter
system is generated. A line in a parameter definition file can be either

    a) Blank
    b) A comment starting with '#'
    c) A parameter definition

A parameter definition is made of the parameter name, the parameter type
and a default value. Valid datatypes are:

    - u32 - 32 bit unsigned integer
    - i32 - 32 bit signed integer
    - f32 - single precision float

More may be added in the future.
