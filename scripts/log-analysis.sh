#!/bin/bash

if [ -z $1 ]; then
	echo "analysis.sh <DIR>"
	exit -1
else
	CODE_DIR="$@"
fi

cd ${CODE_DIR}

CANDUMP=/home/renan/College/TCC/1-analise-final/100%-excel/100%-excel.log
wc -l $CANDUMP

awk '{print $2*1000}' time-dif.txt  > /tmp/time-dif_esperandonafila
bash ~/bin/plot-summary.sh /tmp/time-dif_esperandonafila && mv /tmp/time-dif_esperandonafila-r-summary.png ./1-esperandonafila_r-summary.png
bash ~/bin/statistics.sh /tmp/time-dif_esperandonafila | tee 1-esperandonafila_stats.txt

## ===> COMENTAR ABAIXO <=== 
#exit 1


## CANGEN
awk '{print $3}' ${CANDUMP} | sed 's/#.*//' | gawk --non-decimal-data '{ $1 = sprintf("%d", "0x" $1) } 1' | sort | uniq | sort -n > ids.txt
awk '{print $3,$1}' ${CANDUMP} | sed 's/[()]//g;s/#.* / /' | awk '{id=$1; milis=$2*1000; if(last[id]==0){last[id]=milis} else{print id,(milis-last[id]); last[id]=milis}}' \
 | gawk --non-decimal-data '{ $1 = sprintf("%d", "0x" $1) } 1'  > candump_cycles_dec.txt

awk '{print $2}' candump_cycles_dec.txt > /tmp/time-dif_cycles_cangen && bash ~/bin/plot-summary.sh /tmp/time-dif_cycles_cangen  && mv /tmp/time-dif_cycles_cangen-r-summary.png ./z_cycles_cangen_r-summary.png
awk '{print $3*1000}' time-dif.txt  > /tmp/time-dif_cycles_ponte  && bash ~/bin/plot-summary.sh /tmp/time-dif_cycles_ponte   && mv /tmp/time-dif_cycles_ponte-r-summary.png  ./z_cycles_ponte_r-summary.png


TMPDIR=./tmp
mkdir ${TMPDIR}

rm ${TMPDIR}/stats_*_cangen.txt
cat ids.txt | while read ID; do ( grep ^$ID\\b candump_cycles_dec.txt | awk '{print $2}' | ~/bin/statistics.sh > ${TMPDIR}/stats_${ID}_cangen.txt ); done


## BRIDGE
rm ${TMPDIR}/stats_*_atrasos.txt ${TMPDIR}/stats_*_ponte.txt 2> /dev/null
cat ids.txt | while read ID; do ( grep ^$ID\\b time-dif.txt | awk '{if($2>0)print $2*1000}' | ~/bin/statistics.sh > ${TMPDIR}/stats_${ID}_atrasos.txt ); done
cat ids.txt | while read ID; do ( grep ^$ID\\b time-dif.txt | awk '{if($3>0)print $3*1000}' | ~/bin/statistics.sh > ${TMPDIR}/stats_${ID}_ponte.txt ); done

## COMPARE
egrep '^TrMean:' ${TMPDIR}/stats_*_cangen.txt | awk '{print $1"\t"$2}'    | sed 's/^.*stats_//;s/_cangen.*:/ /' | tr " " "\t" | sort -n > /tmp/cycle-cangen
egrep '^TrMean:' ${TMPDIR}/stats_*_ponte.txt  | awk '{print $1"\t"$2}'    | sed 's/^.*stats_//;s/_ponte.*:/ /'  | tr " " "\t" | sort -n > /tmp/cycle-ponte
join -j 1 /tmp/cycle-cangen /tmp/cycle-ponte | tr " " "\t" | sort -n > /tmp/join
join -j 1 /tmp/cycle-cangen /tmp/cycle-ponte | tr " " "\t" | awk '{if(($3-$2)>0) diff=$3-$2; else diff=$2-$3; printf "%d\t%.2f\n", $1,diff}'        | sort -n > /tmp/overhead
join -j 1 /tmp/cycle-cangen /tmp/cycle-ponte | tr " " "\t" | awk '{if(($3-$2)>0) diff=$3-$2; else diff=$2-$3; printf "%d\t%.2f\n", $1,diff/$2*100}' | sort -n > /tmp/percent

## OVERHEAD

## ATENCAO===> COM VIRGULAS
#join -j 1 /tmp/join /tmp/percent  | tr "." "," > /tmp/join2
#join -j 1 /tmp/join2 /tmp/overhead | tr "." "," | sort -n -k 4 | awk 'BEGIN{print "#ID CANGEN BRIDGE ATRASO_% ATRASO_ms"}{print $0}' | tr " " "\t" > 1-media_overhead.txt

## ATENCAO===> COM PONTOS
join -j 1 /tmp/join /tmp/percent  > /tmp/join2
join -j 1 /tmp/join2 /tmp/overhead | sort -n -k 4 | awk 'BEGIN{print "#id cangen bridge overhead_% overhead_ms"}{print $0}' | tr " " "\t" > 1-media_overhead.txt

head 1-media_overhead.txt
echo '...'
tail 1-media_overhead.txt

grep -v '#' 1-media_overhead.txt | awk '{print $5}' > /tmp/time-dif_media_espera_na_fila
bash ~/bin/plot-summary.sh /tmp/time-dif_media_espera_na_fila && mv /tmp/time-dif_media_espera_na_fila-r-summary.png ./1-media_espera_na_fila_r-summary.png
bash ~/bin/statistics.sh /tmp/time-dif_media_espera_na_fila | tee ./1-media_espera_na_fila_stats.txt


grep -v '#' 1-media_overhead.txt | awk '{if($4 > 0) {sum   += $4; count++; sum_ms=$3-$2}}END{print "\n## Média dos Atrasos: "sum/count" % \n## Média dos Atrasos: "sum_ms/count" ms"}' > 1-overhead.txt
grep -v '#' 1-media_overhead.txt | awk '{if($2 > 0){vazao += 16 * (1000.0/$2)}}END{print "\n## Vazão cangen: " vazao/2^10 " Kbps"}' >> 1-overhead.txt
grep -v '#' 1-media_overhead.txt | awk '{if($3 > 0){vazao += 16 * (1000.0/$3)}}END{print   "## Vazão ponte:  " vazao/2^10 " Kbps"}' >> 1-overhead.txt
awk '{print $4}' time-dif.txt | grep -v index |sh ~/bin/statistics.sh | grep TrMean | awk '{quadros=$2; taxa_efetiva=(1-(14+4)/(quadros*16))*100; print "\n## Eficiencia de protocolo: "taxa_efetiva" %"}' >> 1-overhead.txt
cat 1-overhead.txt

grep -v '#' 1-media_overhead.txt  | awk '{printf "%d\t%f\n", $1, $5}' | sort -nk 1 > 1-Atraso_vs_ID.txt
grep -v '#' 1-media_overhead.txt  | awk '{printf "%d\t%f\n", $2, $5}' | sort -nk 1 > 1-Atraso_vs_Ciclo.txt

## BACKUP and CLEAN UP
tar -cvJf simulacao_timedif_deltas.tar.xz candump_*.txt time-dif.txt && rm candump_*.txt time-dif.txt
tar -cvJf tmp-stats_per_ID.tar.xz tmp && rm -R ./tmp