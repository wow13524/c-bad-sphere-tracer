# if you type 'make' without arguments, this is the default
NETPBM_CONFIG_OVERRIDE = "DEFAULT_TARGET = nonmerge\n\
NETPBMLIBTYPE=unixstatic\n\
NETPBMLIBSUFFIX=a\n\
STATICLIB_TOO=N\n\
CFLAGS = -O3 -ffast-math -w\n\
CFLAGS_MERGE = -Wno-missing-declarations -Wno-missing-prototypes\n\
LDRELOC = ld --reloc\n\
LINKER_CAN_DO_EXPLICIT_LIBRARY=Y\n\
LINKERISCOMPILER = Y\n\
LEX=\n\
CFLAGS_SHLIB += -fPIC"
NETPBM_SRC = https://sourceforge.net/projects/netpbm/files/latest/download
PROG    = draw
all:    $(PROG)

# Tell make about the file dependencies
HEAD	= scene.h render_output.h camera.h vector3.h color3.h ray.h instance.h sdf_instance.h material.h light.h hdri.h
OBJ     = main.o scene.o camera.o render_output.o vector3.o color3.o instance.o sdf_instance.o material.o light.o hdri.o

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
	@$(CC) $(CFLAGS) $(OBJ) $(LIB) -o $@ -lm -pthread

pnmtopng:
	@echo "[pnmtopng] pnmtopng not found, building"
	@echo "[pnmtopng] downloading and unpacking latest source"
	@mkdir netpbm
	@curl -s -L $(NETPBM_SRC) | tar --directory=netpbm --strip-components=1 -xz

	@echo "[pnmtopng] making pnmtopng binary"
	@cd netpbm &&\
	cp config.mk.in config.mk &&\
	echo $(NETPBM_CONFIG_OVERRIDE) >> config.mk &&\
	echo '' > configure &&\
	cd converter/other &&\
	sed -i 's/pkg-config/pkg-config --silence-errors/g' Makefile ../../GNUmakefile &&\
	make -s pnmtopng --keep-going &&\
	mv pnmtopng ../../../pnmtopng

	@echo "[pnmtopng] cleaning up"
	@rm -rf netpbm.tar netpbm
	@echo "[pnmtopng] done!"

clean:
	@rm -f $(OBJ) $(PROG)
	@echo "cleaned."

png:
	@make -s --no-print-directory
	@make pnmtopng -s --no-print-directory
	@./draw | ./pnmtopng > output.png

timepng:
	@make -s --no-print-directory
	@make pnmtopng -s --no-print-directory
	@bash -c 'time make -s --no-print-directory png'