Standalone

This is part of the “Projekt SMPS.”

The project is developed using PlatformIO as the framework.

The standalone folder is specifically intended for testing individual components and ongoing development.

There will also be a main PlatformIO project folder. This folder will contain the fully integrated code for the final solution. Each individual component will be placed in its own file, including the functions and behavior required to handle that element—either independently, in coordination with other modules, or within the main program.

A key design decision is whether modules should be implemented as individual classes (allowing state to be managed and modified) or as function-based implementations without maintaining state.

# Working with Hardware 🛠️
To work with hardware e.g schematics and pcb designs you need git LFS and Kicad

### git lfs 
To install git lfs run
```
git lfs install
```
**This must be run** it prevents committing unlocked files which can cause accidental overwrites
```
git config lfs.locksverify true
```

### Kicad
Install via website
## Daily workflow
#### 1. Before editing
Before editing a PCB or schematic run:
```
git lfs locks
```
This allows you to see the files that are locked. \
If the files are unlocked, lock them.\
An example:
```
git lfs lock hardware/buck-converter/buck-converter.sch
```
#### 2. Working
Open the files, edit and commit.
Commit example
```
git add hardware/my_project.kicad_pcb
git commit -m "Route motor driver power stage"
git push
```

#### 3. IMPORTANT - unlock when finished🔓
Unlock when finished editing and commiting, so that others can work on the files
Example:
```
git lfs unlock hardware/buck-converter/buck-converter.sch
```
You can also use the "ID" the lock has from the git lfs locks output.
