# C Bad Sphere Tracer

This is a project I've been working on since I started learning C in order to have a better grasp on the language.  This code implements a rudimentary sphere tracer written in pure C with the goal of being able to render images within a reasonable amount of time on a Raspberry Pi 4 Revision 4.  Some example renders are included below.

## Implemented Features
- Multithreading
- Monte Carlo method
- Render to terminal (full color)
- Render to .ppm
- Render to .png (depends on [Netpbm's `pnmtopng`](https://netpbm.sourceforge.net/), included in this repository precompiled for RPi4)
- Solid diffuse color
- Reflection
- Refraction
- Transmission
- HDRI support
- Raytraced shadows (treats all objects as opaque)

## Quickstart

Clone the repository and run `make png` to render the scene to `output.png`.

## Example Renders
Whitted.  Diffuse, reflection, and refraction; however, no Fresnel or HDRI support.  Clamp tonemapping.
![Fixed Refraction](example_outputs/output_whitted_hd.png)

Whitted.  Diffuse, reflection, and refraction with Fresnel and HDRI support.  Luminance tonemapping.
![HDRI, luminance tonemapping](example_outputs/output_whitted_hdri.png)

Whitted.  Cubes in scene with rendering artifacts.  Diffuse, reflection, and refraction with Fresnel and HDRI support.  ACES tonemapping.
![HDRI, ACES tonemapping](example_outputs/output_whitted_aces.png)

Whitted.  Cubes in scene with rendering artifacts.  Diffuse, reflection, and refraction with Fresnel and HDRI support.  ACES tonemapping.  Rendered in 4K.
![HDRI, ACES tonemapping, 4k](example_outputs/output_whitted_4k.png)

Monte Carlo with 1024 samples-per-pixel.  Cubes in scene.  Diffuse, reflection, and refraction with Fresnel and HDRI support.  Reflect if ray cannot be refracted.  ACES tonemapping.
![HDRI, ACES tonemapping, 4k](example_outputs/output_mc_1024spp.png)