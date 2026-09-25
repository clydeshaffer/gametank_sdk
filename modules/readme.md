The modules folder is designed to give an easy place to store libraries made by other GameTank developers separately from your project-specific code and from the base SDK libraries. Modules should be a subfolder inside the modules folder, with their own src directory that will be picked up by the makefile. For example "modules/mycoolplugin/src"


Later, modules folders will also get their own scripts folder for compile-time processing hooks, and their own assets folder.
