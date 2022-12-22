# if you type 'make' without arguments, this is the default
PROG    = draw
all:    $(PROG)

# Tell make about the file dependencies
HEAD	= scene.h render_output.h camera.h vector3.h color3.h ray.h instance.h sdf_instance.h material.h light.h hdri.h
OBJ     = main.o scene.o camera.o render_output.o vector3.o color3.o ray.o instance.o sdf_instance.o material.o light.o hdri.o

# special libraries This can be blank
LIB     =

# select the compiler and flags
# you can over-ride these on the command line e.g. make DEBUG= 
CC      = gcc
DEBUG	=
CSTD	=
WARN	= -Wall -Wextra -Werror
CDEFS	=
CFLAGS	= -Ofast -I. $(DEBUG) $(WARN) $(CSTD) $(CDEFS)

$(OBJ):	$(HEAD)

# specify how to compile the target
$(PROG):	$(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LIB) -o $@ -lm -pthread

clean:
	rm -f $(OBJ) $(PROG)

timepng:
	make
	bash -c 'time make png'

png:
	make
	./draw | ./pnmtopng > output.png