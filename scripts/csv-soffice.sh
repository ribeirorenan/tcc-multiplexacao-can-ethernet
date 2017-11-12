#!/bin/bash
grep TrMean stats_*_atrasos.txt | sed 's/^stats_//; s/_.*://; s/\./,/' | sort -n | awk 'BEGIN{print "ID;Atraso(ms)"}{print $1";"$2}' > 1-histograma_IDxAtraso.csv
soffice --calc 1-histograma_IDxAtraso.csv

grep TrMean stats_*_cangen.txt | sed 's/^stats_//; s/_.*://; s/\./,/' | sort -n | awk '{print NR,$2}' > /tmp/cangen-cycle
grep TrMean stats_*_atrasos.txt  | sed 's/^stats_//; s/_.*://; s/\./,/' | sort -n | awk '{print NR,$2}' > /tmp/queue-delay
join -j 1 /tmp/cangen-cycle /tmp/queue-delay | awk '{print $2";"$3}' | sort -n | awk 'BEGIN{print "Periodo;Atraso(ms)"}{print $0}' > 1-histograma_CyclexAtraso.csv
soffice --calc 1-histograma_CyclexAtraso.csv
