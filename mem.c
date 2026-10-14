/* On inclut l'interface publique */
#include "mem.h"
/* ainsi que les détails d'implémentation locaux */
#include "common.h"
#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

/* Définition de l'alignement recherché
 * Avec gcc, on peut utiliser __BIGGEST_ALIGNMENT__
 * sinon, on utilise 16 qui conviendra aux plateformes qu'on cible
 */
#ifdef __BIGGEST_ALIGNMENT__
#define ALIGNMENT __BIGGEST_ALIGNMENT__
#else
#define ALIGNMENT 16

#endif
#define align(n) (((n) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

//Variable globales

/* structure placée au début de la zone de l'allocateur

   Elle contient toutes les variables globales nécessaires au
   fonctionnement de l'allocateur

   Elle peut bien évidemment être complétée
*/
struct allocator_header {
    size_t memory_size;
    mem_fit_function_t *fit;
    struct fb *first; //premier bloc libre
};

/* La seule variable globale autorisée
 * On trouve à cette adresse le début de la zone à gérer
 * (et une structure 'struct allocator_header)
 */
static void *memory_addr;
static size_t mem_size;

static inline void *get_system_memory_addr() {
    return memory_addr;
}

static inline struct allocator_header *get_header() {
    struct allocator_header *h;
    h = get_system_memory_addr();
    return h;
}

static inline size_t get_system_memory_size() {
    return get_header()->memory_size;
}
//structure free bloc taille+prochain
struct fb {
    // Taille, entête compris
    size_t size;
    struct fb *next;
       /* ... */
};

//structure occupied bloc taille
struct ob {
    size_t size; //taille de la zone occuppée
       /* ... */
};

//prend zone de memoire quon peux
// utilise et combier d'octets elle tient
void mem_init(void *mem, size_t taille) {
    memory_addr = mem;//fournir la zone a utilser
    mem_size = taille;
    /* On vérifie qu'on a bien enregistré les infos et qu'on
    * sera capable de les récupérer par la suite
    */
    if (mem==NULL) return;
    if (taille< sizeof(struct allocator_header)+sizeof(struct fb)) return;
    assert(mem == get_system_memory_addr());
    
    struct  allocator_header* mema=(struct allocator_header*)mem;//on cast pour pouvoir acceder au champs
    mema->memory_size = taille;//fournir sa taille(8192)
    assert(taille == get_system_memory_size());

    struct fb* zl=(struct fb*)(mem + sizeof(struct allocator_header));
    zl->size = taille-(sizeof(struct allocator_header));
    zl->next = NULL;
    mema->first = zl;
    mema->fit=mem_fit_first;
    /* On enregistre une fonction de recherche par défaut */
    mem_fit(&mem_fit_first);
}

// affiche : s'il est libre (1) ou occupé (0)
void mem_show(void (*print)(void *, size_t, int)) {
    /* ... */
    void* fin_zone_a_parcourir=(void*) (memory_addr + mem_size); //la fin de la zone max a allouer

    void* pointeur= (void*) (memory_addr + sizeof(struct allocator_header)); // on recupere ladresse de la zone apres le header
    struct fb* zl= get_header()->first;//le premier noeud zone libre
    while (pointeur < fin_zone_a_parcourir) {
        /*Si la zone pointée est une zone libre*/
        if(pointeur==zl){
            print(pointeur+sizeof(struct fb*),((struct fb*)pointeur)->size,1);//on donne en argument le pointeur et comme on sait que cest une zone libre
                                                          //on le cast pour recuperer sa taille et on affiche 1 pour zone libre
            pointeur=pointeur+((struct fb*)pointeur)->size;
            zl=zl->next;
        }
        /* Si la zone pointée est une zone occuppée */
        else{
            print(pointeur+sizeof(struct ob*),((struct ob*)pointeur)->size,0);//on donne en argument le pointeur et comme on sait que cest une zone occupée
                                                          //on le cast pour recuperer sa taille et on affiche 1 pour zone libre
            pointeur=pointeur+((struct ob*)pointeur)->size;
        }
    }
}

void mem_fit(mem_fit_function_t *f) {
    if (f==NULL)return;
    get_header()->fit=f;
}

