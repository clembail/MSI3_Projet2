Voici un résumé fichier par fichier de ce que tu dois implémenter pour réussir ta migration vers Metal, en suivant la structure logique que nous avons définie.

Je divise cela en trois catégories : **L'Infrastructure**, **Le Code GPU** (ce qui remplace `.cu`), et **Le Code Hôte** (C++ qui pilote le GPU).

---

### 1. L'Infrastructure & Configuration

#### `src/CMakeLists.txt`

* **Rôle :** Chef d'orchestre de la compilation.
* **Contenu :**
* Il ne doit plus chercher CUDA.
* Il doit inclure le chemin vers le dossier `metal-cpp`.
* Il doit lier les Frameworks Apple : `Foundation`, `Metal`, `QuartzCore`.
* **Crucial :** Il doit contenir une commande personnalisée (`add_custom_command`) qui appelle l'outil en ligne de commande `metal` pour compiler tes fichiers `.metal` en un fichier binaire `default.metallib`, et placer ce fichier à côté de ton exécutable.



#### `src/metal/Context.hpp` (et `.cpp`)

* **Rôle :** Singleton de gestion du GPU (remplace l'initialisation implicite de CUDA).
* **Contenu :**
* Une classe qui s'initialise une seule fois.
* Elle demande au système le GPU par défaut (`MTLCreateSystemDefaultDevice`).
* Elle crée une file d'attente de commandes (`CommandQueue`).
* Elle charge la librairie compilée (`default.metallib`) pour pouvoir accéder à tes fonctions GPU plus tard.



#### `src/metal/SharedStructs.h`

* **Rôle :** Le "dictionnaire" commun entre le C++ et le Metal.
* **Contenu :**
* Contient uniquement des `struct` C simples.
* C'est ici que tu définis la structure qui contiendra tes dimensions (`n`, `dx`, `xmin`, `dt`, etc.) que tu mettais auparavant dans `dim.cu`.
* Utilisé par le CPU pour remplir les données, et par le GPU pour les lire.



---

### 2. Le Code GPU (Fichiers `.metal` et headers associés)

Ces fichiers sont écrits en **MSL (Metal Shading Language)**.

#### `src/metal/User.h` (Remplace `user.cu` / `user.cuh`)

* **Rôle :** Bibliothèque de fonctions mathématiques utilisateur.
* **Contenu :**
* Contient les fonctions mathématiques pures (ex: fonction source, condition initiale).
* Doit inclure `<metal_stdlib>`.
* Ce sont des fonctions `inline` (pas de `kernel` ici) appelées par les kernels principaux.



#### `src/metal/kernels.metal`

* **Rôle :** Le cœur du calcul parallèle (contient tous tes kernels).
* **Contenu :**
* Inclut `SharedStructs.h` et `User.h`.
* **Kernel `k_iteration` :** Reçoit les tableaux `u`, `v` et les constantes. Calcule la nouvelle valeur pour chaque point de la grille (l'équivalent de ton `global void iteration` en CUDA).
* **Kernel `k_init` :** Initialise le tableau avec les conditions de départ.
* **Kernel `k_boundaries` :** Applique les conditions aux limites sur les bords du domaine.
* **Kernel `k_reduction` (Variation) :** Effectue la somme locale des différences (première étape de la réduction parallèle). Utilise la mémoire `threadgroup` (équivalent de `__shared__` en CUDA) pour sommer les threads d'un même groupe.



---

### 3. Le Code Hôte (Implémentation C++ avec metal-cpp)

Ces fichiers gardent les mêmes noms de fonctions que dans les fichiers `.hxx` d'origine, mais changent l'implémentation interne.

#### `src/metal/memmove_metal.cpp` (Implémente `memmove.hxx`)

* **Rôle :** Gestionnaire de mémoire.
* **Contenu :**
* `allocate` : Crée un `MTLBuffer` en mode **Shared** (partagé). Stocke le pointeur dans une `std::map` globale pour faire le lien entre "adresse C++" et "Objet Metal". Retourne le pointeur brut (`contents()`).
* `copy...` : Comme la mémoire est unifiée sur Apple Silicon, ces fonctions ne font plus de copie physique "Device vers Host". Elles servent surtout de barrière de synchronisation (attendre que le GPU ait fini le travail avant que le CPU ne lise).



#### `src/metal/dim_metal.cpp` (Implémente `dim.hxx`)

* **Rôle :** Gestion des constantes globales.
* **Contenu :**
* Dans `setDims` : Au lieu de copier vers `__constant__` (CUDA), tu remplis une instance de la structure définie dans `SharedStructs.h`. Cette structure sera passée en argument à chaque appel de kernel.



#### `src/metal/iteration_metal.cpp` (Implémente `iteration.hxx`)

* **Rôle :** Lanceur du calcul principal.
* **Contenu :**
* Récupère le `CommandBuffer` et le `ComputeCommandEncoder`.
* Sélectionne le pipeline correspondant au kernel `k_iteration`.
* Associe les buffers (les tableaux `u` et `v`) aux index attendus par le shader.
* Envoie la structure des dimensions (via `setBytes` ou un buffer constant).
* Calcule la taille de la grille (`dispatchThreadgroups`) et lance le calcul.



#### `src/metal/values_metal.cpp` (Implémente les appels dans `values.cxx`)

* **Rôle :** Lanceur des kernels d'initialisation.
* **Contenu :**
* Similaire à `iteration_metal.cpp`, mais pour lancer `k_init` (remplir à zéro ou valeur initiale) et `k_boundaries` (gérer les bords).



#### `src/metal/variation_metal.cpp` (Implémente `variation.hxx`)

* **Rôle :** Lanceur de la réduction (calcul de l'erreur).
* **Contenu :**
* C'est souvent le plus complexe. Il lance le kernel de réduction `k_reduction`.
* Il récupère le résultat partiel (somme des blocs).
* Il termine souvent la somme finale sur le CPU (car il reste peu de valeurs à additionner), ou lance une seconde passe GPU si le tableau est immense.



---

### Résumé des fichiers inchangés (ou presque)

Tu n'as **pas** besoin de toucher à la logique métier de haut niveau :

* `main.cxx`
* `parameters.cxx`
* `scheme.cxx`
* `values.cxx` (Sauf s'il contient des appels CUDA directs, mais normalement il appelle les fonctions définies dans les headers `.hxx`).

Cette séparation te permet de garder ton projet propre : le `main` ne sait même pas qu'il tourne sur du Metal, il appelle juste des fonctions standard.
