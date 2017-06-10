#!/bin/sh

# autogen.sh -- Autotools bootstrapping
#

aclocal-1.11 &&\
autoheader &&\
autoconf &&\
automake-1.11 --add-missing --copy

