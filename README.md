compile with:
    g++ -std=c++14 -O2 main.cpp -o check_3_colorability

Compiled program reads graph either from stdin or a file, whose name was passed as the parameter and returns 1 if it is 3-colorable.

There are generators that write into stdin and take 2 parameters: number of vertices and edge probability.

One possible test would be:
    while :; do python3 generate_3_colorable.py 100 0.6 | check_3_colorability; done
