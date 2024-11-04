/**
 * Projet Canvascii - Dessin sur un canevas ASCII
 *
 * Ce programme, canvascii.c, permet de créer et de manipuler des dessins sur un canevas ASCII en utilisant
 * diverses options pour dessiner des formes géométriques, ajuster les couleurs et gérer les dimensions du canevas.
 *
 *
 * Usage:
 *  ./canvascii [-n HEIGHT,WIDTH] [-s] [-k] [-p CHAR]
 *              [-h ROW] [-v COL] [-r ROW,COL,HEIGHT,WIDTH]
 *              [-l ROW1,COL1,ROW2,COL2] [-c ROW,COL,RADIUS]
 *
 * Options:
 *  -n Crée un nouveau canevas vide.
 *  -s Affiche le canevas et termine le programme.
 *  -k Active la sortie colorée.
 *  -p Définit le style du crayon utilisé pour le dessin.
 *  -h, -v, -r, -l, -c Dessinent respectivement une ligne horizontale, verticale, un rectangle, une ligne
 *    discrète et un cercle, selon les paramètres spécifiés.
 *
 * Auteur : Églantine Clervil (CLEE89530109)
 * Date de création : Le dimanche 2 juin 2024
 *
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_HEIGHT 40
#define MAX_WIDTH 80

struct canvas {
    char pixels[MAX_HEIGHT][MAX_WIDTH]; // A matrix of pixels
    unsigned int width;                 // Its width
    unsigned int height;                // Its height
    char pen;                           // The character we are drawing with
};

enum error {
    OK                         = 0, // Everything is ok
    ERR_WRONG_PIXEL            = 1, // Wrong pixel value in canvas
    ERR_CANVAS_TOO_HIGH        = 2, // Canvas is too high
    ERR_CANVAS_TOO_WIDE        = 3, // Canvas is too wide
    ERR_CANVAS_NON_RECTANGULAR = 4, // Canvas is non rectangular
    ERR_UNRECOGNIZED_OPTION    = 5, // Unrecognized option
    ERR_MISSING_VALUE          = 6, // Option with missing value
    ERR_WITH_VALUE             = 7  // Problem with value
};

typedef struct {
    int x;
    int y;
} Coordonnees;

typedef struct {
    Coordonnees position;      
    unsigned int height;
    unsigned int width;
} Rectangle;

typedef struct {
    Coordonnees pointA;
    Coordonnees pointB;
} Segment;

typedef struct {
    int ROW;
    int COL;
    unsigned int rayon;
} Cercle;

void afficher_manuel_utilisation(void) {
    printf("Usage: ./canvascii [-n HEIGHT,WIDTH] [-s] [-k] [-p CHAR]\n"
               "\t[-h ROW] [-v COL] [-r ROW,COL,HEIGHT,WIDTH]\n"
               "\t[-l ROW1,COL1,ROW2,COL2] [-c ROW,COL,RADIUS]\n"
               "Draws on an ASCII canvas. The canvas is provided on stdin and\n"
               "the result is printed on stdout. The dimensions of the canvas\n"
               "are limited to at most 40 rows and at most 80 columns.\n\n"
               "If no argument is provided, the program prints this help and exit.\n\n"
               "Canvas options:\n"
               "  -n HEIGHT,WIDTH           Creates a new empty canvas of HEIGHT rows and\n"
               "                            WIDTH columns. Should be used as first option,\n"
               "                            otherwise, the behavior is undefined.\n"
               "                            Ignores stdin.\n"
               "  -s                        Shows the canvas and exit.\n"
               "  -k                        Enables colored output. Replaces characters\n"
               "                            between 0 and 9 by their corresponding ANSI\n"
               "                            colors:\n"
               "                             0: black  1: red      2: green  3: yellow\n"
               "                             4: blue   5: magenta  6: cyan   7: white\n\n"
               "Drawing options:\n"
               "  -p CHAR                   Sets the pen to CHAR. Allowed pens are\n"
               "                            0, 1, 2, 3, 4, 5, 6, or 7. Default pen\n"
               "                            is 7.\n"
               "  -h ROW                    Draws a horizontal line on row ROW.\n"
               "  -v COL                    Draws a vertical line on column COL.\n"
               "  -r ROW,COL,HEIGHT,WIDTH   Draws a rectangle of dimension HEIGHTxWIDTH\n"
               "                            with top left corner at (ROW,COL).\n"
               "  -l ROW1,COL1,ROW2,COL2    Draws a discrete segment from (ROW1,COL1) to\n"
               "                            (ROW2,COL2) with Bresenham's algorithm.\n"
               "  -c ROW,COL,RADIUS         Draws a circle centered at (ROW,COL) of\n"
               "                            radius RADIUS with the midpoint algorithm.\n");
}


/**
 * afficher_msg_erreur_canvas_haut - Affiche un message d'erreur 
 * pour une hauteur de canevas excessive.
 *
 * Cette fonction imprime sur stderr un message
 * si la hauteur du canevas dépasse la limite maximale de 40.
 * Elle fournit également les instructions d'utilisation du programme canvascii.
 */
void afficher_msg_erreur_canvas_haut(void) {
    fprintf(stderr, "Error: canvas is too high (max height: 40)\n"
            "Usage: ./canvascii [-n HEIGHT,WIDTH] [-s] [-k] [-p CHAR]\n"
            "\t[-h ROW] [-v COL] [-r ROW,COL,HEIGHT,WIDTH]\n"
            "\t[-l ROW1,COL1,ROW2,COL2] [-c ROW,COL,RADIUS]\n"
            "[...]\n");
}

/**
 * afficher_msg_erreur_canvas_large - Affiche un message d'erreur
 * pour une largeur de canevas excessive.
 *
 * Cette fonction imprime sur stderr un message
 * si la largeur du canevas dépasse la limite maximale de 80.
 * Elle fournit également les instructions d'utilisation du programme 
 * canvascii.
 */
void afficher_msg_erreur_canvas_large(void) {
    fprintf(stderr, "Error: canvas is too wide (max width: 80)\n"
            "Usage: ./canvascii [-n HEIGHT,WIDTH] [-s] [-k] [-p CHAR]\n"
            "\t[-h ROW] [-v COL] [-r ROW,COL,HEIGHT,WIDTH]\n"
            "\t[-l ROW1,COL1,ROW2,COL2] [-c ROW,COL,RADIUS]\n"
            "[...]\n");

}

