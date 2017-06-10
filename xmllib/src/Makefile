CC=gcc
#CC=arm-none-linux-gnueabi-gcc

CFLAGS= -Wall\
        -DFEATURE_XMLLIB \
        -fPIC

IDIR = -I../../core/inc \
       -I../inc \
       -I../../core/inc \
       -I../../common/inc

ODIR = ../obj
LDIR = ../lib
SDIR = .

_OBJ = xmllib_decl.o\
       xmllibi_decl_utf8.o\
       xmllibi_utf8_util.o\
       xmllib_generator.o\
       xmllibi_tok_ascii.o\
       xmllib_parser.o\
       xmllibi_decl_ascii.o\
       xmllibi_tok_utf8.o\
       xmllib_tok.o\

OBJ = $(patsubst %, $(ODIR)/%,$(_OBJ))

$(ODIR)/%.o:$(SDIR)/%.c
	@-mkdir -p $(ODIR) 2> /dev/null
	$(CC) -c -o $@ $< $(CFLAGS) $(IDIR)

libxml: $(OBJ)
	@-mkdir -p $(LDIR) 2> /dev/null
	ar rcs $(LDIR)/libxml.a $(OBJ)

clean:
	rm -f $(ODIR)/*.o *~ $(LDIR)/*
