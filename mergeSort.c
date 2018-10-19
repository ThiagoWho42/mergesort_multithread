#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>


struct Params
{
    int *start;
    size_t len;
    int depth;
};

// Usado para sincronizar stdout do overlap.
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void *merge_sort_thread(void *pv);


// Declaraçao da função merge,usando o algoritmo de ordenação mergesort.
void merge(int *start, int *mid, int *end)
{
    int *res = malloc((end - start)*sizeof(*res));
    int *lhs = start, *rhs = mid, *dst = res;

    while (lhs != mid && rhs != end)
        *dst++ = (*lhs < *rhs) ? *lhs++ : *rhs++;

    while (lhs != mid)
        *dst++ = *lhs++;

    // Copiando resultados
    memcpy(start, res, (rhs - start) * sizeof *res);
    free(res);
}

// Jogando a ordenação no multi-thread
void merge_sort_mt(int *start, size_t len, int depth)
{
    if (len < 2)
        return;

    if (depth <= 0 || len < 4)
    {
        merge_sort_mt(start, len/2, 0);
        merge_sort_mt(start+len/2, len-len/2, 0);
    }
    else
    {
        struct Params params = { start, len/2, depth/2 };
        pthread_t thrd;

        pthread_mutex_lock(&mtx);
        printf("Iniciando a subthread...\n");
        pthread_mutex_unlock(&mtx);

        // Criação da threads
        pthread_create(&thrd, NULL, merge_sort_thread, &params);

        //Faz  função voltar ao topo 
        merge_sort_mt(start+len/2, len-len/2, depth/2);

        // Lança a thread
        pthread_join(thrd, NULL);

        pthread_mutex_lock(&mtx);
        printf("Finalizando a subthread.\n");
        pthread_mutex_unlock(&mtx);
    }

    // Particionando o array usando o merge.
    merge(start, start+len/2, start+len);
}

// o thread-proc chama o merge_sort. E passa para ele apenas
//  dados por parametro usando o algoritmo do merge_sort
void *merge_sort_thread(void *pv)
{
    struct Params *params = pv;
    merge_sort_mt(params->start, params->len, params->depth);
    return pv;
}

// API publica para o uso do merge_sort
void merge_sort(int *start, size_t len)
{
    merge_sort_mt(start, len, 4);
}

int main()
{
    static const unsigned int N = 200048;
    int *data = malloc(N * sizeof(*data));
    unsigned int i;

    srand((unsigned)time(0));
    for (i=0; i<N; ++i)
    {
        data[i] = rand() % 100024;
        printf("%4d ", data[i]);
        if ((i+1)%8 == 0)
            printf("\n");
    }
    printf("\n");

    // Chama a multi-threds com o merge-sort
    merge_sort(data, N);
    for (i=0; i<N; ++i)
    {
        printf("%4d ", data[i]);
        if ((i+1)%8 == 0)
            printf("\n");
    }
    printf("\n");

    free(data);

    return 0;

}