#!/bin/bash

############################################################
#
# Author: Rasmus Zakarias
# Project: Umbrella
# Date: 27. Nov 2014
# 
# Description: The Umbrella project is defined by this file: One
# umbrella script to build everything.
#
# We depend only on deptree.sh file to describe the build
# order resolving inter-project dependencies.
#
# Usage:
#
# build.sh <config> [<project name>] - where <config> is the
# configuration debug or release and <project name> is the name of the
# sub-autotool project to build. Giving no project name builds all
# projects.
#
############################################################

BUILDDIR=${PWD}/sys
MAKE_SCRIPTS=$(find . -type f | grep -v .svn  | grep -e "linux.mak$")

COLOR_NORM="\033[0m"
COLOR_GREEN="\033[1;32m"
COLOR_WHITE="\033[1;38m"
COLOR_RED="\033[1;31m"
COLOR_YELLOW="\033[1;33m"

echo ${0}

if [ ! -f /bin/bash ]; 
then
 echo "Error: This IS a Bach script, sh does not cut it."
 exit -1;
fi

#
# Note build time in seconds since EPOCH
#
START=$(date +%s)

theend() {
 echo "Umberlla build script ran for " $(( $(date +%s) - ${START})) " seconds building ${2} projects."
 exit ${1}
}

put() {
    eval hash"$1"='$2';
}

get() {
    eval echo '${hash'"\"${1}\""'#hash}';
}

print_start() {
    printf "${COLOR_WHITE}%-50s %s${COLOR_NORM}" ${1}
}

print_ok() {
    echo -e "${COLOR_WHITE} [ ${COLOR_GREEN} OK ${COLOR_NORM}${COLOR_WHITE} ] ${COLOR_NORM}"
}

print_fail() {
    echo -e "${COLOR_WHITE} [ ${COLOR_RED}FAIL${COLOR_NORM}${COLOR_WHITE} ] ${COLOR_NORM}"
}

print_skip() {
    echo -e "${COLOR_WHITE} [ ${COLOR_YELLOW}SKIP${COLOR_NORM}${COLOR_WHITE} ] ${COLOR_NORM}"
}

helper_assign_priority() {
    echo $(for mkfile in ${MAKE_SCRIPTS}; do
	PROJ=${mkfile}
	P=$(echo ${PROJ} | grep -e ${1})
	if [ "${P}" != "" ]; then
	    echo -n -e "${R}\n${2}${PROJ}"
	else
	    echo -n -e "${R}\n${PROJ}"
	fi
    done  | sort )
}

set_swl() {
    MAKE_SCRIPTS=$(helper_assign_priority ${1} ${2})
}

build_dir() {
    CURDIR=${1}
    if [ -z ${2} ]; then 
	CONFIG="release";
    else
	CONFIG=${2};
    fi
    pushd ${CURDIR} 2>&1 > /dev/null
    FST=0;
    if [ ! -f Makefile ]; 
    then
	OUT1=$(make -f linux.mak ${CONFIG} BUILDDIR=${BUILDDIR} 2>&1)
	FST=$?
    fi
    OUT2=$(make install 2>&1)
    SND=$?

    if [[ "${FST}" == "0" && "${SND}" == "0" ]]; then
	echo "1"
	echo "" > build.log	
    else

	echo "${OUT2}" >> build.log
	echo "0"
    fi

    popd 2>&1 > /dev/null
}

clean_dir() {
    CURDIR=${1}
    pushd ${CURDIR} 2>&1 > /dev/null
    OUT=$(make --silent -f linux.mak clean 2>&1)
    FST=$?
    if [[ "$?" == "0"  ]]; then
	echo "1"
    else
	echo "0"
    fi
    popd 2>&1 > /dev/null
}


