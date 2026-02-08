C'est une excellente approche. Un bon rapport de calcul haute performance (HPC) ne se limite pas à dire "ça va plus vite", mais doit expliquer **pourquoi** et **si le résultat est juste**.

Voici un plan structuré pour comparer tes deux versions, avec les métriques précises à relever.

### 1. Protocole de Validation (Véracité de la solution)

Comme nous l'avons vu, la comparaison binaire stricte est impossible à cause de la différence `float` (Metal) vs `double` (Kokkos). Voici comment contourner cela :

* **L'approche Visuelle (Qualitative)** :
* Configure les deux codes avec les mêmes paramètres : , ,  itérations.
* Génère les fichiers `.vtr` (VTK) avec `freq=100` (paramètre `--out`).
* Ouvre les résultats finaux dans **Paraview**.
* **Action :** Fais une capture d'écran "côte à côte". Les champs de solution (couleurs) doivent être visuellement identiques.


* **L'approche Globale (La "Variation")** :
* Ton code calcule une valeur scalaire à chaque itération : la `variation` (norme de la différence entre  et ).
* **Graphique 1 :** Trace la courbe `Variation = f(itération)` pour Metal et Kokkos sur le même graphe (échelle logarithmique sur Y souvent utile).
* **Analyse :** Les courbes doivent se superposer presque parfaitement au début. Si elles divergent légèrement vers la fin, explique que c'est dû à l'accumulation des erreurs d'arrondi (`float` vs `double`).


* **Le Test "Iso-Précision" (Optionnel mais rigoureux)** :
* Pour prouver que ton code Metal est juste, compile ta version Kokkos en modifiant `type.hxx` pour utiliser `float` au lieu de `double`.
* Lance les deux. Si les résultats se rapprochent (ex: identiques jusqu'à la 5ème décimale), tu as prouvé que la logique est bonne et que la différence venait uniquement de la précision.



### 2. Protocole de Performance

C'est ici que tu vas comparer ton Mac (Apple Silicon) contre le GPU NVIDIA de l'école. Il ne faut pas comparer le "Temps Total" (qui inclut l'écriture disque et l'initialisation), mais le **Temps de Calcul Pur**.

**Les métriques à mesurer :**

1. **Temps moyen par itération ()** : Prends le temps de la boucle principale et divise par le nombre d'itérations.
2. **Débit (Throughput) en MPoints/s** : C'est la métrique reine en HPC.



Cela te permet de comparer l'efficacité indépendamment de la taille de la grille (dans une certaine mesure).

**Les expériences à mener (Scaling) :**
Fais varier la taille de la grille () pour voir comment chaque architecture encaisse la charge.

| Taille de grille () | Nombre de points total | Kokkos (OpenMP) | Kokkos (Cuda) | Metal (Mac) | Séquentiel (Mac) |
| --- | --- | --- | --- | --- | --- |
| 64^3 | ~262 k | ... ms | ... ms | ... ms | ... ms |
| 128^3 | ~2.1 M  | ... ms | ... ms | ... ms | ... ms |
| 256^3 | ~16.7 M | ... ms | ... ms | ... ms | ... ms |
| 512^3 | ~134 M | ... ms | ... ms | ... ms | ... ms |

**Graphique 2 :** Trace le **Débit (MPoints/s)** en fonction de la **Taille du problème (Nombre de points)**.

**Ce que tu vas probablement observer (Analyse à mettre dans le rapport) :**

* **Petites grilles () :** Le GPU NVIDIA risque d'être lent (latence de lancement des kernels CUDA). Le CPU (OpenMP) ou Metal (mémoire unifiée) pourraient être devant.
* **Grandes grilles () :** C'est là que les GPU brillent.
* Le GPU NVIDIA de l'école devrait avoir une puissance brute énorme.
* L'architecture Apple Silicon a une **bande passante mémoire** phénoménale (jusqu'à 400 GB/s sur les puces Pro/Max). Comme ton code est "Memory Bound" (il fait peu de calculs par octet lu : juste une addition et une multiplication), le Mac pourrait surprendre et être très compétitif face à une carte NVIDIA plus ancienne ou milieu de gamme.



### 3. Comparaison de l'Implémentation (Qualité du Code)

N'oublie pas cette partie "Génie Logiciel". Compare l'effort de développement.

* **Kokkos :**
* *Avantage :* Un seul code source (`.cxx`) fonctionne sur CPU et GPU. Abstraction puissante.
* *Inconvénient :* Compilation complexe, dépendance lourde, syntaxe parfois verbeuse (`Kokkos::View`, `parallel_for`).


* **Metal :**
* *Avantage :* Contrôle total, pas de librairie tierce (natif macOS), debugging facile avec Xcode.
* *Inconvénient :* Code très verbeux (il faut 5 fichiers `.cxx/.hxx/.metal` pour faire ce que Kokkos fait en 1), gestion manuelle de la mémoire et des pipelines, non portable (ne marche que sur Apple).



### Résumé de ton plan de rapport :

1. **Introduction :** Présentation du problème physique et des deux machines (CPU/GPU, RAM).
2. **Validation :**
* Comparaison visuelle (Paraview).
* Analyse de la courbe de variation (explication de l'écart `float`/`double`).


3. **Performance :**
* Tableau des temps de calcul.
* Graphique de débit (MPoints/s).
* Discussion sur la bande passante mémoire (le facteur limitant probable).


4. **Critique des Modèles :**
* Kokkos (Portabilité) vs Metal (Performance native/Spécificité).
* Difficultés rencontrées lors du portage (ex: la gestion de la réduction qui est automatique en Kokkos mais manuelle en Metal).



C'est un plan solide qui couvre tous les aspects attendus d'un projet de calcul parallèle !
