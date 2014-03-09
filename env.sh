#!/bin/bash
#
# MiniMacs Linux Bash Development Environment
#
# This script sets up aliases for building and update components fast.
#
# To use this file it must be `sourced` by the current shell, e.g.
# source env.sh or . ./env.sh for short.
#
#
export PVF_HOME=$(dirname ${0})
pushd ${PVF_HOME} >/dev/null
export PVF_HOME=${PWD}
popd >/dev/null
DEFAULT_MAKE="make clean depshape CFLAGS=\"-O0 -g3 -DSTATS_ON=1\""
alias build="make clean all CFLAGS=\"-O0 -g3 -DSTATS_ON=1\""
alias osalup="pushd ${PVF_HOME}/osal && ${DEFAULT_MAKE} && popd && ${DEFAULT_MAKE} && popd"
alias dsup="pushd ${PVF_HOME}/ds && ${DEFAULT_MAKE} && popd && ${DEFAULT_MAKE} && popd"
alias carenaup="pushd ${PVF_HOME}/carena && ${DEFAULT_MAKE} && popd && ${DEFAULT_MAKE} && popd"
alias cminimacsup="pushd ${PVF_HOME}/cminimacs && ${DEFAULT_MAKE} && popd && ${DEFAULT_MAKE} && popd"
alias encodingup="pushd ${PVF_HOME}/encoding && ${DEFAULT_MAKE} && popd && ${DEFAULT_MAKE} && popd"
alias mathup="pushd ${PVF_HOME}/math && ${DEFAULT_MAKE} && popd && ${DEFAULT_MAKE} && popd"
alias caesup="pushd ${PVF_HOME}/caes && ${DEFAULT_MAKE} && popd && ${DEFAULT_MAKE} && popd"
echo "Parvisfortis Development Environment"
echo "  build  - build the current directory with make"
echo "  <dirname>up - update <dirname>'s depshape"
echo ""
echo "  Home is ${PVF_HOME}"
export PATH=${PATH}:.
export PS1="\[\033[31m\][minimacs]\[\033[00m\] \w \[\033[31m\]\$\[\033[00m\] "