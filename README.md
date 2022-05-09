# AdvanceMAME RPI Buildroot Package
  
Buildroot package for building AdvanceMAME with optimization flags for the Raspberry PI, with VideoCore and Framebuffer acceleration.
  
Copy the "package" folder to your buildroot directory and add the following line to [buildroot]/package/Config.in:  
```
source "package/advancemame/Config.in"
```
  
Tested using buildroot 2022.02.1 and raspberrypi4_defconfig. May also work with previous buildroot versions  
  
Make changes on package/advancemame/advancemame.mk to fit your needs, if you want to add SDL support, ncurses, etc.
