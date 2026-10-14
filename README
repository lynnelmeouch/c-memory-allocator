# Allocateur Mémoire

Allocateur mémoire simple en C, réalisé dans le cadre d'un TP. Il réimplémente de façon simplifiée `malloc` et `free` pour gérer dynamiquement une zone mémoire fournie.

## Réalisé en Binôme

## Fonctionnement

L'allocateur gère une liste chaînée de blocs libres :

- **`mem_init`** : initialise la zone en enregistrant son adresse de début, sa taille et la stratégie de recherche, puis crée un premier bloc libre couvrant toute la zone.
- **`mem_alloc`** : cherche un bloc libre assez grand avec la stratégie **first-fit**. Si le bloc est plus grand que nécessaire, il est découpé et le reste reste libre. S'il a exactement la bonne taille, il est utilisé entièrement. La fonction retourne l'adresse de la zone utilisable.
- **`mem_free`** : remet le bloc dans la liste des blocs libres et le fusionne avec les blocs libres voisins (précédent et/ou suivant) pour limiter la fragmentation.

## Contenu du package

| Fichier | Rôle |
|---|---|
| `mem.c` / `mem.h` | Implémentation et interface de l'allocateur (`mem_init`, `mem_alloc`, `mem_free`) |
| `common.c` / `common.h` | Fonctions et définitions communes |
| `malloc_stub.c` / `malloc_stub.h` | Redirige `malloc`/`free` vers notre allocateur pour tester une application réelle |
| `memshell.c` | Petit programme de test interactif de l'allocateur |
| `test_init.c` | Petit programme testant simplement l'initialisation de l'allocateur |
| `Makefile` | Compile ces programmes et permet de tester l'allocateur avec une application réelle (`make test_ls`) |

> **Attention :** sans implémentation correcte du début de l'allocateur, `test_init` boucle indéfiniment.

## Compilation et tests

```bash
make            # compile les programmes
./test_init     # teste l'initialisation
./memshell      # test interactif
make test_ls    # teste l'allocateur avec la commande ls
```

**N.B. :** dans `memshell`, l'adresse affichée pour un bloc occupé correspond au début de la zone utilisable par l'utilisateur, et non à l'adresse interne utilisée par l'allocateur.
