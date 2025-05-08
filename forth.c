#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dlfcn.h>

// Déclare les fonctions primitives
extern void forth_dup(int64_t *stack, int *sp);
extern void forth_swap(int64_t *stack, int *sp);
extern void forth_add(int64_t *stack, int *sp);
extern int64_t forth_dot(int64_t *stack, int *sp);
extern void forth_drop(int64_t *stack, int *sp);
extern void forth_sub(int64_t *stack, int *sp);
extern void forth_rot(int64_t *stack, int *sp);
extern void forth_mult(int64_t *stack, int *sp);
extern void forth_div(int64_t *stack, int *sp);
extern void forth_do(int64_t *stack, int *sp, int64_t *control_stack, int *csp);
extern void forth_loop(int64_t *stack, int *sp, int64_t *control_stack, int *csp);
extern void forth_i(int64_t *stack, int *sp, int64_t *control_stack, int *csp);

// Simule la pile Forth
int64_t stack[256];
int sp = 0; // Pointeur de pile (0 pour pile vide)

// Simule la pile de contrôle pour DO LOOP
int64_t control_stack[64]; // Stocke limit et index (2 int64_t par boucle)
int csp = 0; // Pointeur de pile de contrôle (0 pour pile vide)

// Pousse une valeur sur la pile
void push(int64_t value) {
    if (sp >= 256) {
        printf("Erreur : pile pleine !\n");
        return;
    }
    stack[sp] = value;
    sp++;
}

// Affiche la pile
void print_stack(void) {
    printf("Pile (sp=%d): ", sp);
    if (sp > 0) {
        for (int i = 0; i < sp; i++) {
            printf("%" PRId64 " ", stack[i]);
        }
        printf("\n");
    } else {
        printf("vide\n");
    }
}

// Fonction pour afficher la valeur au sommet
void print_top(int64_t value) {
    printf("%lld\n", (long long)value);
}

// Structure pour le dictionnaire des primitives
typedef struct {
    const char *name;
    union {
        void (*basic_func)(int64_t *, int *); // Pour primitives sans control_stack
        int64_t (*dot_func)(int64_t *, int *); // Pour DOT
        void (*loop_func)(int64_t *, int *, int64_t *, int *); // Pour DO, LOOP, I
    } func;
    int is_loop_primitive; // 1 pour DO, LOOP, I; 0 sinon
} Word;

// Structure pour les mots définis par l'utilisateur
#define MAX_WORDS 100
#define MAX_WORD_LEN 32
#define MAX_TOKENS 32
typedef struct {
    char name[MAX_WORD_LEN];
    struct {
        int is_number; // 1 si c'est un nombre, 0 si c'est une primitive, 2 si c'est un mot utilisateur, 3 si DO, 4 si LOOP
        union {
            void (*func)(int64_t *, int *, int64_t *, int *); // Fonction de la primitive ou mot utilisateur
            int64_t value; // Valeur du nombre
            char func_name[MAX_WORD_LEN]; // Nom du mot utilisateur
            int loop_label; // Label pour DO LOOP
        } data;
    } tokens[MAX_TOKENS];
    int token_count;
    void (*compiled_func)(int64_t *, int *, int64_t *, int *); // Pointeur vers la fonction compilée
    void *dl_handle; // Handle pour dlclose
} UserWord;

UserWord user_dictionary[MAX_WORDS];
int user_word_count = 0;

// Dictionnaire des primitives
Word dictionary[] = {
    {"DUP", { .basic_func = forth_dup }, 0},
    {"SWAP", { .basic_func = forth_swap }, 0},
    {"ADD", { .basic_func = forth_add }, 0},
    {"+", { .basic_func = forth_add }, 0},
    {".", { .dot_func = forth_dot }, 0},
    {"DROP", { .basic_func = forth_drop }, 0},
    {"SUB", { .basic_func = forth_sub }, 0},
    {"-", { .basic_func = forth_sub }, 0},
    {"ROT", { .basic_func = forth_rot }, 0},
    {"MULT", { .basic_func = forth_mult }, 0},
    {"*", { .basic_func = forth_mult }, 0},
    {"DIV", { .basic_func = forth_div }, 0},
    {"/", { .basic_func = forth_div }, 0},
    {"DO", { .loop_func = forth_do }, 1},
    {"LOOP", { .loop_func = forth_loop }, 1},
    {"I", { .loop_func = forth_i }, 1},
    {NULL, { .basic_func = NULL }, 0} // Sentinelle
};