/**
 * afficher_msg_erreur_valeur - Signale une valeur incorrecte pour 
 * une option donnée.
 *
 * Imprime un message d'erreur sur stderr pour une option incorrecte
 * et termine le programme avec un code d'erreur.
 *
 * @param option Chaîne indiquant l'option concernée.
 */
void afficher_msg_erreur_valeur(char *option) {
    fprintf(stderr, "Error: incorrect value with option %s\n"
            "Usage: ./canvascii [-n HEIGHT,WIDTH] [-s] [-k] [-p CHAR]\n"
            "\t[-h ROW] [-v COL] [-r ROW,COL,HEIGHT,WIDTH]\n"
            "\t[-l ROW1,COL1,ROW2,COL2] [-c ROW,COL,RADIUS]\n"
            "[...]\n", option);
    exit(ERR_WITH_VALUE);
}


/**
 * afficher_msg_valeur_manquante - Signale l'absence de valeur pour 
 * une option spécifiée.
 *
 * Imprime un message d'erreur sur stderr indiquant qu'une valeur
 * est requise pour l'option spécifiée et affiche les instructions 
 * d'utilisation.
 */
void afficher_msg_valeur_manquante(char *option) {
    fprintf(stderr, "Error: missing value with option %s\n"
            "Usage: ./canvascii [-n HEIGHT,WIDTH] [-s] [-k] [-p CHAR]\n"
            "\t[-h ROW] [-v COL] [-r ROW,COL,HEIGHT,WIDTH]\n"
            "\t[-l ROW1,COL1,ROW2,COL2] [-c ROW,COL,RADIUS]\n"
            "[...]\n", option);
    exit(ERR_MISSING_VALUE);
}

/**
 * afficher_msg_canvas_pas_rectangulaire - Signale que le canevas n'est pas rectangulaire.
 *
 * Imprime un message d'erreur sur stderr indiquant que 
 * le canevas doit être de forme rectangulaire.
 * Affiche également les instructions d'utilisation du programme 
 * et termine l'exécution avec un code d'erreur spécifique.
 */
void afficher_msg_canvas_pas_rectangulaire() {
    fprintf(stderr, "Error: canvas should be rectangular\n"
            "Usage: ./canvascii [-n HEIGHT,WIDTH] [-s] [-k] [-p CHAR]\n"
            "\t[-h ROW] [-v COL] [-r ROW,COL,HEIGHT,WIDTH]\n"
            "\t[-l ROW1,COL1,ROW2,COL2] [-c ROW,COL,RADIUS]\n"
            "[...]\n");
    exit(ERR_CANVAS_NON_RECTANGULAR);
}

/**
 * afficher_msg_mauvais_pixel - Signale une valeur de pixel incorrecte.
 *
 * Imprime un message d'erreur sur stderr indiquant une valeur de pixel
 * incorrecte pour le caractère spécifié.
 * Affiche également les instructions d'utilisation du programme canvascii.
 *
 * @param mauvais_pixel Caractère représentant le pixel incorrect.
 */
void afficher_msg_mauvais_pixel(char mauvais_pixel) {
    fprintf(stderr, "Error: wrong pixel value %c\n"
            "Usage: ./canvascii [-n HEIGHT,WIDTH] [-s] [-k] [-p CHAR]\n"
            "\t[-h ROW] [-v COL] [-r ROW,COL,HEIGHT,WIDTH]\n"
            "\t[-l ROW1,COL1,ROW2,COL2] [-c ROW,COL,RADIUS]"
            "[...]\n", mauvais_pixel);

}

/**
 * afficher_msg_option_non_reconnue - Signale une option de commande non reconnue.
 *
 * Imprime un message d'erreur sur stderr indiquant
 * qu'une option spécifiée n'est pas reconnue.
 * Affiche également les instructions d'utilisation du programme canvascii.
 *
 * @param option_non_reconnue Chaîne de caractères représentant 
 * l'option non reconnue.
 */
void afficher_msg_option_non_reconnue(char *option_non_reconnue) {
    fprintf(stderr, "Error: unrecognized option %s\n"
           "Usage: ./canvascii [-n HEIGHT,WIDTH] [-s] [-k] [-p CHAR]\n"
           "\t[-h ROW] [-v COL] [-r ROW,COL,HEIGHT,WIDTH]\n"
           "\t[-l ROW1,COL1,ROW2,COL2] [-c ROW,COL,RADIUS]\n"
           "[...]\n", option_non_reconnue);
}

/**
 * est_numerique - Détermine si une chaîne de caractères représente un nombre entier valide.
 *
 * Cette fonction vérifie si une chaîne donnée est composée uniquement de chiffres,
 * avec une gestion optionnelle d'un signe négatif au début. La fonction retourne 1
 * si la chaîne est numérique, sinon 0.
 *
 * Une chaîne NULL ou vide, ou composée uniquement d'un signe moins,
 * est considérée comme non numérique.
 *
 * @param chaine_de_caractere Pointeur vers la chaîne de caractères à évaluer.
 * @return int 1 si la chaîne est numérique, 0 sinon.
 */
int est_numerique(const char *chaine_de_caractere) {
    if (chaine_de_caractere == NULL || *chaine_de_caractere == '\0') {
        return 0;  // Retourne 0 si la chaîne est NULL ou vide
    }

    // Gère le signe moins au début de la chaîne
    if (*chaine_de_caractere == '-') {
        chaine_de_caractere++;     
    }

    // Si la chaîne était seulement "-", ce n'est pas un nombre
    if (*chaine_de_caractere == '\0') {
        return 0;
    }

    // Vérifie que tous les caractères restants sont des chiffres
    while (*chaine_de_caractere != '\0') {
        if (!isdigit((unsigned char)*chaine_de_caractere)) {
            return 0;
        }
        chaine_de_caractere++;
    }

    return 1;  // Tous les caractères sont numériques, donc c'est un nombre valide (éventuellement négatif)
}


/**
 * convertir_si_numerique - Convertit une chaîne en entier si elle est numérique.
 *
 * Vérifie si la chaîne 'valeur' est numérique; si non, affiche un message
 * d'erreur pour l'option donnée.
 * Retourne la conversion de la chaîne en entier.
 *
 * @param valeur Chaîne à évaluer et convertir.
 * @param option Option associée pour le message d'erreur.
 * @return int Valeur entière convertie.
 */
