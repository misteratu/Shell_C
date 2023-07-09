#define _POSIX_C_SOURCE 200809L
#include <signal.h> //Signaux

# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <sys/wait.h> /* wait */
#include <string.h>

#include <sys/signal.h>

#include "readcmd.h"
#include <stdbool.h>
#include <string.h>   /* opérations sur les chaines */
#include <fcntl.h> 
#include <sys/types.h>

#define CONSTANT_1024 1024

enum status {
    Actif, Suspendu
};
typedef enum status status;


// Structure d'un processus
typedef struct job {
    int id;         // Identifiant du processus au sein du minishell    
    pid_t pid;      // PID d'un processus             
    char* command;  // Commande rentrée par l'utilisateur
    status status;  // Statut du processus
} job;

// Structure de la liste des processus en arrière plan
typedef struct liste_job {
    int Nb_jobs;    // Nombre de processus en arrière plan
    job list[CONSTANT_1024]; // Liste de processus en arrière plan
}liste_job;

// Définition de la liste des processus en arrière plan pour le minishell
liste_job liste_proc;

// Définition du processus en a
pid_t pid_fg;


// test
int test(int ret, const char *error_message) {
    if (ret == -1) {
        perror(error_message);
        exit(1);
    }
    return ret;
}

// Ajouter un processus en arrière plan.
int add_PID(int pid, status status_job, struct cmdline *commande) {
    if (liste_proc.Nb_jobs < 1024) {
        // Augmenter le nombre de processus en cours.
        liste_proc.Nb_jobs++;
        
        // Définition d'un deuxième nombre de processus en arrière plan car les listes commencent par 0 et non 1.
        int Nb_proc = liste_proc.Nb_jobs - 1;

        // Attribuer le PID voulu au nouveau processus en arrière plan.
        liste_proc.list[Nb_proc].pid = pid;

        // Ajouter de la commande dans la liste
        char *nouvelle_commande = malloc(10*sizeof(char));
        strcpy(nouvelle_commande, commande->seq[0][0]);
        liste_proc.list[Nb_proc].command = nouvelle_commande;

        // Ajouter le statut du processus
        liste_proc.list[Nb_proc].status = status_job;

        // Déterminer l'indice du processus dans le terminal.
        if (Nb_proc >= 1) {
            liste_proc.list[Nb_proc].id = liste_proc.list[Nb_proc - 1].id + 1;
        } else {
            liste_proc.list[Nb_proc].id = 1;
        }

    } else {
        // Sortie si le nombre de processus est trop élevé
        perror("Trop de processus pour ce mini shell");
        exit(1);
    }
    return 0;
}


// Trouver l'indice dans la liste d'un processus ayant un pid : int pid.
int find_PID(int pid) {
    // Indice de parcours dans la liste
    int k = 0;
    // Booléen déterminant si le processus a été trouvé.
    bool found = false;
    while (k < liste_proc.Nb_jobs - 1 || !found) {
        // Condition de découverte 
        if (liste_proc.list[k].pid != pid) {
            found = true;
        } else {
            // Incrémenter car processus non trouvé.
            k++;
        }
    }

    // Si le processus a été trouvé on renvoie l'indice correspondant sinon renvoie -1.
    if (found) {
        return k;
    } else {
        return -1;
    }
}

// Trouver l'indice dans la liste d'un processus ayant un id : int id.
int find_ID(int id) {
    // Indice de parcours dans la liste
    int k = 0;

    // Booléen déterminant si le processus a été trouvé.
    bool found = false;
    while (k < liste_proc.Nb_jobs - 1 || !found) {
        // Condition de découverte 
        if (liste_proc.list[k].id != id) {
            found = true;
        } else {
            // Incrémenter car processus non trouvé.
            k++;
        }
    }
    
    // Si le processus a été trouvé on renvoie l'indice correspondant sinon renvoie -1.
    if (found) {
        return k;
    } else {
        return -1;
    }
}

// Supprimer un processus ayant un pid particulier.
int del_PID(int pid) {
    // Trouver l'indice du processus correspondant dans la liste en arrière plan.
    int indice = find_PID(pid);
    if (indice != -1) {
        // Supprimer l'indice en le remplacant par le dernier élément.
        if (indice != liste_proc.Nb_jobs - 1) {
            liste_proc.list[indice] = liste_proc.list[liste_proc.Nb_jobs - 1];
        }
        // Diminution du nombre de processus.
        liste_proc.Nb_jobs--; 
        return 0;
    } else {
        // Gérer l'erreur dans le cas où le processus n'est pas en arrière plan.
        return -1;
    }
}

