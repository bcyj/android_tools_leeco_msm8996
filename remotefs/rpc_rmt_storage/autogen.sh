#!/bin/sh

# autogen.sh -- Autotools bootstrapping
#

aclocal &&\
autoheader &&\
autoconf &&\
automake --add-missing --copy