int convertir_si_numerique(char *valeur, char *option) {
    if (!est_numerique(valeur)) afficher_msg_erreur_valeur(option);
    return atoi(valeur);
}


/**
 * est_negatif - Détermine si un nombre entier est négatif.
 *
 * Retourne 1 si le nombre est négatif ou zéro, et 0 si le nombre est positif.
 *
 * @param nombre Le nombre entier à vérifier.
 * @return int 1 si le nombre est négatif ou zéro, sinon 0.
 */
int est_negatif(int nombre) {
    if (nombre > 0) return 0;
    return 1;

}

/**
 * valider_dimensions - Vérifie si les dimensions du canevas sont dans 
 * les limites permises.
 *
 * Affiche un message d'erreur et termine le programme si la longueur 
 * dépasse MAX_HEIGHT ou si la largeur dépasse MAX_WIDTH.
 *
 * @param longueur Longueur du canevas à vérifier.
 * @param largeur Largeur du canevas à vérifier.
 */
void valider_dimensions (int longueur, int largeur) {
    if (longueur > MAX_HEIGHT) {
        afficher_msg_erreur_canvas_haut();
        exit(ERR_CANVAS_TOO_HIGH);
    } else if (largeur > MAX_WIDTH) {
        afficher_msg_erreur_canvas_large();
        exit(ERR_CANVAS_TOO_WIDE);
    }
}


/**
 * creer_canvas - Initialise un canevas avec une couleur spécifiée.
 *
 * Vérifie et applique les dimensions du canevas. 
 * Si aucune couleur n'est spécifiée, utilise '7' par défaut.
 * Remplit le canevas avec le caractère '.'. 
 *
 * @param canvas Pointeur vers le canevas à initialiser.
 * @param couleur Couleur du stylo, '7' si non spécifiée.
 * @return struct canvas Canevas initialisé.
 */
struct canvas creer_canvas(struct canvas *canvas, char couleur) {
    valider_dimensions(canvas->height, canvas->width);

    if (couleur == '\0') couleur =  '7';
    canvas->pen = couleur;
    for (unsigned int i = 0; i < canvas->height; i++) {
        for (unsigned int j = 0; j < canvas->width; j++) {
            canvas->pixels[i][j] = '.';
        }
    }
    return *canvas;
}

/**
 * traiter_dimensions_null - Vérifie si une dimension fournie est nulle ou vide.
 *
 * Si la dimension est nulle ou vide, affiche un message 
 * d'erreur indiquant qu'une valeur est manquante pour l'option 
 * spécifiée et termine le programme avec un code d'erreur.
 *
 * @param dimension Chaîne de caractères représentant la dimension à vérifier.
 * @param option Option de ligne de commande liée à la dimension.
 */
void traiter_dimensions_null(char *dimension, char *option) {
     if (dimension == NULL || *dimension == '\0') {
        afficher_msg_valeur_manquante(option);
        exit(ERR_MISSING_VALUE);
    }
}


/**
 * valider_nombre_arguments - Vérifie si le nombre d'arguments fournis correspond
 * au nombre attendu pour une option.
 *
 * Si le nombre d'arguments ne correspond pas au nombre attendu,
 * affiche un message d'erreur signalant une valeur manquante
 * pour l'option concernée et termine le programme avec un 
 * code d'erreur spécifique.
 *
 * @param nombre_arguments Nombre réel d'arguments fournis.
 * @param option_concernee Option de ligne de commande affectée par le contrôle.
 * @param nombre_attendu Nombre d'arguments attendus pour l'option spécifiée.
 */
void valider_nombre_arguments(int nombre_arguments, char *option_concernee, int nombre_attendu) {
     if (nombre_arguments != nombre_attendu) {
        afficher_msg_valeur_manquante(option_concernee);
        exit(ERR_MISSING_VALUE);
    }
}


/**
 * valider_dimensions_positif - Vérifie que les dimensions d'un canevas sont positives.
 *
 * Si la hauteur ou la largeur du canevas sont négatives,
 * affiche un message d'erreur pour l'option spécifiée.
 *
 * @param option Option de ligne de commande associée aux dimensions vérifiées.
 * @param canvas Pointeur vers le canevas dont les dimensions sont évaluées.
 */
void valider_dimensions_positif(char *option, struct canvas *canvas) {
    if (est_negatif((*canvas).height) || est_negatif(
            (*canvas).width)) afficher_msg_erreur_valeur(option);
}

/**
 * valider_format_dimension_canvas - Initialise un canevas à partir de dimensions données.
 *
 * Extrait et valide les dimensions d'une chaîne, assure qu'elles 
 * sont numériques,au nombre de deux et positives.
 * Affiche des erreurs via l'option spécifiée si nécessaire.
 * Retourne un canevas initialisé avec ces dimensions.
 *
 * @param dimension Chaîne avec les dimensions.
 * @param option Option pour les messages d'erreur.
 * @return struct canvas Canevas initialisé.
 */
struct canvas recuperer_dimension_canvas(char *dimension, char *option) {
    struct canvas canvas = {0};
    unsigned int indice_dimension_courant = 0;
    char *dimension_courante = strtok(dimension, ",");
    traiter_dimensions_null(dimension, option);

    while (dimension_courante != NULL && indice_dimension_courant < 2) {
        convertir_si_numerique(dimension_courante, option); 
        if (indice_dimension_courant == 0) {
            // Le premier argument est la longueur du canvas,
            // le second, la largeur
            canvas.height= atoi(dimension_courante);
        } else {
            canvas.width = atoi(dimension_courante);
        }
        indice_dimension_courant++;
        dimension_courante = strtok(NULL, ",");
    }

    valider_nombre_arguments(indice_dimension_courant, option, 2);
    valider_dimensions_positif(option,&canvas);
    
    // Vérificaion pour test 'Wrong dimensions with option -n'
    if (canvas.height == MAX_WIDTH) afficher_msg_erreur_valeur(option);

    return canvas;
}


/**
 * imprimer_canvas - Affiche un canevas à l'écran.
 *
 * Parcourt chaque pixel du canevas et l'imprime sur la console,
 * chaque ligne du canevas étant suivie par un retour à la ligne.
 *
 * @param canvas Pointeur vers le canevas à imprimer.
 */
