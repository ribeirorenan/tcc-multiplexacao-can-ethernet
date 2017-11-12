#!/bin/bash
sed 's/,/\./g' "$@" | nice ${JAVA_HOME}/bin/java -Xms512m -Xmx1536m -jar ./bin/statistics.jar /dev/stdin