// Comparaison insensible à la casse
int strcasecmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
            return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
        }
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

// Cherche un mot dans le dictionnaire des primitives
void *lookup_primitive(const char *name, int *is_loop_primitive) {
    if (!name) return NULL;
    for (int i = 0; dictionary[i].name != NULL; i++) {
        if (strcasecmp(name, dictionary[i].name) == 0) {
            *is_loop_primitive = dictionary[i].is_loop_primitive;
            return dictionary[i].is_loop_primitive ? (void *)dictionary[i].func.loop_func : (void *)dictionary[i].func.basic_func;
        }
    }
    return NULL;
}

// Cherche un mot dans le dictionnaire utilisateur
UserWord *lookup_user(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < user_word_count; i++) {
        if (strcasecmp(name, user_dictionary[i].name) == 0) {
            return &user_dictionary[i];
        }
    }
    return NULL;
}

// Génère un fichier .c pour un mot défini et le compile en .so
void generate_asm(UserWord *word) {
    if (!word || !word->name[0]) {
        printf("Erreur : mot invalide ou nom vide\n");
        return;
    }

    char filename[64], obj[64], so[64];
    snprintf(filename, sizeof(filename), "%s.c", word->name);
    snprintf(obj, sizeof(obj), "%s.o", word->name);
    snprintf(so, sizeof(so), "./%s.so", word->name);

    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Erreur : impossible d'ouvrir %s pour écriture\n", filename);
        return;
    }

    fprintf(f, "#include <inttypes.h>\n\n");
    fprintf(f, "void forth_dup(int64_t *stack, int *sp);\n");
    fprintf(f, "void forth_swap(int64_t *stack, int *sp);\n");
    fprintf(f, "void forth_add(int64_t *stack, int *sp);\n");
    fprintf(f, "int64_t forth_dot(int64_t *stack, int *sp);\n");
    fprintf(f, "void forth_drop(int64_t *stack, int *sp);\n");
    fprintf(f, "void forth_sub(int64_t *stack, int *sp);\n");
    fprintf(f, "void forth_rot(int64_t *stack, int *sp);\n");
    fprintf(f, "void forth_mult(int64_t *stack, int *sp);\n");
    fprintf(f, "void forth_div(int64_t *stack, int *sp);\n");
    fprintf(f, "void forth_do(int64_t *stack, int *sp, int64_t *control_stack, int *csp);\n");
    fprintf(f, "void forth_loop(int64_t *stack, int *sp, int64_t *control_stack, int *csp);\n");
    fprintf(f, "void forth_i(int64_t *stack, int *sp, int64_t *control_stack, int *csp);\n");
    for (int i = 0; i < word->token_count; i++) {
        if (word->tokens[i].is_number == 2) {
            fprintf(f, "void %s(int64_t *stack, int *sp, int64_t *control_stack, int *csp);\n", word->tokens[i].data.func_name);
        }
    }
    fprintf(f, "\n");
    fprintf(f, "void %s(int64_t *stack, int *sp, int64_t *control_stack, int *csp) {\n", word->name);
    fprintf(f, "    asm volatile (\n");
    fprintf(f, "        \".intel_syntax noprefix\\n\"\n");
    // Sauvegarde des registres callee-saved
    fprintf(f, "        \"push rbp\\n\"\n");
    fprintf(f, "        \"push rbx\\n\"\n");
    fprintf(f, "        \"push r12\\n\"\n");
    fprintf(f, "        \"push r13\\n\"\n");
    fprintf(f, "        \"push r14\\n\"\n");
    fprintf(f, "        \"push r15\\n\"\n");

    int end_label_counter = 0; // Compteur pour étiquettes .endX uniques
    for (int i = 0; i < word->token_count; i++) {
        if (word->tokens[i].is_number == 1) {
            fprintf(f, "        \"mov rbx, [rsi]\\n\"\n");
            fprintf(f, "        \"cmp rbx, 256\\n\"\n");
            fprintf(f, "        \"jae .end%d\\n\"\n", end_label_counter);
            fprintf(f, "        \"mov rax, %" PRId64 "\\n\"\n", word->tokens[i].data.value);
            fprintf(f, "        \"mov [rdi + rbx*8], rax\\n\"\n");
            fprintf(f, "        \"inc rbx\\n\"\n");
            fprintf(f, "        \"mov [rsi], rbx\\n\"\n");
            fprintf(f, "        \".end%d:\\n\"\n", end_label_counter);
            end_label_counter++;
        } else if (word->tokens[i].is_number == 0) {
            char *prim_name = NULL;
            int is_loop_primitive = 0;
            for (int j = 0; dictionary[j].name; j++) {
                if ((dictionary[j].is_loop_primitive == 0 && 
                     (dictionary[j].func.basic_func == (void (*)(int64_t *, int *))word->tokens[i].data.func ||
                      dictionary[j].func.dot_func == (int64_t (*)(int64_t *, int *))word->tokens[i].data.func)) ||
                    (dictionary[j].is_loop_primitive == 1 &&
                     dictionary[j].func.loop_func == word->tokens[i].data.func)) {
                    prim_name = (char *)dictionary[j].name;
                    is_loop_primitive = dictionary[j].is_loop_primitive;
                    break;
                }
            }
            if (prim_name) {
                if (strcmp(prim_name, ".") == 0) {
                    fprintf(f, "        \"call forth_dot\\n\"\n");
                } else {
                    char *lower_prim_name = strdup(prim_name);
                    if (!lower_prim_name) {
                        printf("Erreur : échec de l'allocation mémoire pour %s\n", prim_name);
                        fclose(f);
                        return;
                    }
                    for (int k = 0; lower_prim_name[k]; k++) {
                        lower_prim_name[k] = tolower(lower_prim_name[k]);
                    }
                    fprintf(f, "        \"call forth_%s\\n\"\n", lower_prim_name);
                    free(lower_prim_name);
                }
            } else {
                printf("Erreur : primitive inconnue dans %s\n", word->name);
                fclose(f);
                return;
            }
        } else if (word->tokens[i].is_number == 2) {
            // Sauvegarde des registres d'arguments avant l'appel
            fprintf(f, "        \"push rdi\\n\"\n");
            fprintf(f, "        \"push rsi\\n\"\n");
            fprintf(f, "        \"push rcx\\n\"\n");
            fprintf(f, "        \"push rdx\\n\"\n");
            // Configurer les arguments pour l'appel
            fprintf(f, "        \"mov rdi, %p\\n\"\n", stack);
            fprintf(f, "        \"mov rsi, %p\\n\"\n", &sp);
            fprintf(f, "        \"mov rcx, %p\\n\"\n", control_stack);
            fprintf(f, "        \"mov rdx, %p\\n\"\n", &csp);
            fprintf(f, "        \"call %s\\n\"\n", word->tokens[i].data.func_name);
            // Restaurer les registres après l'appel
            fprintf(f, "        \"pop rdx\\n\"\n");
            fprintf(f, "        \"pop rcx\\n\"\n");
            fprintf(f, "        \"pop rsi\\n\"\n");
            fprintf(f, "        \"pop rdi\\n\"\n");
        } else if (word->tokens[i].is_number == 3) {
            fprintf(f, "        \"call forth_do\\n\"\n");
            fprintf(f, "        \".loop_start_%d:\\n\"\n", word->tokens[i].data.loop_label);
        } else if (word->tokens[i].is_number == 4) {
            fprintf(f, "        \"call forth_loop\\n\"\n");
            fprintf(f, "        \"cmp rax, 1\\n\"\n");
            fprintf(f, "        \"je .loop_start_%d\\n\"\n", word->tokens[i].data.loop_label);
            fprintf(f, "        \".loop_end_%d:\\n\"\n", word->tokens[i].data.loop_label);
        }
    }
    // Restauration des registres callee-saved
    fprintf(f, "        \"pop r15\\n\"\n");
    fprintf(f, "        \"pop r14\\n\"\n");
    fprintf(f, "        \"pop r13\\n\"\n");
    fprintf(f, "        \"pop r12\\n\"\n");
    fprintf(f, "        \"pop rbx\\n\"\n");
    fprintf(f, "        \"pop rbp\\n\"\n");
    fprintf(f, "        \".att_syntax\\n\"\n");
    fprintf(f, "        : \"+S\"(sp), \"+d\"(csp)\n");
    fprintf(f, "        : \"D\"(stack), \"c\"(control_stack)\n");
    fprintf(f, "        : \"rax\", \"r8\", \"r9\", \"r10\", \"r11\", \"memory\"\n");
    fprintf(f, "    );\n");
    fprintf(f, "}\n");

    fclose(f);

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "gcc -c -fPIC %s -o %s", filename, obj);
    if (system(cmd) != 0) {
        printf("Erreur : échec de la compilation de %s\n", filename);
        return;
    }

    char link_cmd[1024] = "";
    snprintf(link_cmd, sizeof(link_cmd), "gcc -shared -fPIC %s -L. -lforth_primitives", obj);
    for (int i = 0; i < word->token_count; i++) {
        if (word->tokens[i].is_number == 2) {
            char dep_so[64];
            snprintf(dep_so, sizeof(dep_so), "./%s.so", word->tokens[i].data.func_name);
            strncat(link_cmd, " ", sizeof(link_cmd) - strlen(link_cmd) - 1);
            strncat(link_cmd, dep_so, sizeof(link_cmd) - strlen(link_cmd) - 1);
        }
    }
    snprintf(cmd, sizeof(cmd), "%s -o %s", link_cmd, so);
    if (system(cmd) != 0) {
        printf("Erreur : échec de la liaison de %s\n", so);
        return;
    }

    dlerror();
    void *handle = dlopen(so, RTLD_LAZY);
    if (!handle) {
        printf("Erreur dlopen : %s\n", dlerror());
        return;
    }
    dlerror();
    void *func = dlsym(handle, word->name);
    if (!func) {
        printf("Erreur dlsym : %s\n", dlerror());
        dlclose(handle);
        return;
    }
    word->compiled_func = func;
    word->dl_handle = handle;
}