#------------------------------------------------------------
# 
# Build all linux.mak files.
#
# For each make script called "linux.mak" in the sub-directories we
# change directory to where that file is make. However, if there is a
# filter list and the current directory is not on that list we will
# skip it.
#
#------------------------------------------------------------
build_all_linuxdotmak() {
    . deptree.sh
    ALL_OK="1"
    BUILT_PROJECTS=0
    for mkfile in ${MAKE_SCRIPTS}; do
	OK="1"
	CURDIR=$(dirname ${mkfile:2});
	if [ "${FILTER}" == "" ]; then
	    print_start ${CURDIR:0:${#CURDIR}-6};
	    case ${1} in
		"build")
		    OK=$(build_dir ${CURDIR} ${2}) 
		    ;;
		"clean")
		    OK=$(clean_dir ${CURDIR})
		    ;;
	    esac

	    if [ "${OK}" == "0" ]; then
		print_fail;
		ALL_OK="0";
	    else
		print_ok;
	    fi
	else
	    B="0"
	    for proj in ${FILTER}; do
		if [ "$(echo ${mkfile} | grep ${proj})" != "" ]; then
		    case ${1} in
			"build")
			    OK=$(build_dir ${CURDIR} ${2})
			    ;;
			"clean")
			    OK=$(clean_dir ${CURDIR})
			    ;;
		    esac
		    B="1"
		fi;
	    done
	    
	    if [ "${B}" != "0" ]; then
		print_start ${CURDIR:0:${#CURDIR}-6};
		if [ "${OK}" == "0" ]; then
		print_fail;
		ALL_OK="0";
		else
		    BUILT_PROJECTS=$(( ${BUILT_PROJECTS} + 1 ));
		    print_ok;
		fi
	    fi
	fi
    done
    
    [ "${ALL_OK}" == "1" ] && theend 0 ${BUILT_PROJECTS}|| theend -1 ${BUILT_PROJECTS}
}

info_proj() {
    FOUND="0"
    echo -e "Information about project ${1}:\n\n"
    if [ "${1}" == "" ]; then
	echo "Please state the name of an existing project also.";
	exit -1;
    fi

    for mkfile in ${MAKE_SCRIPTS}; do
	if [ "$(echo ${mkfile} | grep ${1})" != "" ]; then
	    FOUND="${mkfile}";
	fi;
    done
    
    if [ "${FOUND}" != "0" ]; then
	README=$(dirname ${FOUND})/../README
	if [ -f ${README} ]; then
	    cat ${README}
	else
	    echo "Project ${1} is not documented please write \"${README}\".";
	fi
    else
	echo "Project ${1} does not exists.";
    fi
    echo -e "\n\n "
}


############################################################
#
# Script that provides Eclipse project support
#
############################################################

#TODO(rwz): Add the content needed to create an Eclipse CDT project from automake like on OSX
BASE64_PROJ_TEMPLATE_ZIP_LIN_LUNA=""
#TODO(rwz): Add the content needed to create an Eclipse CDT project from automake like on OSX
BASE64_PROJ_TEMPLATE_ZIP_WIN_LUNA=""
BASE64_PROJ_TEMPLATE_ZIP_OSX_LUNA=`cat <<_EOM
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
_EOM`

exec_test() {
    echo $(command -v ${1})
}

check_dep_programs() {
    if [ "$(exec_test base64)" == "0" ]; 
    then
	echo "base64 command does not exists as required."
	exit -1;
    fi

    if [ "$(exec_test unzip)" == "0" ]; 
    then
	echo "unzip command does not exists as required."
	exit -1;
    fi

   if [ "$(exec_test sed)" == "0" ]; 
    then
	echo "sed command does not exists as required."
	exit -1;
    fi

   if [ "$(exec_test realpath)" == "0" ]; 
    then
	echo "realpath command does not exists as required."
	exit -1;
    fi

}

test_dir() {

    if [ ! -d ${1}/linux ]; then
	echo "Error: ${1} does not look like an Umbrella automake project."
	exit -1;
    fi

    if [ ! -f ${1}/linux/linux.mak ]; 
    then
	echo "Error: ${1} does not look like an Umbrella automake project."
	exit -1;
    fi
}

diewarn_if_dir_exists() {
  if [ -d ${1} ]; then
      echo "Warning: Directory already exists in ${1}."
      exit -1;
  fi
}

replace_pattern() {
    pattern=${1};
    text=${2}
    file=${3}
    target=${4}
    echo "TARGET: ${target}"
    cat ${file} | sed 's|<\!--###'${pattern}'###-->|'${text}'|g' > ${target};
    if [ "$?" != "0" ]; then
	echo "Warning: Creating file ${target} resulted in non-zero return code.";
    fi
}

#
# ${1} - the path of the project in automake 
#
make_eclipse_project() {
  path=${1};
  if [ "${path}" == "" ];
  then
      echo "${0}: <path to Unbrella-automake project>";
      exit -1;
  fi

  check_dep_programs

  test_dir ${path}

  path=$(realpath ${path})

  name=$(basename ${path})
  target_eclipse_dir=${path}/../${name}_eclipse_proj
  build_path="${path}/linux"
  temp_file="/tmp/eclipse.proj.zip.b64"

  diewarn_if_dir_exists ${target_eclipse_dir}
  
  mkdir -p ${target_eclipse_dir}
  echo ${BASE64_PROJ_TEMPLATE_ZIP_OSX_LUNA} | base64 -d > ${temp_file}
  
  unzip ${temp_file} -d ${target_eclipse_dir}
  rm -f ${temp_file}
  
  replace_pattern "name" ${name} "${target_eclipse_dir}/.project" ${target_eclipse_dir}/.project1
  replace_pattern "path" ${path} "${target_eclipse_dir}/.project1" ${target_eclipse_dir}/.project
  rm -f ${target_eclipse_dir}/.project1

  replace_pattern "path" ${build_path}  ${target_eclipse_dir}/.cproject ${target_eclipse_dir}/.cproject1
  mv ${target_eclipse_dir}/.cproject1 ${target_eclipse_dir}/.cproject

}
############################################################
#
# END of Eclipse project support
#
############################################################



############################################################
#
# New Project
#
############################################################


temp_zip_file=/tmp/umbrella.project.zip

PROJECT_TEMPLATE_ZIP=`cat <<_EOF
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
_EOF`

CODENAME_PTRN="<!--###codename###-->"
PROJNAME_PTRN="<!--###projname###-->"
MAINFILE_PTRN="<!--###mainfile###-->"

new_project() {
echo -n "Project path with basename as wanted project-name> "
read projectpath
echo -n "Project codename> "
read codename
echo -n "mainfile> "
read mainfile

projectname=$(basename ${projectpath})

echo -n "will create directory ${projectpath} with project ${projectname} continue [Y/n]? " 
read ok

if [ "${ok}" == "" ]; 
then 
 ok="Y";
fi

if [ "${ok}" == "Y" ];then
echo -e "${PROJECT_TEMPLATE_ZIP}" | base64 -d > ${temp_zip_file}
if [ -d ${projectpath} ];
then
echo "Error: project in ${projectpath} already exists. Please remove first."
exit -1
fi 
mkdir -p ${projectpath}
unzip ${temp_zip_file} -d ${projectpath} 
rm -f ${temp_zip_file}

## FILES ##
# template.am -> <!-###projname##-->.am
# configure.am replace
# src/Makefile.am replace
# test/Makefile.am replace
# include/Makefile.am replace
# touch <!--###mainfile###-->.c

mv ${projectpath}/template.am ${projectpath}/${projectname}.am
echo ${projectpath}/configure.ac ${projectpath}/configure.ac1
replace_pattern "projname" ${projectname} ${projectpath}/configure.ac ${projectpath}/configure.ac1
rm -f ${projectpath}/configure.ac
replace_pattern "codename" ${codename}    ${projectpath}/configure.ac1 "${projectpath}/configure.ac"
rm -f ${projectpath}/configure.ac1
echo ${projectpath}/configure.ac ${projectpath}/configure.ac1
replace_pattern "mainfile" "src/${mainfile}"  "${projectpath}/configure.ac" "${projectpath}/configure.ac1"
mv ${projectpath}/configure.ac1 ${projectpath}/configure.ac

replace_pattern "projname" "${projectname}" "${projectpath}/src/Makefile.am" "${projectpath}/src/Makefile.am1"
rm -f ${projectpath}/src/Makefile.am

replace_pattern "mainfile" "${mainfile}"    "${projectpath}/src/Makefile.am1" "${projectpath}/src/Makefile.am"
rm -f ${projectpath}/src/Makefile.am1

replace_pattern "projname" "${projectname}" "${projectpath}/test/Makefile.am" "${projectpath}/test/Makefile.am1"
mv ${projectpath}/test/Makefile.am1 ${projectpath}/test/Makefile.am

replace_pattern "mainfile" "${mainfile}" "${projectpath}/include/Makefile.am" "${projectpath}/include/Makefile.am1"
mv ${projectpath}/include/Makefile.am1 ${projectpath}/include/Makefile.am

echo -e "#include <${mainfile}.h>\nstatic char * libname() { return \"${projectname}\"; } " > ${projectpath}/src/${mainfile}.c
touch ${projectpath}/include/${mainfile}.h
fi 
}

function run_tests() {
    for mkfile in ${MAKE_SCRIPTS}; do
	if [ "$(echo ${mkfile} | grep ${1})" != "" ]; then
	    DIR=$(dirname ${mkfile})
	    FAILED=0
	    NOTESTS=0
	    FOUND="${mkfile}";
	    TESTS=`find ${DIR}/test -perm -u=x -type f`
	    if [ ! -d ${DIR}/test ]; then
		echo "No tests, You need to build this project first";
	    else
		for t in ${TESTS}; do
		    e=`${t} 2>&1 > /dev/null ; echo $?`
		    print_start "Test:\"${t}\"";
		    if [ "${e}" != "0" ]; then
			print_fail
			FAILED=$((${FAILED} + 1))
		    else
			print_ok
		    fi
		    
		    NOTESTS=$((${NOTESTS} + 1))
		done
		
		echo "Ran ${NOTESTS} out of which ${FAILED} tests failed."
	    fi
	fi;
    done
    
    exit ${FAILED}
}

############################################################
#
#
#
############################################################

echo "############################################################"
echo "# Umbrella build script system                             #"
echo "#                                                          #"
echo "# Available commands are:                                  #"
echo "#  new <proj_path> create a new project, e.g platform/time.#"
echo "#  release [proj_1,...,proj_n] build projects listed.      #"
echo "#  debug [proj_1,...,proj_n] build projects listed  .      #"
echo "#  clean [proj_1,...,proj_n] clean projects listen.        #"
echo "#  eclipse [proj1,...,proj_n] build Eclipse proj.          #"
echo "#  info <proj> describe project <proj>.                    #"
echo "#                                                          #"
echo "############################################################"

CMD=$1
shift
FILTER=${@}

case $CMD in
    "new")
	new_project ${1}
	;;
    "release") 
	if [ "$FILTER" == "" ]; then
	    echo "Building all projects in ${BUILDDIR}: "
	else
	    echo "Building project(s) in ${BUILDDIR}: ${@}"
	fi
	build_all_linuxdotmak "build" "release";
	;;
     "debug") 
	if [ "$FILTER" == "" ]; then
	    echo "Building all projects in ${BUILDDIR}: "
	else
	    echo "Building project(s) in ${BUILDDIR}: ${@}"
	fi
	build_all_linuxdotmak "build" "debug";

	;;
    "test")
	run_tests ${FILTER};
	;;
    "clean")
	if [ "$FILTER" == "" ]; then
	    echo "Cleaning all projects: "
	    echo "${BUILDDIR} nuked !"
	    rm -rf ${BUILDDIR}
	else
	    echo "Cleaning project(s): ${@}"
	fi
	build_all_linuxdotmak "clean";
	;;
    "eclipse")
	make_eclipse_project ${1};
	;;
    "info")
	echo "$(info_proj $1)";
	;;
    "extract_eclipse")
	$(echo -e ${BASE64_PROJ_TEMPLATE_ZIP_OSX_LUNA} | base64 -d > eclipse.proj.zip)
	;;
    "")
	echo "Please pick a command from the set above."
	;;
    *)
	echo "Unfortunately your ${CMD} command is unknown";
esac

