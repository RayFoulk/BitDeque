# BitDeque
A general purpose deque of bits

# License
MIT

## Purpose
This is a general utility class for use in other larger projects.  Essentially it exchanges memory usage (high) for fine-grained run-time control over bitfields in arbitrary data structures.  This could be used for cryptography, compression, serial communication, assembers/disassembers, etc.  Bit fields can be software-defined and parameterized, allowing the user to write flexible yet fast code that is compiled once, rather than what is typically done where data structures are written and defined to match a particular hardware case.

## (Ambitious) Use Case Examples
- A serial communication library that can communicate with a wide range of UUT types, where each type has a parameterized ICD.
- An arbitrary precision integer or floating point math library
- A universal assembler, with each machine type's opcodes defined in a human-readable parameter file.

## TODO
Add built-in class serialization / deserialization to/from JSON (Not XML!)

## Q&A
- Why is your code wrapped at 52 characters?
    - Because I developed this on my tiny mobile phone (yes, really).  Normally at my desktop I wrap lines at ~100 characters.
- Why do you dislike XML?
    - I actually do like some aspects of XML (portability, flexibility, ease of use, semi-human readable)
    - However it has some drawbacks: information density is low by comparison, and it's not _that_ human-readable
    
