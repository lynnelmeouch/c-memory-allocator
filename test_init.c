#include "common.h"
#include "mem.h"
#include <assert.h>
#include <stdio.h>

static void test_alloc_max(void) {
    printf("allocation maximale\n");

    mem_init(get_memory_adr(), get_memory_size());
    printf("  taille memoire = %zu\n", get_memory_size());

    void *p = alloc_max(get_memory_size());
    printf("  alloc_max : %p\n", p);
    assert(p != NULL);

    void *q = mem_alloc(16);
    assert(q == NULL);
    printf("===== TEST ALLOC OK =====\n");
}


static void test_alloc_free(void) {
    printf("allocation + liberation\n");

    mem_init(get_memory_adr(), get_memory_size());

    void *p = mem_alloc(32);
    printf("  alloc 32 -> %p\n", p);
    assert(p != NULL);

    printf("  free %p\n", p);
    mem_free(p);

    void *q = mem_alloc(32);
    printf("  realloc 32 : %p\n", q);
    assert(q != NULL);
    printf("===== TEST ALLOC ET FREE OK =====\n");
}


static void test_allocations_multiples(void) {
    printf("allocations multiples\n");

    mem_init(get_memory_adr(), get_memory_size());

    void *a = mem_alloc(16);
    void *b = mem_alloc(32);
    void *c = mem_alloc(64);

    printf("  a=%p b=%p c=%p\n", a, b, c);

    assert(a != NULL);
    assert(b != NULL);
    assert(c != NULL);
    printf("===== TEST PLUSIEURS ALLOCATION OK =====\n");
}


static void test_double_free(void) {
    printf("double free\n");

    mem_init(get_memory_adr(), get_memory_size());

    void *p = mem_alloc(32);
    printf("  alloc 32 -> %p\n", p);
    assert(p != NULL);

    mem_free(p);
    printf("  free %p\n", p);

    mem_free(p); // 2eme free

    void *q = mem_alloc(32);
    printf("  alloc apres double free :  %p\n", q);
    assert(q != NULL);
    printf("===== TEST DOUBLE FREE OK =====\n");
}

static void test_fragmentation(void) {
    printf("fragmentation\n");

    mem_init(get_memory_adr(), get_memory_size());

    void *a = mem_alloc(32);
    void *b = mem_alloc(32);
    void *c = mem_alloc(32);

    printf("  a=%p b=%p c=%p\n", a, b, c);
    assert(a && b && c);

    mem_free(b);
    printf("  free b=%p\n", b);

    void *d = mem_alloc(16);
    printf("  alloc 16 dans trou : %p\n", d); // alignement 16 
    assert(d != NULL);
    printf("===== TEST FRAGMANTATION OK =====\n");
}

static void test_fusion_blocs_libres(void) {
    printf("fusion des blocs libres\n");

    mem_init(get_memory_adr(), get_memory_size());

    void *a = mem_alloc(64);
    void *b = mem_alloc(64);

    printf("  a=%p b=%p\n", a, b);
    assert(a && b);

    mem_free(a);
    mem_free(b);
    printf("  blocs liberes\n");

    void *c = mem_alloc(128);
    printf("  alloc 128 apres fusion : %p\n", c);
    assert(c != NULL);
    printf("===== TEST FUSION OK =====\n");
}

static void test_free_adresse_invalide(void) {
    printf("free adresse invalide\n");

    mem_init(get_memory_adr(), get_memory_size());

    int x;
    printf("  free adresse invalide %p\n", &x);
    mem_free(&x);

    void *p = mem_alloc(16);
    printf("  alloc apres free invalide : %p\n", p);
    assert(p != NULL);
    printf("===== TEST FREE ZONE INVALIDE OK =====\n");
}

int main(void) {
    printf("===== TESTS ALLOCATEUR MEMOIRE =====\n");

    test_alloc_max();
    test_alloc_free();
    test_allocations_multiples();
    test_double_free();
    test_fragmentation();
    test_fusion_blocs_libres();
    test_free_adresse_invalide();

    printf("\n===== TOUS LES TESTS OK =====\n");
    return 0;
}