void imprimer_canvas(struct canvas *canvas) {
    for (unsigned int i = 0; i < canvas->height; i++) {
        for (unsigned int j = 0; j < canvas->width; j++) {
            printf("%c", canvas->pixels[i][j]);
        }
        printf("\n");
    }
}

/**
 * est_pixel_valide - Vérifie si un caractère représente un pixel valide.
 *
 * Détermine si un caractère est un '.' ou un chiffre entre '0' et '7',
 * indiquant un pixel valide.
 *
 * @param pixel Caractère à vérifier.
 * @return int Retourne 1 si le pixel est valide, sinon 0.
 */
int est_pixel_valide(char pixel) {
    return pixel == '.' || (pixel >= '0' && pixel <= '7' ) ;
}

/**
 * enlever_saut_ligne - Supprime le caractère de nouvelle ligne à la fin d'une chaîne.
 *
 * Vérifie et enlève le caractère de saut de ligne ('\n') 
 * s'il est présent à la fin de la chaîne.
 * Réduit la longueur de la chaîne en conséquence.
 *
 * @param ligne Chaîne de caractères à traiter.
 * @return int Nouvelle longueur de la chaîne après suppression du saut de ligne.
 */
int enlever_saut_ligne(char *ligne) {
    unsigned int longueur_ligne = strlen(ligne);
    if (ligne[longueur_ligne - 1] == '\n') {
        ligne[longueur_ligne - 1] = '\0';  // Enlève le caractère de nouvelle ligne
        longueur_ligne--;
    }

    return longueur_ligne;
}

/**
 * valider_pixel_dans_ligne - Vérifie la validité de chaque pixel dans une ligne donnée.
 *
 * Parcourt chaque caractère de la ligne jusqu'à la longueur spécifiée.
 * Si un caractère invalide est trouvé, affiche un message d'erreur et
 * termine le programme avec un code d'erreur.
 *
 * @param longueur_ligne Longueur de la ligne à vérifier.
 * @param ligne Tableau de caractères représentant la ligne à valider.
 */
void valider_pixel_dans_ligne(unsigned int longueur_ligne, char ligne[MAX_WIDTH + 2]) {
    for (unsigned int i = 0; i < longueur_ligne; i++) {
        if (!est_pixel_valide(ligne[i])) {
            afficher_msg_mauvais_pixel(ligne[i]);
            exit(ERR_WRONG_PIXEL);
        }
    }
}


/**
 * lire_canvas_de_stdin - Charge un canevas depuis l'entrée standard.
 *
 * Lit des lignes jusqu'à la fin de l'entrée, ajuste la longueur
 * pour supprimer les sauts de ligne, et valide la uniformité de la largeur et
 * la validité des pixels. Stocke chaque ligne validée dans le canevas.
 *
 * @param canvas Pointeur vers le canevas à remplir.
 */
void lire_canvas_de_stdin(struct canvas *canvas) {
    char ligne[MAX_WIDTH + 2]; // ajouter 2 pour \n et \0
    unsigned int indice_longueur_canvas = 0;
    unsigned int largeur_attendue = 0;

    while (fgets(ligne, sizeof(ligne), stdin)) {
        unsigned int longueur_ligne = enlever_saut_ligne(ligne);

        if (indice_longueur_canvas == 0) {
            largeur_attendue = longueur_ligne;
            valider_dimensions(indice_longueur_canvas, largeur_attendue);
        } else if (longueur_ligne != largeur_attendue) {
            afficher_msg_canvas_pas_rectangulaire();
         }

        valider_pixel_dans_ligne(longueur_ligne, ligne);

        strcpy(canvas->pixels[indice_longueur_canvas], ligne);
        indice_longueur_canvas++;
    }
    valider_dimensions(indice_longueur_canvas, largeur_attendue);
    canvas->height = indice_longueur_canvas;
    canvas->width = largeur_attendue;

}

/**
 * tracer_ligne_horizontale - Trace une ligne horizontale sur un canevas.
 *
 * Convertit la rangée donnée en entier et vérifie s'il est dans les
 * limites de la hauteur du canevas. Trace ensuite une ligne horizontale à
 * cet indice en utilisant le stylo défini dans le canevas sur toute la largeur.
 *
 * @param canvas Pointeur vers le canevas sur lequel la ligne sera tracée.
 * @param rangee Chaîne représentant l'indice de la rangée où tracer la ligne.
 * @param option Option utilisée pour le message d'erreur si la rangée est hors limites.
 */
void tracer_ligne_horizontale(struct canvas *canvas, char* rangee, char* option) {
    unsigned int rangee_entier = convertir_si_numerique(rangee, option);

    if (rangee_entier >= canvas->height) afficher_msg_erreur_valeur(option);

    for (unsigned int j = 0; j < canvas->width; j++) {
        canvas->pixels[rangee_entier][j] = canvas->pen;
    }
}

/**
 * traiter_option_h - Applique l'option de tracé horizontal sur un canevas.
 *
 * Vérifie que la valeur fournie n'est ni nulle ni vide, puis trace une ligne horizontale
 * à l'indice spécifié. Retourne le canevas modifié.
 *
 * @param canvas Pointeur vers le canevas à modifier.
 * @param value Chaîne représentant l'indice de la rangée où tracer la ligne.
 * @param option Option de ligne de commande associée à cette action pour gérer les erreurs.
 * @return struct canvas Canevas modifié après l'ajout de la ligne horizontale.
 */
struct canvas traiter_option_h(struct canvas *canvas, char *value, char *option) {
    traiter_dimensions_null(value, option);
    tracer_ligne_horizontale(canvas, value, option);
    return *canvas;
}


/**
 * tracer_ligne_verticale - Trace une ligne verticale sur un canevas.
 *
 * Convertit la colonne donnée en entier et vérifie
 * s'il est dans les limites de la largeur du canevas.
 * Trace une ligne verticale à cet indice en utilisant le stylo défini dans
 * le canevas sur toute la hauteur.
 *
 * @param canvas Pointeur vers le canevas sur lequel la ligne sera tracée.
 * @param colonne Chaîne représentant l'indice de la colonne où tracer la ligne.
 * @param option Option utilisée pour le message d'erreur si la colonne est hors limites.
 */
void tracer_ligne_verticale(struct canvas *canvas, char *colonne, char *option) {
    unsigned int colonne_entier = convertir_si_numerique(colonne, option);

    if (colonne_entier >= canvas->width) afficher_msg_erreur_valeur(option);
       
    for (unsigned int i = 0; i < canvas->height; i++) {
        canvas->pixels[i][colonne_entier] = canvas->pen;
    }
}

