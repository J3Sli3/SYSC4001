if [ ! -d "bin" ]; then
    mkdir bin
else
	rm zbin/*
fi
g++ -g -O0 -I . -o bin/interrupts interrupts.cpp