#ifndef DEESCHEME_H
#define DEESCHEME_H

#define HASHMAP_SIZE 30
#define TABLE_SIZE 500

typedef struct entry {
  char* key;
  struct object* val;
  struct entry* next;
} entry;

entry *symbolmap[HASHMAP_SIZE];
char symboltable[TABLE_SIZE];
char *tablehead = symboltable;

int hash(const char*);
void insert_symbol(char*,struct object*);
struct object* retrieve_symbol(const char*);

typedef enum type {NUM, BOOL, CHAR, CONS, SYMBOL, FUNC, BUILTIN, L_EOF} type;

typedef struct object {
  type type;
  union {
    int val;
    struct {
      struct object* car;
      struct object* cdr;
    };
    char* symbol;
  };
  int flags;
} object;

#define QUOTED 1

object nil = {CONS, {NULL, NULL}};
char* builtins[] = { "def", "quote", "car", "cdr", "print" };
#define NUM_BUILTINS 5

object* read(FILE*);
object* read_pair(FILE*);
object* read_symbol(FILE*);
int is_builtin(const char*);
object* eval(object*);
object* dofunc(object*,object*);
void print(object*);
void print_pair(object*);

#endif /* DEESCHEME_H */
