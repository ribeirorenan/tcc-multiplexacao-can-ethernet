#!/bin/bash

if [ -z $1 ]; then
	echo "plot-estatisticas.sh <DIR>"
	exit -1
else
	CODE_DIR="$@"
fi

cd ${CODE_DIR}

## Atraso Absoluto, independente de ID
tar xxvf simulacao_timedif_deltas.tar.xz -C /tmp/
awk '{print $2*1000}' /tmp/time-dif.txt > 1-esperandonafila

FILE="1-esperandonafila"
bash ~/bin/statistics.sh "$FILE" | tee "$FILE"_stats.txt
bash ~/bin/plot-summary.sh "$FILE" 
mv "$FILE" "$FILE".txt

## Atraso Médio por ID (média das médias por ID)
TMPDIR=./tmp
grep TrMean ${TMPDIR}/stats_*_queue.txt | sort -n | awk '{print $2}' > 1-media_espera_na_fila.txt

FILE="1-media_espera_na_fila.txt"
bash ~/bin/statistics.sh "$FILE" | tee "$FILE"_stats.txt
bash ~/bin/plot-summary.sh "$FILE" 
mv "$FILE" "$FILE".txt
