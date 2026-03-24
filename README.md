<img src="README_title.png" alt="drawing" width="500"/>

Explode your ball to reshape the course or get a boost in mid-air.

## Feedback welcome!

Game is work in progress. It would be much appreciated if you gave it a go and
left your feedback on the
[Issues](https://github.com/frankkormann/bomb-golf/issues) page!

## Installing

Go to the [Release](https://github.com/frankkormann/bomb-golf/releases) tab and
follow the instructions for the latest release. You will need to
[hack your 3DS](https://3ds.hacks.guide/) to run on real hardware.

## Building

`make 3dsx` or `make cia`

Note: You should recompile all source files in between building a `3dsx` and
a `cia`. Some files compile differently depending on the final build type.

### Dependencies

* [devkitPro](https://devkitpro.org/) for the 3DS
* [citro2d](https://github.com/devkitPro/citro2d/) and
  [citro3d](https://github.com/devkitPro/citro3d/)

To compile as `cia`:

* [makerom](https://github.com/3DSGuy/Project_CTR/tree/master/makerom)
* [bannertool](https://github.com/diasurgical/bannertool)
