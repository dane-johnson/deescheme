#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "deescheme.h"

int hash(const char* key) {
  int sum = 0;
  for (int i = 0; i < strlen(key); i++) {
    sum += key[i];
  }
  return sum % HASHMAP_SIZE;
}

void insert_symbol(char* key, object* val) {
  int h = hash(key);
  entry* myentry = (entry*) malloc(sizeof(entry));
  myentry->key = key;
  myentry->val = val;
  myentry->next = NULL;
  if (symbolmap[h]) {
    entry* curr;
    curr = symbolmap[h];
    while(curr->next) {
      if (strcmp(curr->key, key) == 0) {
        curr->val = val;
        return;
      }
      curr = curr->next;
    }
    if (strcmp(curr->key, key) == 0) {
      curr->val = val;
      return;
    }
    curr->next = myentry;
  } else {
    symbolmap[h] = myentry;
  }
}

object* retrieve_symbol(const char* key) {
  int h = hash(key);
  if (symbolmap[h]) {
    entry* curr = symbolmap[h];
    while(curr) {
      if (strcmp(key, curr->key) == 0) {
        return curr->val;
      };
      curr = curr->next;
    }
  }
  return NULL;
}

/********/
/* READ */
/********/

void stripws(FILE* in) {
  int c;
  c = getc(in);
  while(c == ' ' || c == '\t' || c == '\n') {
    c = getc(in);
  }
  ungetc(c, in);
}

object* read(FILE* in) {
  object* o = (object*) malloc(sizeof(object));
  int quoted = 0;
  stripws(in);
  int c;
  c = getc(in);
  if (c == EOF) {
    o->type = L_EOF;
    return o;
  }
  if (c == '\'') {
    quoted = 1;
    c = getc(in);
  }
  if (isdigit(c)) {
    ungetc(c, in);
    o->type = NUM;
    fscanf(in, "%d", &o->val);
    return o;
  } else if (c == '#') {
    c = getc(in);
    if (c == 't') {
      o->val = 't';
      o->type = BOOL;
      return o;
    } else if (c == 'f') {
      o->val = 'f';
      o->type = BOOL;
      return o;
    } else if (c == '\\') {
      c = getc(in);
      o->type = CHAR;
      o->val = c;
      return o;
    } else {
      fprintf(stderr, "unknown bool %c\n", c);
      exit(1);
    }
  } else if (c == '(') {
    o = read_pair(in);
    if (quoted) {
      o->flags |= QUOTED;
    }
    return o;
  } else {
    ungetc(c, in);
    o = read_symbol(in);
    if (quoted) {
      o->flags |= QUOTED;
    }
    return o;
  }
}

object* read_pair(FILE* in) {
  object* cons = (object*) malloc(sizeof(object));
  cons->type = CONS;
  stripws(in);
  int c;
  c = getc(in);
  if (c == ')') {
    cons->car = NULL;
    cons->cdr = NULL;
    return cons;
  } else {
    ungetc(c, in);
    object* car = read(in);
    cons->car = car;
    object* cdr = read_pair(in);
    cons->cdr = cdr;
    return cons;
  }
}

object* read_symbol(FILE* in) {
  int c;
  char* head = tablehead;
  int len = 0;
  object* o = (object*) malloc(sizeof(object));
  c = getc(in);
  while (c != ' ' && c != '\t' && c != '\n' && c != ')') {
    head[len] = c;
    len++;
    c = getc(in);
  }
  if (c == ')') {
    ungetc (c, in);
  }
  if (!retrieve_symbol(head)) {
    insert_symbol(head, &undefined);
  }
  o->symbol = head;
  tablehead += len + 1;
  if (is_builtin(o->symbol)) {
    o->type = BUILTIN;
  } else {
    o->type = SYMBOL;
  }
  return o;
}

int is_builtin(const char* c) {
  for(int i = 0; i < NUM_BUILTINS; i++) {
    if (strcmp(c, builtins[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

/********/
/* EVAL */
/********/

object* eval(object* o) {
  object* func;
  if (o->flags & QUOTED) {
    o->flags &= ~QUOTED;
    return o;
  }
  switch(o->type) {
  case SYMBOL:
    return retrieve_symbol(o->symbol);
  case CONS:
    func = eval(o->car);
    return dofunc(func, o->cdr);
  default:
    return o;
  }
}

object* dofunc(object* func, object* argcons) {
  if (strcmp("def", func->symbol) == 0) {
    insert_symbol(argcons->car->symbol, eval(argcons->cdr->car));
    return &nil;
  } else if (strcmp("quote", func->symbol) == 0) {
    argcons->flags |= QUOTED;
    return argcons;
  } else if (strcmp("car", func->symbol) == 0) {
    return eval(argcons->car)->car;
  } else if (strcmp("cdr", func->symbol) == 0) {
    return eval(argcons->car)->cdr;
  } else if (strcmp("print", func->symbol) == 0) {
    print(eval(argcons->car));
    printf("\n");
    return &nil;
  } else if (strcmp("eval", func->symbol) == 0) {
    return eval(eval(argcons->car));
  } else {
    fprintf(stderr, "void function %s", func->symbol);
    return &nil;
  }
}

/*********/
/* PRINT */
/*********/

void print(object* o) {
  if (!o) {
    printf("undefined");
    return;
  }
  switch(o->type) {
  case NUM:
    printf("%d", o->val);
    break;
  case BOOL:
    printf("#%c", o->val);
    break;
  case CHAR:
    printf("#\\%c", o->val);
    break;
  case CONS:
    if (!o->car) {
      printf("nil");
      break;
    }
    printf("(");
    print_pair(o);
    printf(")");
    break;
  case BUILTIN:
  case SYMBOL:
    printf("%s", o->symbol);
    break;
  case NONE:
    printf("undefined");
    break;
  }
}

void print_pair(object* o) {
  if (!o->car) {
    return;
  } else {
    print(o->car);
    if (o->cdr->car) {
      printf(" ");
      print_pair(o->cdr);
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc == 2){
    FILE* in;
    in = fopen(argv[1], "rw");
    while(1) {
      object* o = read(in);
      object* retval = eval(o);
      if (retval->type == L_EOF) {
        fclose(in);
        return 0;
      }
    }
  } else {
    while(1) {
      object* o = read(stdin);
      object* retval = eval(o);
      if (retval->type == L_EOF) {
        return 0;
      }
      print(retval);
      printf("\n");
    }
  }
}