void *mem_alloc(size_t taille) {
    if (taille ==0 ) return NULL;
    /* ... */
    size_t size = align(sizeof(struct ob) + taille);
    struct fb *fb = get_header()->fit(get_header()->first, size); // je cherche first bloc libre grand 
    if (fb == NULL) return NULL;
    if (fb->size < size) return NULL;
       
    struct fb *prev = NULL;
    struct fb *courant = get_header()->first;

    while (courant != fb){
        prev = courant ;
        courant = courant ->next;
    }

    size_t reste = fb -> size - size;
     if (reste >= sizeof(struct fb)+ALIGNMENT){
         
        struct fb *nouveau = (struct fb *)((char *)fb + size);// on cree nouveu bloc libre 
        nouveau->size = reste;
        nouveau->next = fb->next;
        
        if (prev == NULL)
            get_header()->first = nouveau;
        else
            prev -> next = nouveau;

        ((struct ob *)fb)->size = size;
     } else {
            if (prev == NULL)
                get_header()->first = fb->next;
            else
                prev->next = fb->next;
         
        // cast pour transformer en occupe 
        ((struct ob *)fb)->size = fb -> size;
    }

    // retourner l'adresse 
    return (char *)fb + sizeof(struct ob);
}
static int is_in_free_list(struct fb *list, struct fb *blk) {
    while (list) {
        if (blk >= list &&
            (char *)blk < (char *)list + list->size) {
            return 1;
        }
        list = list->next;
    }
    return 0;
}

void mem_free(void *mem) {
    if (mem == NULL) return;

    struct allocator_header *header = get_header();
    if(header==NULL) return;
    /* récupérer le bloc occupé */
    struct ob *block = (struct ob *)((char *)mem - sizeof(struct ob));
    struct fb *nouv = (struct fb *)block;

    /* détection double free */
    if (is_in_free_list(header->first, nouv)) {
        fprintf(stderr, "free d'une zone non allouee (double free)\n");
        return;
    }

    nouv->size = block->size;
    nouv->next = NULL;

    /* insertion triée */
    struct fb *prev = NULL;
    struct fb *current = header->first;

    while (current != NULL && current < nouv) {
        prev = current;
        current = current->next;
    }

    nouv->next = current;
    if (prev != NULL)
        prev->next = nouv;
    else
        header->first = nouv;

    /* fusion avec le suivant */
    if (current != NULL && (char *)nouv + nouv->size == (char *)current) {

        nouv->size += current->size;
        nouv->next = current->next;
    }

    /* fusion avec le précédent */
    if (prev != NULL && (char *)prev + prev->size == (char *)nouv) {

        prev->size += nouv->size;
        prev->next = nouv->next;
    }
}



/*Fonction renvoyant l'adresse du premier bloc libre de taille 
supérieure ou égale à size présent dans la liste de blocs libre dont 
l'adresse est list. Renvoie NULL si un tel bloc n'existe pas*/
struct fb *mem_fit_first(struct fb *list, size_t size) {
    if (list == NULL) return NULL;
    struct fb* zl=list;
    while(zl!=NULL){
        if((zl->size)>=size){
            return zl;
        }
        zl = zl->next;
    }
    return NULL;
}

/* Fonction à faire dans un second temps
 * - utilisée par realloc() dans malloc_stub.c
 * - nécessaire pour remplacer l'allocateur de la libc
 * - donc nécessaire pour 'make test_ls'
 * Lire malloc_stub.c pour comprendre son utilisation
 * (ou en discuter avec l'enseignant)
 */
size_t mem_get_size(void *zone) {
    /* zone est une adresse qui a été retournée par mem_alloc() */

    /* la valeur retournée doit être la taille maximale que
     * l'utilisateur peut utiliser dans cette zone */
    if (zone == NULL){
        return 0;
    }
    struct ob *bloc = (struct ob *)((char *)zone - sizeof(struct ob));
    return bloc->size - sizeof(struct ob);
}


/* Fonctions pour les stratégies avancées */
struct fb *mem_fit_best(struct fb *list, size_t size){
    if (list == NULL) return NULL;
    struct fb *current = list;
    struct fb *good = NULL;//la best zone a allouer
    while (current != NULL) {
        if (current->size >= size){
            //on verifie si la best est nulle ou si il ya meilleur
            if (good==NULL||current->size<good->size){
                good=current;
            }
        }
        current=current->next;
    }
    return good;
}

struct fb *mem_fit_worst(struct fb *list, size_t size){
    if (list == NULL) return NULL;
    struct fb *current=list;
    struct fb *worst=NULL;

    while (current != NULL){
        if (current->size >= size){
            if (worst == NULL || current->size > worst->size){
                worst=current;
            }
        }
        current = current->next;
    }
    return worst;
}