Standalone

This is part of the “Projekt Varierende PSU.”

The project is developed using PlatformIO as the framework.

The standalone folder is specifically intended for testing individual components and ongoing development.

There will also be a main PlatformIO project folder. This folder will contain the fully integrated code for the final solution. Each individual component will be placed in its own file, including the functions and behavior required to handle that element—either independently, in coordination with other modules, or within the main program.

A key design decision is whether modules should be implemented as individual classes (allowing state to be managed and modified) or as function-based implementations without maintaining state.