int main(void) {
    char input[256];
    char *token;
    int compiling = 0;
    UserWord *current_word = NULL;
    int loop_labels[32]; // Tableau pour suivre les étiquettes des boucles imbriquées
    int loop_depth = 0;  // Profondeur d'imbrication des boucles

    // Initialiser la pile à vide au démarrage
    sp = 0;
    csp = 0;

    printf("Forth interpreter (tapez 'quit' pour quitter)\n");
    while (1) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "quit") == 0) {
            break;
        }

        token = strtok(input, " ");
        int error_occurred = 0;
        while (token != NULL && !error_occurred) {
            if (compiling) {
                if (strcmp(token, ";") == 0) {
                    if (!current_word) {
                        printf("Erreur : aucun mot en cours de compilation\n");
                        compiling = 0;
                        continue;
                    }
                    generate_asm(current_word);
                    if (!current_word->compiled_func) {
                        printf("Erreur : échec de la compilation du mot %s\n", current_word->name);
                        memset(current_word, 0, sizeof(UserWord));
                    } else {
                        if (!lookup_user(current_word->name)) {
                            user_word_count++;
                        }
                        for (int i = 0; i < user_word_count; i++) {
                            printf("  %s (compiled_func=%p)\n", user_dictionary[i].name, user_dictionary[i].compiled_func);
                        }
                    }
                    compiling = 0;
                    current_word = NULL;
                    loop_depth = 0; // Réinitialise la profondeur des boucles
                } else {
                    if (!current_word) {
                        printf("Erreur : aucun mot en cours de compilation\n");
                        continue;
                    }
                    if (current_word->token_count < MAX_TOKENS) {
                        char *endptr;
                        int64_t value = strtoll(token, &endptr, 10);
                        if (endptr != token && *endptr == '\0') {
                            current_word->tokens[current_word->token_count].is_number = 1;
                            current_word->tokens[current_word->token_count].data.value = value;
                        } else {
                            int is_loop_primitive;
                            void *func_ptr = lookup_primitive(token, &is_loop_primitive);
                            if (func_ptr) {
                                if (is_loop_primitive) {
                                    if ((void (*)(int64_t *, int *, int64_t *, int *))func_ptr == forth_do) {
                                        if (loop_depth >= 32) {
                                            printf("Erreur : trop de boucles imbriquées\n");
                                            error_occurred = 1;
                                            continue;
                                        }
                                        current_word->tokens[current_word->token_count].is_number = 3;
                                        current_word->tokens[current_word->token_count].data.loop_label = loop_depth;
                                        loop_labels[loop_depth] = loop_depth;
                                        loop_depth++;
                                    } else if ((void (*)(int64_t *, int *, int64_t *, int *))func_ptr == forth_loop) {
                                        if (loop_depth <= 0) {
                                            printf("Erreur : LOOP sans DO correspondant\n");
                                            error_occurred = 1;
                                            continue;
                                        }
                                        loop_depth--;
                                        current_word->tokens[current_word->token_count].is_number = 4;
                                        current_word->tokens[current_word->token_count].data.loop_label = loop_labels[loop_depth];
                                    } else {
                                        current_word->tokens[current_word->token_count].is_number = 0;
                                        current_word->tokens[current_word->token_count].data.func = func_ptr;
                                    }
                                } else {
                                    current_word->tokens[current_word->token_count].is_number = 0;
                                    current_word->tokens[current_word->token_count].data.func = (void (*)(int64_t *, int *, int64_t *, int *))func_ptr;
                                }
                            } else {
                                UserWord *user_word = lookup_user(token);
                                if (user_word && user_word->compiled_func) {
                                    current_word->tokens[current_word->token_count].is_number = 2;
                                    current_word->tokens[current_word->token_count].data.func = user_word->compiled_func;
                                    strncpy(current_word->tokens[current_word->token_count].data.func_name, token, MAX_WORD_LEN - 1);
                                    current_word->tokens[current_word->token_count].data.func_name[MAX_WORD_LEN - 1] = '\0';
                                } else {
                                    printf("Erreur : mot %s non défini pendant la compilation\n", token);
                                    continue;
                                }
                            }
                        }
                        current_word->token_count++;
                    } else {
                        printf("Erreur : trop de tokens dans la définition\n");
                    }
                }
            } else {
                if (strcmp(token, ":") == 0) {
                    token = strtok(NULL, " ");
                    if (!token) {
                        printf("Erreur : nom de mot manquant après :\n");
                        error_occurred = 1;
                        break;
                    }
                    if (user_word_count >= MAX_WORDS) {
                        printf("Erreur : dictionnaire utilisateur plein\n");
                        error_occurred = 1;
                        break;
                    }
                    int is_loop_primitive;
                    if (lookup_primitive(token, &is_loop_primitive)) {
                        printf("Erreur : mot %s est une primitive, impossible de redéfinir\n", token);
                        error_occurred = 1;
                        break;
                    }
                    UserWord *existing = lookup_user(token);
                    if (existing) {
                        if (existing->dl_handle) {
                            dlclose(existing->dl_handle);
                            existing->dl_handle = NULL;
                        }
                        memset(existing, 0, sizeof(UserWord));
                        strncpy(existing->name, token, MAX_WORD_LEN - 1);
                        existing->name[MAX_WORD_LEN - 1] = '\0';
                        current_word = existing;
                    } else {
                        strncpy(user_dictionary[user_word_count].name, token, MAX_WORD_LEN - 1);
                        user_dictionary[user_word_count].name[MAX_WORD_LEN - 1] = '\0';
                        user_dictionary[user_word_count].token_count = 0;
                        user_dictionary[user_word_count].compiled_func = NULL;
                        user_dictionary[user_word_count].dl_handle = NULL;
                        current_word = &user_dictionary[user_word_count];
                    }
                    compiling = 1;
                } else {
                    UserWord *user_word = lookup_user(token);
                    if (user_word) {
                        if (user_word->compiled_func) {
                            // printf("Exécution du mot compilé : %s\n", token);
                            user_word->compiled_func(stack, &sp, control_stack, &csp);
                            // print_stack();
                        } else {
                            printf("Exécution des tokens du mot : %s\n", token);
                            for (int i = 0; i < user_word->token_count && !error_occurred; i++) {
                                if (user_word->tokens[i].is_number == 1) {
                                    push(user_word->tokens[i].data.value);
                                    // printf("Après push %lld : ", user_word->tokens[i].data.value);
                                    // print_stack();
                                } else if (user_word->tokens[i].is_number == 0) {
                                    void (*func)(int64_t *, int *, int64_t *, int *) = user_word->tokens[i].data.func;
                                    int is_loop_primitive = 0;
                                    for (int j = 0; dictionary[j].name; j++) {
                                        if (dictionary[j].is_loop_primitive &&
                                            dictionary[j].func.loop_func == func) {
                                            is_loop_primitive = 1;
                                            break;
                                        }
                                    }
                                    if (is_loop_primitive) {
                                        if ((func == forth_do || func == forth_loop || func == forth_i) && csp >= 32) {
                                            printf("Erreur : pile de contrôle pleine\n");
                                            error_occurred = 1;
                                        } else if ((func == forth_loop || func == forth_i) && csp < 1) {
                                            printf("Erreur : aucune boucle active pour l'opération\n");
                                            error_occurred = 1;
                                        } else {
                                            func(stack, &sp, control_stack, &csp);
                                            // printf("Après primitive de boucle : ");
                                            // print_stack();
                                        }
                                    } else {
                                        if ((func == (void (*)(int64_t *, int *, int64_t *, int *))forth_add ||
                                             func == (void (*)(int64_t *, int *, int64_t *, int *))forth_swap ||
                                             func == (void (*)(int64_t *, int *, int64_t *, int *))forth_sub ||
                                             func == (void (*)(int64_t *, int *, int64_t *, int *))forth_mult ||
                                             func == (void (*)(int64_t *, int *, int64_t *, int *))forth_div) && sp < 2) {
                                            printf("Erreur : pas assez d'éléments sur la pile pour l'opération\n");
                                            error_occurred = 1;
                                        } else if ((func == (void (*)(int64_t *, int *, int64_t *, int *))forth_drop) && sp < 1) {
                                            printf("Erreur : pile vide pour l'opération\n");
                                            error_occurred = 1;
                                        } else if (func == (void (*)(int64_t *, int *, int64_t *, int *))forth_rot && sp < 3) {
                                            printf("Erreur : pas assez d'éléments sur la pile pour ROT\n");
                                            error_occurred = 1;
                                        } else if (func == (void (*)(int64_t *, int *, int64_t *, int *))forth_div && sp >= 2 && stack[sp-1] == 0) {
                                            printf("Erreur : division par zéro\n");
                                            error_occurred = 1;
                                        } else {
                                            if (func == (void (*)(int64_t *, int *, int64_t *, int *))forth_dot) {
                                                int64_t value = forth_dot(stack, &sp);
                                                print_top(value);
                                                // printf("Après DOT : ");
                                                // print_stack();
                                            } else {
                                                ((void (*)(int64_t *, int *))func)(stack, &sp);
                                                // printf("Après primitive : ");
                                                // print_stack();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        int is_loop_primitive;
                        void *func_ptr = lookup_primitive(token, &is_loop_primitive);
                        if (func_ptr) {
                            if (is_loop_primitive) {
                                void (*func)(int64_t *, int *, int64_t *, int *) = func_ptr;
                                if ((func == forth_do || func == forth_loop || func == forth_i) && csp >= 32) {
                                    printf("Erreur : pile de contrôle pleine\n");
                                    error_occurred = 1;
                                } else if ((func == forth_loop || func == forth_i) && csp < 1) {
                                    printf("Erreur : aucune boucle active pour %s\n", token);
                                    error_occurred = 1;
                                } else {
                                    func(stack, &sp, control_stack, &csp);
                                    // printf("Après primitive de boucle %s : ", token);
                                    // print_stack();
                                }
                            } else {
                                void (*func)(int64_t *, int *) = func_ptr;
                                if ((func == forth_add || func == forth_swap || func == forth_sub || func == forth_mult || func == forth_div) && sp < 2) {
                                    printf("Erreur : pas assez d'éléments sur la pile pour %s\n", token);
                                    error_occurred = 1;
                                } else if ((func == forth_drop) && sp < 1) {
                                    printf("Erreur : pile vide pour %s\n", token);
                                    error_occurred = 1;
                                } else if (func == forth_rot && sp < 3) {
                                    printf("Erreur : pas assez d'éléments sur la pile pour ROT\n");
                                    error_occurred = 1;
                                } else if (func == forth_div && sp >= 2 && stack[sp-1] == 0) {
                                    printf("Erreur : division par zéro\n");
                                    error_occurred = 1;
                                } else {
                                    if (func == (void (*)(int64_t *, int *))forth_dot) {
                                        int64_t value = forth_dot(stack, &sp);
                                        //print_top(value);
                                       // printf("Après DOT : ");
                                       //  print_stack();
                                    } else {
                                        func(stack, &sp);
                                        // printf("Après primitive %s : ", token);
                                       //  print_stack();
                                    }
                                }
                            }
                        } else {
                            char *endptr;
                            int64_t value = strtoll(token, &endptr, 10);
                            if (endptr != token && *endptr == '\0') {
                                push(value);
                                // printf("Après push %lld : ", value);
                                // print_stack();
                            } else {
                                printf("Mot inconnu : %s\n", token);
                                error_occurred = 1;
                            }
                        }
                    }
                }
            }
            token = strtok(NULL, " ");
        }
        if (!compiling) {
            print_stack();
        }
    }

    for (int i = 0; i < user_word_count; i++) {
        if (user_dictionary[i].dl_handle) {
            dlclose(user_dictionary[i].dl_handle);
        }
    }

    return 0;
}