// Afficher la liste de processus en arrière plan.
void lj(){
    char * job_status;
    if (liste_proc.Nb_jobs > 0) {
        int i = 0;

        // Parcours de la liste de processus en arrière plan.
        while (i < liste_proc.Nb_jobs) {
            // Affichage des processus actifs.
           if (liste_proc.list[i].status == Actif) {
               job_status = "Actif";
                printf("PID : %i, Etat : %s,  Indice : %i, Commande : %s\n", liste_proc.list[i].pid, job_status,i, liste_proc.list[i].command );
            // Affichage des processus suspendus.
            } else if (liste_proc.list[i].status == Suspendu) {
                job_status="Suspendu";
                printf("PID : %i, Etat : %s,  Indice : %i, Commande : %s\n", liste_proc.list[i].pid, job_status, i, liste_proc.list[i].command);
            }
            // Incrémentation pour passer au prochain processus en arrière plan.
            i++; 
        }
    } else {
        // Cas où aucun processus est présent en arrière plan.
        printf("Aucun processus en arrière plan \n");
    }
    
}

// Suspendre un processus.
void sj(struct cmdline *commande) {
    if (commande->seq[0][1] != NULL) {
        // Convertir l'ID en entier
        int id = atoi(commande->seq[0][1]);

        // Trouver l'indice du processus avec cet ID.
        int indice = find_ID(id);

        if (indice == -1) {
            // Processus non présent en arrière plan.
            printf("L'ID ne correspond à aucun processus.\n");
        } else {
            // Récupérer le PID du processus et envoyer le signal SIGSTOP
            pid_t pid = liste_proc.list[indice].pid;
            test(kill(pid, SIGSTOP), "1");

            // Mettre à jour le statut du processus dans la liste.
            liste_proc.list[indice].status = Suspendu;

            printf("Processus avec ID %d (PID : %d) a été stoppé.\n", id, pid);
        }
    } else {
        // Cas où le nombre d'argument est mauvais.
        printf("L'ID du processus dans le shell est absent.\n");
    }
}

// Reprendre un processus en arrrière plan.
void bg(struct cmdline *commande) {
    if (commande->seq[0][1] != NULL) {
        // Récupérer l'identifiant fourni par l'utilisateur.
        int id = atoi(commande->seq[0][1]);
        int indice = find_ID(id);

        // Vérifier si l'indice correspond à un processus.
        if (indice != -1) {
            // Reprendre le job en arrière-plan.
            int job_pid = liste_proc.list[indice].pid;
            test(kill(job_pid, SIGCONT), "2");

            // Passage en statut actif.
            liste_proc.list[indice].status = Actif;
            printf("Job [%d] repris en arrière-plan\n", id);
        } else {
            // Processus non présent en arrière plan.
            printf("L'identifiant ne correspond à aucun processus.\n");
        }
    } else {
        // Cas où le nombre d'argument est mauvais.
        printf("L'identifiant du processus est absent.\n");
    }
}

// Reprendre un processus en avant-plan.
void fg(struct cmdline *commande) {
    if (commande->seq[0][1] != NULL) {
        // Récupérer l'identifiant fourni par l'utilisateur.
        int id = atoi(commande->seq[0][1]);
        int indice = find_ID(id);

        // Vérifier si l'indice correspond à un processus.
        if (indice != -1) {
            // Reprendre le job en avant-plan.
            int job_pid = liste_proc.list[indice].pid;
            test(kill(job_pid, SIGCONT), "3");
            del_PID(job_pid);
            printf("Job [%d] repris en avant-plan\n", id);

        } else {
            // Processus non présent en arrière plan.
            printf("L'identifiant ne correspond à aucun processus\n");
        }
    } else {
        // Cas où le nombre d'argument est mauvais.
        printf("L'identifiant du processus est manquant\n");
    }
}

//handler du ctrl Z
void handler_ctrlZ() {
	if (pid_fg != 0){
        // Arret du processus de pid : pid_fg.
	    test(kill(pid_fg, SIGSTOP), "4");
        printf("\nLe processus en avant plan de pid : %i est suspendu\n", pid_fg);
        //pid_fg = 0;

    } else if (pid_fg == 0) {
        // Sortie du minishell si aucun processus présent.
        printf("\nAucun processus à arreter\n");
    }
}

