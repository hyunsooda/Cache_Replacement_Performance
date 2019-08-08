# Cache_Replacement_Performance
Performance Comparsion; Random VS LRU VS Genetic

## Performance Figures
<img width="1190" alt="스크린샷 2019-08-09 오전 5 11 11" src="https://user-images.githubusercontent.com/12508269/62735149-7c37b680-ba65-11e9-979d-973a56f47fa0.png">

## How to run (Your C++ Compiler should be supported at least C++17.)
- > g++ paper.cpp -std=c++1z && ./a.out -s 32768 -b 32 -a 16 -r genetic -f <trace file>
- for example :
   > g++ paper.cpp -std=c++1z && ./a.out -s 32768 -b 32 -a 16 -r genetic -f test_repl.trc
   
## How to extract the binary code of some program
- Just use Pintool and then parse according to your flavor. You must first think how to parse the binary code before using Pintool.