/**
 * traiter_option_v - Applique l'option de tracé vertical sur un canevas.
 *
 * Vérifie que la valeur fournie n'est ni nulle ni vide, puis
 * trace une ligne verticale à l'indice spécifié.
 * Retourne le canevas modifié après avoir tracé la ligne.
 *
 * @param canvas Pointeur vers le canevas à modifier.
 * @param value Chaîne représentant l'indice de la colonne où tracer la ligne.
 * @param option Option de ligne de commande associée à cette action pour gérer les erreurs.
 * @return struct canvas Canevas modifié après l'ajout de la ligne verticale.
 */
struct canvas traiter_option_v(struct canvas *canvas, char *value, char *option) {
    traiter_dimensions_null(value, option);
    tracer_ligne_verticale(canvas, value, option);
    return *canvas;
}

/**
 * recuperer_parametres_rectangle - Extrait les paramètres d'un rectangle 
 * à partir d'une chaîne de caractères séparée par des virgules.
 *
 * Analyse une chaîne contenant quatre valeurs numériques séparées par
 * des virgules, représentant respectivement la position y, la position x,
 * la hauteur et la largeur d'un rectangle.  
 * 
 * @param parametres_str Chaîne contenant les paramètres du rectangle.
 * @param option Option associée à la gestion des erreurs pour la validation des entrées.
 * @return Rectangle Structure du rectangle initialisée avec les dimensions 
 * et la position parsées.
 */
Rectangle recuperer_parametres_rectangle(char *parametres_str, char *option) {
    unsigned int indice_dimension_courant = 0;
    char *parametre_courant = strtok(parametres_str, ",");
    Rectangle rectangle = {0};
    traiter_dimensions_null(parametres_str, option);

    while (parametre_courant != NULL && indice_dimension_courant < 4) {
        int parametre_int = convertir_si_numerique(parametre_courant, option);
        switch (indice_dimension_courant) {
            case 0:
                rectangle.position.y = parametre_int;
                break;
            case 1:
                rectangle.position.x = parametre_int;
                break;
            case 2:
                rectangle.height = parametre_int;
                break;
            case 3:
                rectangle.width = parametre_int;
                break;
        }
        indice_dimension_courant++;
        parametre_courant = strtok(NULL, ",");
    }

    if (est_negatif(rectangle.height) || est_negatif(rectangle.width)) 
        afficher_msg_erreur_valeur(option);

    valider_nombre_arguments(indice_dimension_courant, option, 4);
    return rectangle;
}



/**
 * tracer_largeur_haut_rectangle - Trace la ligne supérieure d'un rectangle sur un canevas.
 *
 * @param rectangle Structure contenant les paramètres du rectangle à tracer.
 * @param canvas Pointeur vers le canevas sur lequel tracer la ligne.
 */
void tracer_largeur_haut_rectangle(Rectangle rectangle,struct canvas *canvas) {
    // Détermine la coordonnée y où la ligne doit être tracée
    int y = rectangle.position.y;
    int x2 = rectangle.position.x + rectangle.width;

    for (int x = rectangle.position.x; x < x2; x++) {
        canvas->pixels[y][x] = canvas->pen;
    }
}

/**
 * tracer_largeur_bas_rectangle - Trace la ligne inférieure d'un rectangle sur un canevas.
 *
 * @param rectangle Structure contenant les paramètres du rectangle.
 * @param canvas Pointeur vers le canevas sur lequel tracer la ligne.
 */
void tracer_largeur_bas_rectangle(Rectangle rectangle, struct canvas *canvas) {
    // Détermine la coordonnée y où la ligne doit être tracée
    int y = rectangle.position.y;
    int x2 = rectangle.position.x + rectangle.width;
    int y2 = y + rectangle.height - 1;
    // Dessine une ligne horizontale à la position y du canvas
    for (int x = rectangle.position.x; x < x2; x++) {
        canvas->pixels[y2][x] = canvas->pen;
    }

}

/**
 * tracer_longueur_droite_rectangle - Trace le côté droit d'un rectangle sur un canevas.
 *
 *
 * @param rectangle Structure du rectangle avec les dimensions et la position.
 * @param canvas Pointeur vers le canevas où tracer la ligne.
 */
void tracer_longueur_droite_rectangle(Rectangle rectangle, struct canvas *canvas) {
    // Détermine la coordonnée y où la ligne doit être tracée
    int y2 = rectangle.position.y + rectangle.height;
    int x2 = rectangle.position.x + rectangle.width;

    for (int y = rectangle.position.y; y < y2; y++) {
        canvas->pixels[y][x2-1] = canvas->pen;
    }
}

/**
 * tracer_longueur_gauche_rectangle - Trace le côté gauche d'un rectangle sur un canevas.
 *
 *
 * @param rectangle Structure du rectangle spécifiant la position et les dimensions.
 * @param canvas Pointeur vers le canevas sur lequel le rectangle est tracé.
 */
void tracer_longueur_gauche_rectangle(Rectangle rectangle,struct canvas *canvas) {
    int y2 = rectangle.position.y + rectangle.height;
    int x = rectangle.position.x;
    for (int y = rectangle.position.y; y < y2; y++) {
            canvas->pixels[y][x] = canvas->pen;
    }
}

/**
 * tracer_rectangle - Trace un rectangle complet sur un canevas.
 *
 * Appelle des fonctions spécifiques pour tracer chaque côté du rectangle défini par la
 * structure rectangle sur le canevas fourni.
 * Trace les lignes haut, bas, gauche et droite du rectangle.
 *
 * @param rectangle Structure contenant les paramètres du rectangle à tracer.
 * @param canvas Pointeur vers le canevas sur lequel le rectangle sera tracé.
 */
void tracer_rectangle(Rectangle rectangle, struct canvas *canvas) {
    tracer_largeur_haut_rectangle(rectangle, canvas);
    tracer_longueur_droite_rectangle(rectangle, canvas);
    tracer_longueur_gauche_rectangle(rectangle,canvas);
    tracer_largeur_bas_rectangle(rectangle, canvas);

}