void susp() {
    // Arret du minishell.
    test(kill(getpid(), SIGSTOP), "5");
}

int main() {

    struct sigaction sa;
    sa.sa_handler = handler_ctrlZ;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    struct sigaction s;
    s.sa_handler = handler_ctrlZ;
    sigemptyset(&s.sa_mask);
    sigaction(SIGTSTP, &s, NULL);

    // Boucle infinie pour le shell.
    while (1) {

        pid_fg = 0;
    	
    	printf(">>> ");

        // Lire et analyser la commande entrée par l'utilisateur.
        struct cmdline *cmd = readcmd();

        // Permet de retourner à la ligne si on appuye sur enter.

        if (cmd == NULL || cmd->seq[0] == NULL) {
            
        } else {
            // Exécutez la commande cd sans créer de processus fils.
            if (strcmp(cmd->seq[0][0], "cd") == 0) {
                if (cmd->seq[0][1] == NULL || strcmp(cmd->seq[0][1], "") == 0) {
                    if (test(chdir("/"), "6") < 0) {
                        perror("cd");
                    }
                } else {
                    if (test(chdir(cmd->seq[0][1]), "7") < 0) {
                        perror("cd");
                    }
                }

            // Exécutez la commande exit sans créer de processus fils.
            } else if (strcmp(cmd->seq[0][0], "exit") == 0) {
                exit(1);

            // Exécutez la commande lj sans créer de processus fils.
            } else if (strcmp(cmd->seq[0][0], "lj") == 0) {
                lj();

            // Exécutez la commande sj sans créer de processus fils.
            } else if (strcmp(cmd->seq[0][0], "sj") == 0) {
                sj(cmd);

            // Exécutez la commande bg sans créer de processus fils.
            } else if (strcmp(cmd->seq[0][0], "bg") == 0) {
                bg(cmd);

            // Exécutez la commande fg sans créer de processus fils.
            } else if (strcmp(cmd->seq[0][0], "fg") == 0) {
                fg(cmd);

            // Exécutez la commande susp sans créer de processus fils.
            } else if (strcmp(cmd->seq[0][0], "susp") == 0) {
                susp();

            } else {
                // Créer un processus pour exécuter la commande.
                pid_t pid = test(fork(), "8");

                if (pid < 0) {
                    perror("Erreur lors du fork");
                } else if (pid == 0) {

                    if (cmd->in) {
                        // Redirection de l'entrée
                        int fd_in = test(open(cmd->in, O_RDONLY), "9");
                        if (fd_in < 0) {
                            perror("Erreur lors de l'ouverture du fichier d'entrée");
                            exit(1);
                        }
                        test(dup2(fd_in, 0), "10");
                        test(close(fd_in), "11");
                    }
                    if (cmd->out) {
                        // Redirection de la sortie
                        int fd_out = open(cmd->out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd_out < 0) {
                            perror("Erreur lors de l'ouverture du fichier de sortie");
                            exit(1);
                        }
                        test(dup2(fd_out, 1), "12");
                        test(close(fd_out), "13");
                    }

                    for(int i=0; cmd->seq[i]!=NULL; i++){
                        if (cmd->seq[i+1] != NULL) {
                            int fd[2];
                            test(pipe(fd), "14");
                            if (test(fork(), "15") == 0) {
                                test(dup2(fd[1], 1), "16");
                                test(close(fd[0]), "17");
                                test(execvp(cmd->seq[i][0], cmd->seq[i]), "18");
                            } else {
                                test(dup2(fd[0], 0), "19");
                                test(close(fd[1]), "20");
                            }
                        } else {
                            test(execvp(cmd->seq[i][0], cmd->seq[i]), "21");
                        }
                    }

                    if (execvp(cmd->seq[0][0], cmd->seq[0]) < 0) {
                        perror("Erreur lors de l'exécution de la commande");
                        exit(1);
                    }
                } else {
                    // Processus parent
                    int status;
                    // Choix du avant plan ou de l'arrière plan.
                    if (cmd->backgrounded) {
                        printf("Processus %d exécuté en arrière-plan\n", pid);
                        add_PID(pid, Actif, cmd);
                    } else {
                        pid_fg = pid;
                        waitpid(pid, &status, 0);
                        if (WIFEXITED(status) || WIFSIGNALED(status)) {
                            del_PID(pid_fg);
                        }
                        pid_fg = 0;
                    }
                }
            }

        }
        
    }
}


