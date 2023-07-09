# Shell_C

Ce fichier contient le rapport pour le projet SEC, réalisé par Arthur Picard le 2 juin 2023.

## Choix de conception

- Les processus sont séparés en avant-plan et en arrière-plan pour faciliter la distinction entre les deux. Le programme utilise "pid-fg" pour accéder directement au processus en avant-plan.
- Une liste de taille 1024 est utilisée pour stocker les processus en arrière-plan, permettant ainsi d'ajouter ou de supprimer des éléments facilement. La liste a été préférée à un tableau pour éviter les contraintes d'absence de processus.
- Les messages d'erreurs fonctionnels sont numérotés pour faciliter l'identification de leur origine, étant donné que les mêmes fonctions sont utilisées à plusieurs reprises.
- Des fonctions ont été créées pour les actions lj, sj, bg, fg et susp afin d'éviter d'alourdir le code.

## Questions

### Question 1

L'invite de commande choisie pour le minishell est ">>>", afin de correspondre au TP pratiqué en cours.

### Question 2

Pour tester le problème d'affichage de l'invite de commande pendant l'exécution d'une commande, la commande "sleep 2" peut être utilisée pour mettre en évidence cette anomalie dans l'affichage.

### Question 3

Pour résoudre le problème d'affichage de l'invite de commande avant ou pendant l'exécution d'une commande, la fonction "waitpid" a été utilisée pour attendre la fin d'exécution de la commande souhaitée.

### Question 4

Pour la commande "cd", il est d'abord nécessaire de vérifier si un répertoire est fourni en argument par l'utilisateur. Ensuite, il faut vérifier si le répertoire choisi par l'utilisateur existe réellement. La fonction "chdir" est utilisée pour changer de répertoire courant. Lorsque la commande "cd" n'a pas d'argument (un répertoire), la commande "chdir("/")" est exécutée pour revenir à la racine.

La fonction "exit" recherche la commande "exit" parmi celles saisies par l'utilisateur. Si la commande est détectée, le programme minishell se termine en utilisant "exit(1)".

### Question 5

Pour exécuter un processus en arrière-plan, il suffit de ne pas utiliser "waitpid". Pour mettre à jour l'état des processus en arrière-plan et les supprimer de la liste des processus en arrière-plan une fois terminés, il faut ajouter le code suivant lorsque aucun processus en arrière-plan n'est ajouté :

```c
if (WIFEXITED(status) || WIFSIGNALED(status)) {
    del_PID(pid_fg);
}
```

### Question 6

#### Fonction lj

La fonction lj() permet d'afficher la liste des processus en arrière-plan. Elle affiche le PID, l'ID, l'état du processus et la commande associée à chaque processus. Pour cela, chaque élément de la liste des processus est récupéré grâce à une structure contenant l'ID, le PID, la commande et l'état du processus.

#### Fonction sj

La fonction sj() permet de suspendre un processus. Elle recherche d'abord si le processus en question existe. Si c'est le cas, elle récupère son indice, envoie le signal SIGSTOP pour arrêter le processus, puis remplace l'état "Actif" par "Suspendu" dans la liste des processus.

Après avoir utilisé sj, on remarque que le processus passe bien de l'état "Actif" à "Suspendu", ce qui confirme que la commande sj fonctionne comme attendu.

#### Fonction bg

La fonction bg() permet de reprendre l'exécution d'un processus en arrière-plan. Elle est presque identique à la fonction sj(), mais utilise le signal SIGINT au lieu de SIGSTOP. L'état "Suspendu" dans la liste des processus est remplacé par "Actif".

Après avoir utilisé bg, on remarque que le processus passe bien de l'état "Suspendu" à "Actif", ce qui confirme que la commande bg fonctionne comme attendu.

#### Fonction fg

La fonction fg() permet de reprendre l'exécution d'un processus en avant-plan. Elle est similaire à la fonction sj(), mais utilise le signal SIGCONT pour continuer l'exécution du processus. Le processus est également supprimé de la liste des processus en arrière-plan.

On peut observer que le processus est revenu en avant-plan.

## Difficultés et problèmes

J'ai rencontré quelques difficultés dans la gestion et la compréhension des signaux. De plus, la multitude de fonctions en C m'a posé problème, car je n'étais pas forcément familier avec toutes. Après la remise intermédiaire, j'ai également eu du mal à intégrer proprement les tubes.

## Problèmes

Il y a un petit problème avec les fonctions utilisant les fonctions find-ID et find-PID. Lorsqu'aucun processus correspondant à l'ID ou au PID n'est trouvé, ces fonctions ne renvoient pas -1, ce qui peut poser problème si l'identification est incorrecte.
