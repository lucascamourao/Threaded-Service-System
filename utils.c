#include "util.h"
#include <stdio.h>
#include <math.h>

// Implementação de funções utilitárias

bool is_empty(char *fila){
  return false;
}

bool is_prime(int number) {
    if (number < 2) {
        return false;
    }
    for (int i = 2; i <= sqrt(number); i++) {
        if (number % i == 0) {
            return false;
        }
    }
    return true;
}