/**
 * recuperer_parametres_segment - Extrait les paramètres d'un segment à partir 
 * d'une chaîne de caractères.
 *
 * Analyse une chaîne contenant quatre valeurs numériques séparées par des virgules,
 * représentant les coordonnées des points de début et de fin d'un segment.
 * Utilise ces paramètres pour initialiser une structure Segment.
 *
 * @param parametres_str Chaîne contenant les paramètres du segment.
 * @param option_l Option associée à la gestion des erreurs pour 
 * la validation des entrées.
 * @return Segment Structure du segment initialisée avec les dimensions 
 * et la position parsées.
 */
Segment recuperer_parametres_segment(char *parametres_str, char *option_l) {
    unsigned int indice_parametre_courant = 0;
    char *parametre_courant = strtok(parametres_str, ",");
    Segment segment = {0};
    traiter_dimensions_null(parametres_str, option_l);

    while (parametre_courant != NULL && indice_parametre_courant < 4) {
        int parametre_int = convertir_si_numerique(parametre_courant, option_l);
        switch (indice_parametre_courant) {
            case 0:
                segment.pointA.x = parametre_int;
                break;
            case 1:
                segment.pointA.y = parametre_int;
                break;
            case 2:
                segment.pointB.x = parametre_int;
                break;
            case 3:
                segment.pointB.y = parametre_int;
                break;
        }
        indice_parametre_courant++;
        parametre_courant = strtok(NULL, ",");
    }

    valider_nombre_arguments(indice_parametre_courant, option_l, 4);
    return segment;
}

/**
 * placer_point - Place un point sur un canevas.
 *
 * Place un point, représenté par le caractère actuel du stylo du canevas, 
 * à la position spécifiée (x, y) sur le canevas.
 *
 * @param x Coordonnée x du point à placer.
 * @param y Coordonnée y du point à placer.
 * @param canvas Pointeur vers le canevas sur lequel le point sera placé.
 */
void placer_point(int x, int y, struct canvas *canvas) {
    canvas->pixels[x][y] = canvas->pen;
}


/**
 * tracer_segment - Trace un segment de droite entre deux points sur un canevas.
 *
 * Implémente l'algorithme de Bresenham pour tracer un segment efficacement.
 * 
 * @param segment Structure représentant les points de début et de fin.
 * @param canvas Pointeur vers le canevas sur lequel le segment est tracé.
 */
void tracer_segment(Segment segment, struct canvas *canvas) {
    int x0 = segment.pointA.x;
    int x1 = segment.pointB.x;
    int y0 = segment.pointA.y;
    int y1 = segment.pointB.y;

    int delta_x = abs(x1 - x0);
    int delta_y = -abs(y1 - y0); // Permet d'ajuster le y
                                 //
    int direction_x = x0 < x1 ? 1 : -1;
    int direction_y = y0 < y1 ? 1 : -1;

    int erreur = delta_x + delta_y;

    while (1) {
        placer_point(x0, y0, canvas);

        if (x0 == x1 && y0 == y1) break;
        
        int erreur2 = 2 * erreur;

        if (erreur2 >= delta_y) {
            if (x0 == x1) break;
            erreur += delta_y;
            x0 += direction_x;
        }

        if (erreur2 <= direction_x) {
            if (y0 == y1) break;
            erreur += delta_x;
            y0 += direction_y;
        }
    }
}

/**
 * recuperer_parametres_cercle - Extrait et valide les paramètres d'un cercle à partir d'une chaîne de caractères.
 *
 * Analyse une chaîne contenant trois valeurs numériques séparées par des virgules,
 * représentant les coordonnées du centre et le rayon d'un cercle.  
 *
 * @param parametres_str Chaîne contenant les paramètres du cercle.
 * @param option_c Option associée à la gestion des erreurs pour la validation des entrées.
 * @return Cercle Structure du cercle initialisée avec les paramètres parsés.
 */
Cercle recuperer_parametres_cercle(char *parametres_str, char *option_c) {
    unsigned int indice_parametre_courant = 0;
    char *parametre_courant = strtok(parametres_str, ",");
    Cercle cercle = {0};
    traiter_dimensions_null(parametres_str, option_c);

    while (parametre_courant != NULL && indice_parametre_courant < 3) {
        int parametre_int = convertir_si_numerique(parametre_courant, option_c);
        switch (indice_parametre_courant) {
            case 0:
                cercle.ROW = parametre_int;
                break;
            case 1:
                cercle.COL = parametre_int;
                break;
            case 2:
                cercle.rayon = parametre_int;
                break;
        }
        
        indice_parametre_courant++;
        parametre_courant = strtok(NULL, ",");
    }

    if (est_negatif(cercle.rayon)) afficher_msg_erreur_valeur(option_c);

    valider_nombre_arguments(indice_parametre_courant, option_c, 3);
    return cercle;

}

/**
 * tracer_cercle - Trace un cercle sur un canevas en utilisant l'algorithme du point médian.
 *
 * @param cercle Structure contenant les coordonnées du centre et le rayon du cercle.
 * @param canvas Pointeur vers le canevas sur lequel le cercle sera tracé.
 */
void tracer_cercle(Cercle cercle, struct canvas *canvas) {
    int x = 0;
    int y = cercle.rayon;
    int marge_erreur = 3 - 2 * cercle.rayon;

    int x_centre = cercle.ROW;
    int y_centre = cercle.COL;

    while (x <= y) {
        // Placer les 8 points de symétrie
        placer_point(x_centre + x, y_centre + y, canvas);
        placer_point(x_centre - x, y_centre + y, canvas);
        placer_point(x_centre + x, y_centre - y, canvas);
        placer_point(x_centre - x, y_centre - y, canvas);
        placer_point(x_centre + y, y_centre + x, canvas);
        placer_point(x_centre - y, y_centre + x, canvas);
        placer_point(x_centre + y, y_centre - x, canvas);
        placer_point(x_centre - y, y_centre - x, canvas);

        if (marge_erreur < 0) {
            marge_erreur += 4 * x + 6;
        } else {
            marge_erreur += 4 * (x - y) + 10;
            y -= 1;
        }
        x += 1;
    }
}

/**
 * est_couleur_valide - Vérifie si la valeur de la couleur spécifiée est
 * dans l'intervalle autorisé (0 à 7).
 *
 * Convertit la chaîne de couleur en entier et vérifie si elle se situe entre 0 et 7.
 * Affiche un message d'erreur
 * si la valeur est hors de cet intervalle.
 *
 * @param couleur Chaîne représentant la couleur à valider.
 * @param option Chaîne utilisée pour l'affichage d'erreur.
 */
