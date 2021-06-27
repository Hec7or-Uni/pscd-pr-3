#!/bin/bash
#*****************************************************************
# File:   pract_3_entrega_correcta.bash
# Author: PSCD-Universidad de Zaragoza
# Date:   octubre 2020
# Coms:   script de comprobaci�n de entrega de la pr�ctica 3 de PSCD (20/21)
#         Funcionamiento: 
#          1) Dar permisos de ejecuci�n al script: "chmod u+x pract_3_entrega_correcta.bash"
#          2) Ejecutar mediante
#                  ./pract_3_entrega_correcta.bash NIP1_NIP2
#             en el directorio donde est� el fichero "practica_3_NIP1_NIP2.zip"
#             Crear� un directorio temporal donde descomprimir� el zip,
#             comprobar� si est�n los ficheros que tienen que estar,
#             informando de los problemas. Posteriormente borrar� el directorio creado
#             Si todo ha ido bien, terminar� con el mensaje
#                  ------> OK: I have found all the mandatory files
#*****************************************************************

DIR_CORRECCION=$(pwd)
PREFIX_PRACT="practica_3"
FUENTE_ZIP="${PREFIX_PRACT}_${1}.zip"
USER=a$1       #usuarios en Hendrix son "a" seguido del NIP
FALTAN=()      #ficheros que faltan
FICHEROS=(practica_3.cpp Makefile_p3 disegno.pdf)
DIRECTORIOS=(${PREFIX_PRACT}_${1} ${PREFIX_PRACT}_${1}/librerias/Logger_V3 ${PREFIX_PRACT}_${1}/librerias/Semaphore_V4)

#si alguien usa esto como nombre de directorio, se merece el castigo de perderlo
DIR_TEMPORAL=${DIR_CORRECCION}/__PSCD_NO_ES_INTERESANTE__
DIR_COMPILACION=${DIR_TEMPORAL}/${PREFIX_PRACT}_${1}

FORM=" ------> "
SEP="========================================================================="

echo ${SEP}
#----------------------------------------------------------------
#Comprueba que se invoque con un par�metro (que se supone es un NIP)
if [[ "$#" -ne 1 ]]
then
	echo "Invocar como: ${0} miNip"
	echo "------------------------------------------"
	exit
fi
#----------------------------------------------------------------
#  crear directorio temporal. Si existe, lo borra. Si alguien
#  hab�a llamado a un directorio as�, se lo merece
echo "${FORM}Creating temporal directory ..."
if [ -d "${DIR_TEMPORAL}" ]
then
	/bin/rm -rf ${DIR_TEMPORAL}
fi
mkdir ${DIR_TEMPORAL}
#----------------------------------------------------------------
#  mirar si ha entregado el fichero "zip"

if [ ! -f "${FUENTE_ZIP}" ]
then
	FALTAN=( ${FUENTE_ZIP} "${FALTAN}[@]")
	echo "${FORM}Falta fichero ${FUENTE_ZIP}"
else #comprobar est�n todos los directorios requeridos
	cp ${FUENTE_ZIP} ${DIR_TEMPORAL}/${FUENTE_ZIP}
	cd ${DIR_TEMPORAL}
	echo "${FORM}Extracting zip file ..."
	unzip ${FUENTE_ZIP}
	for dir in "${DIRECTORIOS[@]}"
	do
		if [ ! -d "${dir}" ]
		then
			echo "${FORM}Can't find directory ${dir}"
			FALTAN=(${dir} "${FALTAN}[@]")
		fi
	done
fi
#comprobar est�n todos los ficheros requeridos
if [[ ${#FALTAN[@]} -eq 0 ]] 
then
	for fich in "${FICHEROS[@]}"
	do
		echo "  Checking for ${PREFIX_PRACT}_${1}/${fich}"
		if [ ! -f "${PREFIX_PRACT}_${1}/${fich}" ]
		then
			echo "${FORM}Can't find file ${fich}"
			FALTAN=(${fich} "${FALTAN}[@]")
		fi
	done
fi

#volver al directorio original
cd ${DIR_CORRECCION}
#borrar el temporal
echo "${FORM}Deleting temporary directory ..."
/bin/rm -rf ${DIR_TEMPORAL}
#mensaje final
if [[ ${#FALTAN[@]} -eq 0 ]] 
then
	echo "${FORM}OK: I have found all the mandatory files"
else
	echo "${FORM}ERROR: I can't find some mandatory files"
	echo "${FORM}YOU CANNOT SUBMIT THIS HOMEWORK"
fi
echo ${SEP}
stty erase "^?"
exit 0
