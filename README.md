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
This was rendered with an incorrect implementation of refraction using a Whitted method.
![Broken Refraction](example_outputs/output_hd.png)

This was rendered with a corrected implementation of refraction.
![Fixed Refraction](example_outputs/output_hd_fix.png)

This was rendered with the provided HDRI, with luminance tonemapping.
![HDRI, luminance tonemapping](example_outputs/output_hdri.png)

This was rendered with the provided HDRI, with ACES tonemapping.
![HDRI, ACES tonemapping](example_outputs/output_aces.png)

This was rendered with the provided HDRI, with ACES tonemapping in 4k.
![HDRI, ACES tonemapping, 4k](example_outputs/output_4k.png)

This was rendered with the provided HDRI, with ACES tonemapping in 4k using the Monte Carlo method, but with a bad implementation of Fresnel reflection and without lighting calculations.
![HDRI, ACES tonemapping, 4k](example_outputs/output_mc_badfresnel_nolighting.png)

This was rendered with the provided HDRI, with ACES tonemapping in 4k using the Monte Carlo method, but with all features implemented correctly with 1024 samples-per-pixel.
![HDRI, ACES tonemapping, 4k](example_outputs/output_mc_1024spp.png)