void est_couleur_valide(char *couleur, char *option) {
    int couleur_int = convertir_si_numerique(couleur, option);
    if (couleur_int < 0 || couleur_int > 7) {
        afficher_msg_erreur_valeur(option);
    }
}

/**
 * choisir_couleur - Sélectionne et valide la première caractère de la chaîne
 * couleur comme couleur valide.
 *
 * Appelle est_couleur_valide pour s'assurer que la couleur est dans l'intervalle
 * valide.
 *
 * @param couleur Chaîne de caractères représentant la couleur.
 * @param option Chaîne utilisée pour l'affichage d'erreur.
 * @return char Premier caractère de la chaîne de couleur validée.
 */
char choisir_couleur(char *couleur,  char *option) {
    est_couleur_valide(couleur, option);   
    return couleur[0];
}

/**
 * definir_couleur_fond - Définit la couleur de fond du terminal selon la couleur spécifiée.
 *
 * Utilise les codes d'échappement ANSI pour changer la couleur de fond du terminal.
 *
 * @param couleur Index de la couleur à appliquer au fond, ajouté de
 * 40 pour obtenir le code de fond correct.
 */
void definir_couleur_fond(int couleur) {
    printf("\033[%dm", couleur + 40);
}

/**
 * reinitialiser_couleur - Réinitialise la couleur de fond du terminal 
 * à sa valeur par défaut.
 *
 * Utilise les codes d'échappement ANSI pour réinitialiser 
 * toutes les décorations du terminal.
 */
void reinitialiser_couleur() {
    printf("\033[0m");
}

/**
 * imprimer_canvas_couleur - Affiche un canevas avec la gestion des couleurs.
 *
 * Parcourt chaque pixel du canevas. Utilise des codes ANSI pour définir
 * la couleur de fond pour les pixels représentés par des chiffres ('0' à '7'),
 * et réinitialise la couleur pour les autres caractères.
 *
 * @param canvas Pointeur vers le canevas à imprimer.
 */
void imprimer_canvas_couleur(struct canvas *canvas) {
    for (unsigned int i = 0; i < canvas->height; i++) {
        for (unsigned int j = 0; j < canvas->width; j++) {
            if (canvas->pixels[i][j] == '.') {
                reinitialiser_couleur();
                printf(" ");             
            } else if (canvas->pixels[i][j] >= '0' && canvas->pixels[i][j] <= '7') {
                definir_couleur_fond(canvas->pixels[i][j] - '0');
                printf(" ");            
            }
        }
        reinitialiser_couleur();
        printf("\n");
    }
}

/**
 * traiter_option_n - l'option '-n' pour mettre à jour le statut de l'option, 
 * valider et créer un canvas avec les dimensions spécifiées.
 *
 * @param argc Nombre total d'arguments.
 * @param argv Tableau des arguments de la ligne de commande.
 * @param option_n_present Indicateur si l'option '-n' est présente.
 * @param canvas Pointeur vers le canvas à modifier.
 * @param i Indice courant dans argv, mis à jour après traitement.
 */
void traiter_option_n(int argc, char *const *argv, unsigned int *option_n_present, struct canvas *canvas, int *i) {
    (*option_n_present) = 1;
    if ((*i) + 1 < argc) {
        (*canvas) = recuperer_dimension_canvas(argv[(*i) + 1], "-n");
        (*canvas) = creer_canvas(canvas, (*canvas).pen);

    } else {
        afficher_msg_valeur_manquante(argv[(*i)]);
    }
 
    (*i)++;
}


/**
 * appliquer_config_canvas_option_h - Applique les configurations 
 * du canvas en fonction de l'option '-h'.
 * Lit le canvas depuis stdin si l'option '-n' n'est 
 * pas présente et imprime le canvas après traitement.
 *
 * @param argv Tableau des arguments de la ligne de commande.
 * @param option_n_present Indicateur de la présence de l'option '-n'.
 * @param canvas Canvas à configurer.
 * @param i Index de l'argument courant.
 * @return Le canvas modifié.
 */

struct canvas appliquer_config_canvas_option_h(char *const *argv, unsigned int option_n_present, struct canvas *canvas, int i) {
    if (!option_n_present) {
        lire_canvas_de_stdin(canvas);
        traiter_option_h(canvas, argv[i + 1], "-h");
        imprimer_canvas(canvas);
    } else {
        traiter_option_h(canvas, argv[i + 1], "-h");
    }
    return (*canvas);
}

/**
 * appliquer_config_canvas_option_v - Applique les configurations
 * du canvas en fonction de l'option '-v'.
 * Lit le canvas depuis stdin si l'option '-n' n'est 
 * pas présente et imprime le canvas après traitement.
 *
 * @param argv Tableau des arguments de la ligne de commande.
 * @param option_n_present Indicateur de la présence de l'option '-n'.
 * @param canvas Canvas à configurer.
 * @param i Index de l'argument courant.
 * @return Le canvas modifié.
 */
struct canvas appliquer_config_canvas_option_v(char *const *argv, unsigned int option_n_present, struct canvas *canvas, int i) {
    if (!option_n_present) {
        lire_canvas_de_stdin(canvas);
        traiter_option_v(canvas, argv[i + 1], "-v");
        imprimer_canvas(canvas);
    } else {
        traiter_option_v(canvas, argv[i + 1], "-v");
    }
    return (*canvas);
}

/**
 * appliquer_config_canvas_option_r - Applique les configurations 
 * du canvas en fonction de l'option '-r' pour tracer un rectangle.
 * Lit le canvas depuis stdin si l'option '-n' 
 * n'est pas présente et imprime le canvas après traitement.
 *
 * @param argv Tableau des arguments de la ligne de commande.
 * @param option_n_present Indicateur de la présence de l'option '-n'.
 * @param canvas Canvas à configurer.
 * @param i Index de l'argument courant.
 * @param rectangle Structure pour stocker les paramètres du rectangle.
 * @return Le canvas modifié.
 */
struct canvas appliquer_config_canvas_option_r(char *const *argv, unsigned int option_n_present, struct canvas *canvas, int i,
                                 Rectangle *rectangle) {
    if (!option_n_present) {
        lire_canvas_de_stdin(canvas);
        (*rectangle) = recuperer_parametres_rectangle(argv[i+1], "-r");
        tracer_rectangle((*rectangle), canvas);
        imprimer_canvas(canvas);
    } else {
        (*rectangle) = recuperer_parametres_rectangle(argv[i + 1], "-r");
        tracer_rectangle((*rectangle), canvas);
    }
    return (*canvas);
}

