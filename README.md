[![a COIN-OR project](https://www.coin-or.org/GitHub/coin-or-badge.png)](https://www.coin-or.org)

Copyright 1997-2018, Brian Borchers.  This copy of CSDP is made
available under the Eclipse Public License.  See LICENCE for the
details of the EPL.

CSDP is a software package for solving semidefinite programming
problems.  This project is a C code for SDP that uses BLAS and LAPACK
subroutines.  The algorithm is a predictor-corrector version of the
primal-dual barrier method of Helmberg, Rendl, Vanderbei, and
Wolkowicz.

Please see the [Wiki](https://github.com/coin-or/Csdp/wiki) for more
information about the project, other projects that work with CSDP,
installation instructions, and documentation.

###Integrating CSDP with Sage (and pycsdp)

To create a working installation, you need recent autotools.
Run 

    autoreconf --install 
    automake --add-missing --copy

and then 

    ./configure 
    make
    make install # (eventually)

Currenly, this does not work on OSX 10.6 without gfortran and its libs installed. This is a bug, as in fact no 
Fortran is needed for compiling/using CSDP. If you need it to be callable within [Sage](http://sagemath.org) (e.g. to run [flagmatic](http://www.flagmatic.org)) then you need to do start Sage shell and configure with the right prefix, i.e.

    sage -sh 
    ./configure --prefix=$SAGE_LOCAL
followed by make, etc.

-----------------------
how to cite it using DOI:

[![DOI](https://zenodo.org/badge/5728/dimpase/csdp.png)](http://dx.doi.org/10.5281/zenodo.11411)
