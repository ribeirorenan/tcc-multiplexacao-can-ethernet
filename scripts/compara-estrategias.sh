#!/bin/bash

# for stat in Max Q_90 Median ^Mean CV ; do ( bash ./compara-estrategias.sh $stat '100%' ); done

if [ ! -z $1 ]; then
	FILTRO="$1"
else
	FILTRO="TrMean"
fi

	
echo
echo "Filtro:  $FILTRO" 

if [ ! -z $2 ]; then
	CENARIO="$2"
else
	CENARIO=""
fi

ESTRATEGIAS=$(find ./ -wholename "*1-esperandonafila_stats.txt" | sed 's/^.*excel\///; s/\/.*$//' | sort -u)

if [ -z $CENARIO ]; then 

	echo 
	for E in $ESTRATEGIAS; do(

	   echo "Estrategia: ${E}"
	   find ./ -wholename "*${E}/1-esperandonafila_stats.txt" | sort -n | xargs grep -h "$FILTRO" | awk '{print "\t"$2}'

	); done

else

	echo "Cenario: $CENARIO"

	echo | awk '{print "\nAtraso\tEfic%\tPonte%\tEstrategia"}'
	for E in $ESTRATEGIAS; do(

	   FILE=$(find ./ -wholename "*${E}/1-overhead.txt" | grep "${CENARIO}")
	   PROTO=$(awk 'BEGIN{tput0=1}{ if($3 ~ "cangen") tput0 = $4; if($3 ~ "ponte") tput1 = $4; if ($4 ~ "protocolo") proto = $5 }END{printf "%.2f\t%.2f\n", proto, (tput1/tput0 * 100)}' $FILE)
	   find ./ -wholename "*${E}/1-esperandonafila_stats.txt" | grep "${CENARIO}" | xargs grep -h "$FILTRO" | awk -v e="${E}" -v p="$PROTO" '{printf "%.2f\t%s\t%s\n", $2, p, e}' 

	); done | sort -nr -k 1

fi