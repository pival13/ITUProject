#include <stdint.h>
#include <stdio.h>
#include <omp.h>

#define N 16

int main()
{
    int a[N], b[N], c[N];
    for (uint8_t i = 0; i < N; ++i) {
        a[i] = b[i] = i * 1.0;
        c[i]= 0;
    }

    #pragma omp parallel num_threads(4) firstprivate(a,b) shared(c)
    {
        int id = omp_get_thread_num();int i;
        #pragma omp for schedule(guided,3)
        for (i = 0; i < N; ++i) {
            c[i] = c[i] + a[i] + b[i];
            printf("Thread %d: %d => %d\n", id, i, c[i]);
        }
    }
}