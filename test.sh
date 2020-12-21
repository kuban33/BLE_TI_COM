#!/usr/bin/bash
com="COM10"
mac="E2:8A:76:85:FD:26"

for ((i=0;i!=20;i++))
do
	echo "START TEST RUN=${i}" | tee >(cat >&2) 
	./TE_SPIDER_TIBLE_TOOL.EXE "${com}" "${mac}" FACTORYENABLE SOURCEADC VOLMAX
	sleep 2
	./TE_SPIDER_TIBLE_TOOL.EXE "${com}" "${mac}" LEFT
	sleep 2
	./TE_SPIDER_TIBLE_TOOL.EXE "${com}" "${mac}" RIGHT
	sleep 2
	./TE_SPIDER_TIBLE_TOOL.EXE "${com}" "${mac}" BOTH
	sleep 2
	./TE_SPIDER_TIBLE_TOOL.EXE "${com}" "${mac}" CHARGE
	sleep 2
	./TE_SPIDER_TIBLE_TOOL.EXE "${com}" "${mac}" FACTORYENABLE CHARGEX
	echo "END TEST RUN=${i}"  | tee >(cat >&2)
	sleep 2
done	