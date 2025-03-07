#!/bin/bash

echo "=================== GENERATING TEST FILE ==================="
echo " "
mkdir -p graphdata

if [ ! -f "inputard_4.txt" ]; then
    python3 ard_gen.py 4
fi
echo " "

if [ ! -f "inputm2_4.txt" ]; then
    python3 m2m_gen.py 4
fi
echo " "

if [ ! -f "inputard_8.txt" ]; then
    python3 ard_gen.py 8
fi
echo " "

if [ ! -f "inputm2_8.txt" ]; then
    python3 m2m_gen.py 8
fi
echo " "

if [ ! -f "inputard_16.txt" ]; then
    python3 ard_gen.py 16
fi
echo " "

if [ ! -f "inputm2_16.txt" ]; then
    python3 m2m_gen.py 16
fi
echo " "

if [ ! -f "inputard_24.txt" ]; then
    python3 ard_gen.py 24
fi
echo " "

if [ ! -f "inputm2_24.txt" ]; then
    python3 m2m_gen.py 24
fi

echo " "
exit 1