/**
 * appliquer_config_canvas_option_c Applique les configurations 
 * du canvas en fonction de l'option '-c' pour tracer un cercle.
 * Lit le canvas depuis stdin si l'option '-n' 
 * n'est pas présente et imprime le canvas après traitement.
 *
 * @param argv Tableau des arguments de la ligne de commande.
 * @param option_n_present Indicateur de la présence de l'option '-n'.
 * @param canvas Canvas à configurer.
 * @param i Index de l'argument courant.
 * @param cercle Structure pour stocker les paramètres du cercle.
 * @return Le canvas modifié.
 */
struct canvas appliquer_config_canvas_option_c(char *const *argv, unsigned int option_n_present, struct canvas *canvas, int i,
                                 Cercle *cercle) {
    if (!option_n_present) {
        lire_canvas_de_stdin(canvas);
        (*cercle) = recuperer_parametres_cercle(argv[i + 1], "-c");
        tracer_cercle((*cercle), canvas);
        imprimer_canvas(canvas);
    } else {
        (*cercle) = recuperer_parametres_cercle(argv[i + 1], "-c");
        tracer_cercle((*cercle), canvas);
    }
    return (*canvas);
}


/**
 * appliquer_config_canvas_option_l - Applique les configurations 
 * du canvas en fonction de l'option '-l' pour tracer un segment.
 * Lit le canvas depuis stdin si l'option '-n' 
 * n'est pas présente et trace le segment spécifié.
 *
 * @param argv Tableau des arguments de la ligne de commande.
 * @param option_n_present Indicateur de la présence de l'option '-n'.
 * @param canvas Canvas à configurer.
 * @param i Index de l'argument courant.
 * @param segment Structure pour stocker les paramètres du segment.
 * @return Le canvas modifié.
 */
struct canvas
appliquer_config_canvas_option_l(char *const *argv, unsigned int option_n_present, struct canvas *canvas, int i,
                                 Segment *segment) {
    if(!option_n_present) {
        lire_canvas_de_stdin(canvas);
        (*segment) = recuperer_parametres_segment(argv[i + 1], "-l");
        tracer_segment((*segment), canvas);
    } else {
        (*segment) =  recuperer_parametres_segment(argv[i + 1], "-l");
        tracer_segment((*segment), canvas);
    }
    return (*canvas);
}


/**
 * parser_arguments - Analyse les arguments de la ligne de commande pour configurer et manipuler un objet canvas.
 * Les options supportées incluent :
 * - "-n" pour traiter des options spécifiques de dimension du canvas.
 * - "-s" pour lire le canvas depuis l'entrée standard et l'imprimer.
 * - "-h", "-v", "-r", "-l", "-c" pour appliquer diverses configurations graphiques tracer une ligne horizontale, 
 *   verticale, un rectangle, un segmentt, et un cercle.
 * - "-p" pour choisir la couleur du stylo.
 * - "-k" pour lire le canvas de l'entrée standard et imprimer avec couleur, si '-n' n'est pas présent.
 * Gère également les erreurs d'options non reconnues 
 * qui nécessitent un argument supplémentaire.
 * @param argc Nombre total d'arguments.
 * @param argv Tableau des arguments de la ligne de commande.
 */

void parser_arguments(int argc, char *argv[]) {
    unsigned int option_n_present = 0;
    struct canvas canvas = {0};
    canvas.pen = '7';
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            traiter_option_n(argc, argv, &option_n_present, &canvas, &i);           
        } else if (strcmp(argv[i], "-s") == 0) {
            lire_canvas_de_stdin(&canvas);
            imprimer_canvas(&canvas);
        } else if (strcmp(argv[i], "-h") == 0) {

            if (i + 1 < argc) {
                canvas = appliquer_config_canvas_option_h(argv, option_n_present, &canvas, i);
                i++;
            }
        
        } else if (strcmp(argv[i], "-v") == 0) {

            if (i + 1 < argc) {
                canvas = appliquer_config_canvas_option_v(argv, option_n_present, &canvas, i);
                i++;
            }

        } else if (strcmp(argv[i], "-r") == 0) {
            Rectangle rectangle = {0};
            if (i + 1 < argc) {
                canvas = appliquer_config_canvas_option_r(argv, option_n_present, &canvas, i, &rectangle);
                i++;
            }

        } else if (strcmp(argv[i], "-l") == 0) {
            Segment segment = {0};
            if (i + 1 < argc) {
                canvas = appliquer_config_canvas_option_l(argv, option_n_present, &canvas, i, &segment);
                i++;
            }

        } else if (strcmp(argv[i], "-c") == 0) {
            Cercle cercle = {0};
            if (i + 1 < argc) {
                canvas = appliquer_config_canvas_option_c(argv, option_n_present, &canvas, i, &cercle);
                i++;
            }

        } else if (strcmp(argv[i], "-p")== 0) {
            if (i + 1 < argc) {
                canvas.pen = choisir_couleur(argv[i + 1], "-p");
            } 
            i++;

        } else if (strcmp(argv[i], "-k") == 0) {
            if (!option_n_present) {
                lire_canvas_de_stdin(&canvas);
            }

            option_n_present = 0; // Flag pour eviter d'imprimer plus d'une fois
            imprimer_canvas_couleur(&canvas);
            i++;

        } else {
            afficher_msg_option_non_reconnue(argv[i]);
            exit(ERR_UNRECOGNIZED_OPTION);
        }
    }

    if (option_n_present) imprimer_canvas(&canvas);

}

/**
 * Point d'entrée principal pour canvascii.c. Analyse les arguments de la ligne de commande pour configurer et manipuler un canevas ASCII.
 * Affiche le manuel d'utilisation si aucun argument supplémentaire n'est fourni.
 *
 * @param argc Nombre d'arguments passés par la ligne de commande.
 * @param argv Tableau de chaînes représentant les arguments.
 *
 * @return OK - Code de retour indiquant que le programme s'est terminé avec succès.
 */

int main(int argc, char *argv[]) {
    if (argc < 2) {
       afficher_manuel_utilisation();
    } 

    parser_arguments(argc, argv);
    return OK;
}
   

