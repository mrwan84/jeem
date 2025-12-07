/*
  Jeem Interpreter  v3.0
  A lightweight scripting language interpreter in C.
  Compile:
    Linux/Mac: gcc -O2 -o jeem jeem.c -lm
    Windows:   gcc -O2 -o jeem.exe jeem.c -lm -lws2_32
*/

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <direct.h>
#pragma comment(lib, "ws2_32.lib")
#define sleep_ms(ms) Sleep(ms)
#define PATH_SEP '\\'
#define getcwd _getcwd
#define close closesocket
typedef int socklen_t;
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/select.h>
#define sleep_ms(ms) usleep((ms)*1000)
#define PATH_SEP '/'
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <limits.h>
#include <time.h>
#include <errno.h>

#define MAX_TOK 4096
#define MAX_SRC 1000000
#define MAX_STR 65536
#define MAX_IMPORTS 256
#define HTTP_BUF 65536

typedef struct Value Value;
typedef struct AST AST;
typedef struct Scope Scope;
typedef struct ClassDef ClassDef;

typedef enum {
    TOK_EOF,TOK_NUMBER,TOK_STRING,TOK_IDENT,TOK_TEMPLATE,
    TOK_LPAREN,TOK_RPAREN,TOK_LBRACE,TOK_RBRACE,TOK_LBRACKET,TOK_RBRACKET,
    TOK_COMMA,TOK_COLON,TOK_SEMICOLON,TOK_DOT,TOK_QUESTION,
    TOK_PLUS,TOK_MINUS,TOK_MUL,TOK_DIV,TOK_MOD,TOK_POW,
    TOK_ASSIGN,TOK_EQ,TOK_NEQ,TOK_LT,TOK_GT,TOK_LE,TOK_GE,
    TOK_AND,TOK_OR,TOK_NOT,TOK_PLUSPLUS,TOK_MINUSMINUS,
    TOK_PLUSEQ,TOK_MINUSEQ,TOK_MULEQ,TOK_DIVEQ,TOK_MODEQ,TOK_POWEQ,TOK_RANGE,
    TOK_ARROW,
    TOK_IF,TOK_ELSE,TOK_FOR,TOK_IN,TOK_WHILE,TOK_FUNC,TOK_RETURN,
    TOK_BREAK,TOK_CONTINUE,TOK_CONST,TOK_NULL,TOK_TRUE,TOK_FALSE,
    TOK_CASE,TOK_DEFAULT,TOK_CLASS,TOK_NEW,TOK_THIS,TOK_EXTENDS,TOK_SUPER,
    TOK_IMPORT,TOK_AS,TOK_EXPORT,TOK_UNKNOWN
} TokType;

typedef struct { int is_expr; char *text; } TemplatePart;
typedef struct { TokType type; char text[MAX_TOK]; double num; int line; TemplatePart *tpl; int tpl_n; } Token;
typedef enum { V_NULL,V_BOOL,V_INT,V_FLOAT,V_STRING,V_ARRAY,V_OBJECT,V_FUNC,V_CLASS,V_INSTANCE,V_MODULE,V_CLOSURE } ValueType;
typedef struct { int len,cap; Value **items; } Array;
typedef struct { char *key; Value *val; } KV;
typedef struct { int count,cap; KV *pairs; } Object;
typedef struct { ClassDef *cd; Object *props; } Instance;
typedef struct { char *name; char **params; int param_n; AST *body; Scope *env; int is_arrow; } Closure;

struct Value {
    ValueType type; int ref;
    union { int b; long long i; double f; char *s; Array *arr; Object *obj; Instance *inst; Closure *cl; } as;
};

typedef enum {
    AST_NOP,AST_BLOCK,AST_LITERAL,AST_TEMPLATE,AST_ARRAY,AST_OBJECT,AST_VAR,
    AST_ASSIGN,AST_INDEX_ASSIGN,AST_BINOP,AST_UNARY,AST_TERNARY,AST_INDEX,
    AST_MEMBER,AST_CALL,AST_IF,AST_WHILE,AST_FOR,AST_FOR_C,AST_CASE,
    AST_BREAK,AST_CONTINUE,AST_RETURN,AST_FUNC,AST_CLASS,AST_NEW,AST_THIS,
    AST_SUPER,AST_IMPORT,AST_COMPOUND,AST_EXPR_STMT,AST_ARROW
} ASTType;

typedef struct { int is_def,is_range; long long *vals; int val_n; long long rs,re; AST *body; } CaseBr;

struct AST {
    ASTType type; int line,is_const,is_arrow; char *name; char op[4];
    AST *left,*right,*cond,*body; Value *lit;
    AST **children; int child_n; char **keys;
    char **params; int param_n;
    char *iter_var,*idx_var; AST *iter_expr,*init,*update;
    CaseBr *branches; int branch_n;
    char *parent; AST **methods; int method_n;
    char *imp_path,*imp_alias;
    TemplatePart *tpl; int tpl_n;
};

typedef struct { char *name; char **params; int param_n; AST *body; } Method;
struct ClassDef { char *name,*parent; Method *methods; int method_n,method_cap; };
typedef struct { char *name; Value *val; int is_const; } Symbol;
struct Scope { Symbol *syms; int count,cap; Scope *parent; };
typedef struct { char *name; char **params; int param_n; AST *body; Scope *closure; } FuncDef;
typedef struct { char *path,*alias; Value *module; } ImportEntry;

/* HTTP Server */
typedef struct { char *method; char *path; Value *handler; } HttpRoute;
typedef struct { SOCKET sock; int running,port; HttpRoute *routes; int route_n,route_cap; } HttpServer;

static char *g_src=NULL; static int g_pos=0,g_len=0,g_line=1;
static Token g_tok; static const char *g_file="<input>"; static char *g_dir=NULL;
static Scope *g_global=NULL,*g_scope=NULL;
static FuncDef *g_funcs=NULL; static int g_func_n=0,g_func_cap=0;
static ClassDef *g_classes=NULL; static int g_class_n=0,g_class_cap=0;
static ImportEntry g_imports[MAX_IMPORTS]; static int g_import_n=0;
static int g_break=0,g_continue=0,g_return=0,g_rand_init=0,g_ws_init=0;
static Value *g_retval=NULL,*g_this=NULL;
static HttpServer *g_server=NULL;

/* Timer system */
typedef struct { int id; long long fire_at; int interval; int active; Value *callback; } Timer;
static Timer *g_timers=NULL; static int g_timer_n=0,g_timer_cap=0,g_timer_next_id=1;

static void *xmalloc(size_t n) { void *p=malloc(n); if(!p&&n){fprintf(stderr,"OOM\n");exit(1);} return p; }
static void *xrealloc(void *p,size_t n) { void *q=realloc(p,n); if(!q&&n){fprintf(stderr,"OOM\n");exit(1);} return q; }
static char *xstrdup(const char *s) { if(!s)return NULL; char *p=xmalloc(strlen(s)+1); strcpy(p,s); return p; }
static void error(int line,const char *fmt,...) { va_list ap; fprintf(stderr,"%s:%d: error: ",g_file,line); va_start(ap,fmt); vfprintf(stderr,fmt,ap); va_end(ap); fprintf(stderr,"\n"); exit(1); }
#define runtime_error error

static Value *eval(AST *node);
static Value *call_closure(Value *closure, Value **args, int argc);
static Value *call_method(Value *obj, const char *method, Value **args, int argc);

static Value *val_new(ValueType t) { Value *v=xmalloc(sizeof(Value)); memset(v,0,sizeof(Value)); v->type=t; v->ref=1; return v; }
static Value *val_null(void) { return val_new(V_NULL); }
static Value *val_bool(int b) { Value *v=val_new(V_BOOL); v->as.b=b?1:0; return v; }
static Value *val_int(long long i) { Value *v=val_new(V_INT); v->as.i=i; return v; }
static Value *val_float(double f) { Value *v=val_new(V_FLOAT); v->as.f=f; return v; }
static Value *val_string(const char *s) { Value *v=val_new(V_STRING); v->as.s=xstrdup(s?s:""); return v; }
static Value *val_array(int cap) { Value *v=val_new(V_ARRAY); v->as.arr=xmalloc(sizeof(Array)); v->as.arr->len=0; v->as.arr->cap=cap>0?cap:8; v->as.arr->items=xmalloc(sizeof(Value*)*v->as.arr->cap); return v; }
static Value *val_object(void) { Value *v=val_new(V_OBJECT); v->as.obj=xmalloc(sizeof(Object)); v->as.obj->count=0; v->as.obj->cap=8; v->as.obj->pairs=xmalloc(sizeof(KV)*8); return v; }
static Value *val_func(const char *n) { Value *v=val_new(V_FUNC); v->as.s=xstrdup(n); return v; }
static Value *val_module(void) { Value *v=val_new(V_MODULE); v->as.obj=xmalloc(sizeof(Object)); v->as.obj->count=0; v->as.obj->cap=16; v->as.obj->pairs=xmalloc(sizeof(KV)*16); return v; }
static Value *val_closure_new(Closure *cl) { Value *v=val_new(V_CLOSURE); v->as.cl=cl; return v; }
static Value *val_instance(ClassDef *cd) { Value *v=val_new(V_INSTANCE); v->as.inst=xmalloc(sizeof(Instance)); v->as.inst->cd=cd; v->as.inst->props=xmalloc(sizeof(Object)); v->as.inst->props->count=0; v->as.inst->props->cap=16; v->as.inst->props->pairs=xmalloc(sizeof(KV)*16); return v; }

static void val_incref(Value *v) { if(v)v->ref++; }
static void scope_free(Scope *s);
static void val_free(Value *v);
static void val_decref(Value *v) { if(v&&--v->ref<=0)val_free(v); }

static void val_free(Value *v) {
    if(!v)return;
    switch(v->type) {
        case V_STRING: case V_FUNC: free(v->as.s); break;
        case V_ARRAY: if(v->as.arr){for(int i=0;i<v->as.arr->len;i++)val_decref(v->as.arr->items[i]);free(v->as.arr->items);free(v->as.arr);} break;
        case V_OBJECT: case V_MODULE: if(v->as.obj){for(int i=0;i<v->as.obj->count;i++){free(v->as.obj->pairs[i].key);val_decref(v->as.obj->pairs[i].val);}free(v->as.obj->pairs);free(v->as.obj);} break;
        case V_INSTANCE: if(v->as.inst&&v->as.inst->props){for(int i=0;i<v->as.inst->props->count;i++){free(v->as.inst->props->pairs[i].key);val_decref(v->as.inst->props->pairs[i].val);}free(v->as.inst->props->pairs);free(v->as.inst->props);free(v->as.inst);} break;
        case V_CLOSURE: if(v->as.cl){free(v->as.cl->name);for(int i=0;i<v->as.cl->param_n;i++)free(v->as.cl->params[i]);free(v->as.cl->params);if(v->as.cl->env)scope_free(v->as.cl->env);free(v->as.cl);} break;
        default: break;
    }
    free(v);
}

static void arr_push(Value *arr,Value *item) { if(arr->type!=V_ARRAY)return; Array *a=arr->as.arr; if(a->len>=a->cap){a->cap*=2;a->items=xrealloc(a->items,sizeof(Value*)*a->cap);} val_incref(item); a->items[a->len++]=item; }
static Value *arr_pop(Value *arr) { if(arr->type!=V_ARRAY||arr->as.arr->len==0)return val_null(); return arr->as.arr->items[--arr->as.arr->len]; }
static Value *arr_get(Value *arr,int idx) { if(arr->type!=V_ARRAY)return NULL; Array *a=arr->as.arr; if(idx<0)idx=a->len+idx; if(idx<0||idx>=a->len)return NULL; return a->items[idx]; }
static void arr_set(Value *arr,int idx,Value *val) { if(arr->type!=V_ARRAY)return; Array *a=arr->as.arr; if(idx<0)idx=a->len+idx; if(idx<0||idx>=a->len)return; val_decref(a->items[idx]); val_incref(val); a->items[idx]=val; }

static Value *obj_get(Value *obj,const char *key) { if(obj->type!=V_OBJECT&&obj->type!=V_MODULE)return NULL; for(int i=0;i<obj->as.obj->count;i++)if(strcmp(obj->as.obj->pairs[i].key,key)==0)return obj->as.obj->pairs[i].val; return NULL; }
static void obj_set(Value *obj,const char *key,Value *val) { if(obj->type!=V_OBJECT&&obj->type!=V_MODULE)return; Object *o=obj->as.obj; for(int i=0;i<o->count;i++){if(strcmp(o->pairs[i].key,key)==0){val_decref(o->pairs[i].val);val_incref(val);o->pairs[i].val=val;return;}} if(o->count>=o->cap){o->cap*=2;o->pairs=xrealloc(o->pairs,sizeof(KV)*o->cap);} o->pairs[o->count].key=xstrdup(key); val_incref(val); o->pairs[o->count++].val=val; }
static int obj_has(Value *obj,const char *key) { if(obj->type!=V_OBJECT&&obj->type!=V_MODULE)return 0; for(int i=0;i<obj->as.obj->count;i++)if(strcmp(obj->as.obj->pairs[i].key,key)==0)return 1; return 0; }
static void obj_del(Value *obj,const char *key) { if(obj->type!=V_OBJECT&&obj->type!=V_MODULE)return; Object *o=obj->as.obj; for(int i=0;i<o->count;i++){if(strcmp(o->pairs[i].key,key)==0){free(o->pairs[i].key);val_decref(o->pairs[i].val);memmove(&o->pairs[i],&o->pairs[i+1],sizeof(KV)*(o->count-i-1));o->count--;return;}} }

static Value *inst_get(Value *inst,const char *key) { if(inst->type!=V_INSTANCE)return NULL; Object *o=inst->as.inst->props; for(int i=0;i<o->count;i++)if(strcmp(o->pairs[i].key,key)==0)return o->pairs[i].val; return NULL; }
static void inst_set(Value *inst,const char *key,Value *val) { if(inst->type!=V_INSTANCE)return; Object *o=inst->as.inst->props; for(int i=0;i<o->count;i++){if(strcmp(o->pairs[i].key,key)==0){val_decref(o->pairs[i].val);val_incref(val);o->pairs[i].val=val;return;}} if(o->count>=o->cap){o->cap*=2;o->pairs=xrealloc(o->pairs,sizeof(KV)*o->cap);} o->pairs[o->count].key=xstrdup(key); val_incref(val); o->pairs[o->count++].val=val; }

static Value *val_copy(Value *v) { if(!v)return val_null(); switch(v->type){case V_NULL:return val_null();case V_BOOL:return val_bool(v->as.b);case V_INT:return val_int(v->as.i);case V_FLOAT:return val_float(v->as.f);case V_STRING:return val_string(v->as.s);case V_FUNC:return val_func(v->as.s);case V_CLASS:{Value *c=val_new(V_CLASS);c->as.s=xstrdup(v->as.s);return c;}case V_CLOSURE:val_incref(v);return v;default:val_incref(v);return v;} }
static Value *val_clone(Value *v) { if(!v)return val_null(); switch(v->type){case V_NULL:case V_BOOL:case V_INT:case V_FLOAT:case V_STRING:case V_FUNC:return val_copy(v);case V_ARRAY:{Value *arr=val_array(v->as.arr->cap);for(int i=0;i<v->as.arr->len;i++){Value *item=val_clone(v->as.arr->items[i]);arr_push(arr,item);val_decref(item);}return arr;}case V_OBJECT:{Value *obj=val_object();for(int i=0;i<v->as.obj->count;i++){Value *val=val_clone(v->as.obj->pairs[i].val);obj_set(obj,v->as.obj->pairs[i].key,val);val_decref(val);}return obj;}default:return val_copy(v);} }

static void val_sprint(Value *v,char *buf,int size) {
    if(!v){snprintf(buf,size,"null");return;} int pos=0;
    switch(v->type) {
        case V_NULL:snprintf(buf,size,"null");break; case V_BOOL:snprintf(buf,size,"%s",v->as.b?"true":"false");break;
        case V_INT:snprintf(buf,size,"%lld",v->as.i);break; case V_FLOAT:snprintf(buf,size,"%g",v->as.f);break;
        case V_STRING:snprintf(buf,size,"%s",v->as.s);break; case V_FUNC:snprintf(buf,size,"<func:%s>",v->as.s);break;
        case V_MODULE:snprintf(buf,size,"<module>");break; case V_CLASS:snprintf(buf,size,"<class:%s>",v->as.s);break;
        case V_INSTANCE:snprintf(buf,size,"<%s>",v->as.inst->cd->name);break; case V_CLOSURE:snprintf(buf,size,"<closure>");break;
        case V_ARRAY: pos+=snprintf(buf+pos,size-pos,"["); for(int i=0;i<v->as.arr->len&&i<20&&pos<size-50;i++){if(i)pos+=snprintf(buf+pos,size-pos,", ");Value *item=v->as.arr->items[i];if(item->type==V_STRING)pos+=snprintf(buf+pos,size-pos,"\"%s\"",item->as.s);else{char t[256];val_sprint(item,t,256);pos+=snprintf(buf+pos,size-pos,"%s",t);}} if(v->as.arr->len>20)pos+=snprintf(buf+pos,size-pos,"..."); snprintf(buf+pos,size-pos,"]");break;
        case V_OBJECT: pos+=snprintf(buf+pos,size-pos,"{"); for(int i=0;i<v->as.obj->count&&i<10&&pos<size-50;i++){if(i)pos+=snprintf(buf+pos,size-pos,", ");Value *val=v->as.obj->pairs[i].val;if(val->type==V_STRING)pos+=snprintf(buf+pos,size-pos,"%s: \"%s\"",v->as.obj->pairs[i].key,val->as.s);else{char t[256];val_sprint(val,t,256);pos+=snprintf(buf+pos,size-pos,"%s: %s",v->as.obj->pairs[i].key,t);}} if(v->as.obj->count>10)pos+=snprintf(buf+pos,size-pos,"..."); snprintf(buf+pos,size-pos,"}");break;
    }
}
static char *val_tostr(Value *v) { static char buf[MAX_STR]; val_sprint(v,buf,sizeof(buf)); return buf; }
static int val_truthy(Value *v) { if(!v)return 0; switch(v->type){case V_NULL:return 0;case V_BOOL:return v->as.b;case V_INT:return v->as.i!=0;case V_FLOAT:return v->as.f!=0.0;case V_STRING:return v->as.s&&v->as.s[0];case V_ARRAY:return v->as.arr->len>0;case V_OBJECT:return v->as.obj->count>0;case V_CLOSURE:return 1;default:return 1;} }
static double val_tonum(Value *v) { if(!v)return 0; switch(v->type){case V_INT:return(double)v->as.i;case V_FLOAT:return v->as.f;case V_BOOL:return v->as.b?1.0:0.0;case V_STRING:return atof(v->as.s);default:return 0;} }

static Scope *scope_new(Scope *parent) { Scope *s=xmalloc(sizeof(Scope)); s->syms=NULL; s->count=s->cap=0; s->parent=parent; return s; }
static Scope *scope_clone(Scope *s) { 
    if(!s)return NULL; 
    Scope *clone=xmalloc(sizeof(Scope)); 
    clone->count=0; clone->cap=32; 
    clone->syms=xmalloc(sizeof(Symbol)*clone->cap); 
    clone->parent=g_global;
    /* Walk up scope chain and copy all variables */
    for(Scope *cur=s; cur && cur!=g_global; cur=cur->parent) {
        for(int i=0;i<cur->count;i++) {
            /* Check if we already have this var (inner scope shadows outer) */
            int found=0;
            for(int j=0;j<clone->count;j++) {
                if(strcmp(clone->syms[j].name,cur->syms[i].name)==0) { found=1; break; }
            }
            if(!found) {
                if(clone->count>=clone->cap) { clone->cap*=2; clone->syms=xrealloc(clone->syms,sizeof(Symbol)*clone->cap); }
                clone->syms[clone->count].name=xstrdup(cur->syms[i].name);
                clone->syms[clone->count].val=val_copy(cur->syms[i].val);
                clone->syms[clone->count].is_const=cur->syms[i].is_const;
                clone->count++;
            }
        }
    }
    return clone; 
}
static void scope_free(Scope *s) { if(!s)return; for(int i=0;i<s->count;i++){free(s->syms[i].name);val_decref(s->syms[i].val);} free(s->syms); free(s); }
static Symbol *scope_find(Scope *s,const char *name) { while(s){for(int i=0;i<s->count;i++)if(strcmp(s->syms[i].name,name)==0)return &s->syms[i];s=s->parent;} return NULL; }
static Symbol *scope_find_local(Scope *s,const char *name) { for(int i=0;i<s->count;i++)if(strcmp(s->syms[i].name,name)==0)return &s->syms[i]; return NULL; }
static void scope_def(Scope *s,const char *name,Value *val,int is_const) { Symbol *ex=scope_find_local(s,name); if(ex){if(ex->is_const)runtime_error(g_line,"cannot reassign constant '%s'",name);val_decref(ex->val);val_incref(val);ex->val=val;return;} if(s->count>=s->cap){s->cap=s->cap?s->cap*2:8;s->syms=xrealloc(s->syms,sizeof(Symbol)*s->cap);} s->syms[s->count].name=xstrdup(name);val_incref(val);s->syms[s->count].val=val;s->syms[s->count++].is_const=is_const; }
static void scope_set(Scope *s,const char *name,Value *val) { Symbol *sym=scope_find(s,name); if(!sym){scope_def(s,name,val,0);return;} if(sym->is_const)runtime_error(g_line,"cannot reassign constant '%s'",name); val_decref(sym->val);val_incref(val);sym->val=val; }

static ClassDef *class_find(const char *name) { for(int i=0;i<g_class_n;i++)if(strcmp(g_classes[i].name,name)==0)return &g_classes[i]; return NULL; }
static Method *method_find(ClassDef *cd,const char *name) { for(int i=0;i<cd->method_n;i++)if(strcmp(cd->methods[i].name,name)==0)return &cd->methods[i]; if(cd->parent){ClassDef *p=class_find(cd->parent);if(p)return method_find(p,name);} return NULL; }
static void class_register(ClassDef *cd) { if(g_class_n>=g_class_cap){g_class_cap=g_class_cap?g_class_cap*2:64;g_classes=xrealloc(g_classes,sizeof(ClassDef)*g_class_cap);} g_classes[g_class_n++]=*cd; }
static FuncDef *func_find(const char *name) { for(int i=g_func_n-1;i>=0;i--)if(strcmp(g_funcs[i].name,name)==0)return &g_funcs[i]; return NULL; }
static void func_register(AST *node) { if(g_func_n>=g_func_cap){g_func_cap=g_func_cap?g_func_cap*2:16;g_funcs=xrealloc(g_funcs,sizeof(FuncDef)*g_func_cap);} FuncDef *fn=&g_funcs[g_func_n++]; fn->name=xstrdup(node->name); fn->params=xmalloc(sizeof(char*)*node->param_n); fn->param_n=node->param_n; for(int i=0;i<node->param_n;i++)fn->params[i]=xstrdup(node->params[i]); fn->body=node->body; fn->closure=g_global; }

/* Lexer */
static void skip_ws(void) { while(g_pos<g_len){char c=g_src[g_pos];if(c==' '||c=='\t'||c=='\r'){g_pos++;continue;}if(c=='\n'){g_pos++;g_line++;continue;}if((c=='/'&&g_pos+1<g_len&&g_src[g_pos+1]=='/')||c=='#'){if(c=='/')g_pos+=2;else g_pos++;while(g_pos<g_len&&g_src[g_pos]!='\n')g_pos++;continue;}if(c=='/'&&g_pos+1<g_len&&g_src[g_pos+1]=='*'){g_pos+=2;while(g_pos+1<g_len){if(g_src[g_pos]=='\n')g_line++;if(g_src[g_pos]=='*'&&g_src[g_pos+1]=='/'){g_pos+=2;break;}g_pos++;}continue;}break;} }

static void lex_template(Token *tk,char quote) {
    tk->tpl=NULL; tk->tpl_n=0; g_pos++; char buf[MAX_TOK]; int bp=0;
    while(g_pos<g_len) {
        char c=g_src[g_pos]; if(c==quote){g_pos++;break;} if(c=='\n')g_line++;
        if(c=='{'&&g_pos+1<g_len&&g_src[g_pos+1]=='{') { if(bp>0){buf[bp]='\0';tk->tpl=xrealloc(tk->tpl,sizeof(TemplatePart)*(tk->tpl_n+1));tk->tpl[tk->tpl_n].is_expr=0;tk->tpl[tk->tpl_n++].text=xstrdup(buf);bp=0;} g_pos+=2; char expr[MAX_TOK];int ep=0,depth=0; while(g_pos<g_len&&ep<MAX_TOK-1){if(g_src[g_pos]=='{'){depth++;expr[ep++]=g_src[g_pos++];}else if(g_src[g_pos]=='}'){if(depth>0){depth--;expr[ep++]=g_src[g_pos++];}else if(g_pos+1<g_len&&g_src[g_pos+1]=='}'){g_pos+=2;break;}else expr[ep++]=g_src[g_pos++];}else{if(g_src[g_pos]=='\n')g_line++;expr[ep++]=g_src[g_pos++];}} expr[ep]='\0'; tk->tpl=xrealloc(tk->tpl,sizeof(TemplatePart)*(tk->tpl_n+1));tk->tpl[tk->tpl_n].is_expr=1;tk->tpl[tk->tpl_n++].text=xstrdup(expr);continue; }
        if(c=='\\'&&g_pos+1<g_len){g_pos++;char n=g_src[g_pos++];switch(n){case 'n':buf[bp++]='\n';break;case 't':buf[bp++]='\t';break;case 'r':buf[bp++]='\r';break;case '\\':case '"':case '\'':case '{':buf[bp++]=n;break;default:buf[bp++]=n;}continue;}
        buf[bp++]=c; g_pos++;
    }
    if(bp>0){buf[bp]='\0';tk->tpl=xrealloc(tk->tpl,sizeof(TemplatePart)*(tk->tpl_n+1));tk->tpl[tk->tpl_n].is_expr=0;tk->tpl[tk->tpl_n++].text=xstrdup(buf);}
    int has_expr=0; for(int i=0;i<tk->tpl_n;i++)if(tk->tpl[i].is_expr){has_expr=1;break;}
    if(has_expr){tk->type=TOK_TEMPLATE;tk->text[0]='\0';}
    else{tk->type=TOK_STRING;int pos=0;for(int i=0;i<tk->tpl_n&&pos<MAX_TOK-1;i++){int len=strlen(tk->tpl[i].text);if(pos+len<MAX_TOK-1){strcpy(tk->text+pos,tk->tpl[i].text);pos+=len;}free(tk->tpl[i].text);}tk->text[pos]='\0';free(tk->tpl);tk->tpl=NULL;tk->tpl_n=0;}
}

static void lex_number(Token *tk) { int i=0,has_dot=0; while(g_pos<g_len&&i<MAX_TOK-1){char c=g_src[g_pos];if(isdigit((unsigned char)c)){tk->text[i++]=c;g_pos++;}else if(c=='.'&&!has_dot&&g_pos+1<g_len&&isdigit((unsigned char)g_src[g_pos+1])){has_dot=1;tk->text[i++]=c;g_pos++;}else if((c=='e'||c=='E')&&i>0){tk->text[i++]=c;g_pos++;if(g_pos<g_len&&(g_src[g_pos]=='+'||g_src[g_pos]=='-'))tk->text[i++]=g_src[g_pos++];}else break;} tk->text[i]='\0';tk->num=atof(tk->text);tk->type=TOK_NUMBER; }

static void next_tok(Token *tk) {
    skip_ws(); memset(tk,0,sizeof(Token)); tk->line=g_line;
    if(g_pos>=g_len){tk->type=TOK_EOF;return;}
    char c=g_src[g_pos];
    if(c=='"'||c=='\''||c=='`'){lex_template(tk,c);return;}
    if(isdigit((unsigned char)c)){lex_number(tk);return;}
    if(isalpha((unsigned char)c)||c=='_') {
        int i=0;while(g_pos<g_len&&(isalnum((unsigned char)g_src[g_pos])||g_src[g_pos]=='_')&&i<MAX_TOK-1)tk->text[i++]=g_src[g_pos++];tk->text[i]='\0';
        struct{const char *kw;TokType t;}kws[]={{"if",TOK_IF},{"else",TOK_ELSE},{"for",TOK_FOR},{"in",TOK_IN},{"while",TOK_WHILE},{"func",TOK_FUNC},{"fn",TOK_FUNC},{"function",TOK_FUNC},{"return",TOK_RETURN},{"break",TOK_BREAK},{"continue",TOK_CONTINUE},{"const",TOK_CONST},{"let",TOK_CONST},{"null",TOK_NULL},{"nil",TOK_NULL},{"undefined",TOK_NULL},{"true",TOK_TRUE},{"false",TOK_FALSE},{"case",TOK_CASE},{"default",TOK_DEFAULT},{"class",TOK_CLASS},{"new",TOK_NEW},{"this",TOK_THIS},{"extends",TOK_EXTENDS},{"super",TOK_SUPER},{"import",TOK_IMPORT},{"as",TOK_AS},{"export",TOK_EXPORT},{NULL,TOK_UNKNOWN}};
        for(int k=0;kws[k].kw;k++)if(strcmp(tk->text,kws[k].kw)==0){tk->type=kws[k].t;return;} tk->type=TOK_IDENT;return;
    }
    char c2=(g_pos+1<g_len)?g_src[g_pos+1]:'\0';
    if(c=='='&&c2=='>'){g_pos+=2;tk->type=TOK_ARROW;strcpy(tk->text,"=>");return;}
    if(c=='.'&&c2=='.'){g_pos+=2;tk->type=TOK_RANGE;return;}
    if(c=='='&&c2=='='){g_pos+=2;tk->type=TOK_EQ;return;} if(c=='!'&&c2=='='){g_pos+=2;tk->type=TOK_NEQ;return;}
    if(c=='<'&&c2=='='){g_pos+=2;tk->type=TOK_LE;return;} if(c=='>'&&c2=='='){g_pos+=2;tk->type=TOK_GE;return;}
    if(c=='&'&&c2=='&'){g_pos+=2;tk->type=TOK_AND;return;} if(c=='|'&&c2=='|'){g_pos+=2;tk->type=TOK_OR;return;}
    if(c=='+'&&c2=='+'){g_pos+=2;tk->type=TOK_PLUSPLUS;return;} if(c=='-'&&c2=='-'){g_pos+=2;tk->type=TOK_MINUSMINUS;return;}
    if(c=='+'&&c2=='='){g_pos+=2;tk->type=TOK_PLUSEQ;return;} if(c=='-'&&c2=='='){g_pos+=2;tk->type=TOK_MINUSEQ;return;}
    if(c=='*'&&c2=='*'){g_pos+=2;tk->type=TOK_POW;return;} if(c=='*'&&c2=='='){g_pos+=2;tk->type=TOK_MULEQ;return;}
    if(c=='/'&&c2=='='){g_pos+=2;tk->type=TOK_DIVEQ;return;} if(c=='%'&&c2=='='){g_pos+=2;tk->type=TOK_MODEQ;return;}
    g_pos++;
    switch(c){case'(':tk->type=TOK_LPAREN;return;case')':tk->type=TOK_RPAREN;return;case'{':tk->type=TOK_LBRACE;return;case'}':tk->type=TOK_RBRACE;return;case'[':tk->type=TOK_LBRACKET;return;case']':tk->type=TOK_RBRACKET;return;case',':tk->type=TOK_COMMA;return;case':':tk->type=TOK_COLON;return;case';':tk->type=TOK_SEMICOLON;return;case'.':tk->type=TOK_DOT;return;case'?':tk->type=TOK_QUESTION;return;case'+':tk->type=TOK_PLUS;return;case'-':tk->type=TOK_MINUS;return;case'*':tk->type=TOK_MUL;return;case'/':tk->type=TOK_DIV;return;case'%':tk->type=TOK_MOD;return;case'=':tk->type=TOK_ASSIGN;return;case'<':tk->type=TOK_LT;return;case'>':tk->type=TOK_GT;return;case'!':tk->type=TOK_NOT;return;}
    tk->type=TOK_UNKNOWN;tk->text[0]=c;tk->text[1]='\0';
}

static void advance(void){next_tok(&g_tok);}
static int tok_accept(TokType t){if(g_tok.type==t){advance();return 1;}return 0;}
static void expect(TokType t,const char *msg){if(g_tok.type!=t)error(g_tok.line,"%s",msg);advance();}

/* Parser */
static AST *parse_expr(void);
static AST *parse_stmt(void);
static AST *parse_block(void);
static AST *ast_new(ASTType type){AST *n=xmalloc(sizeof(AST));memset(n,0,sizeof(AST));n->type=type;n->line=g_tok.line;return n;}
static void ast_add_child(AST *p,AST *c){p->children=xrealloc(p->children,sizeof(AST*)*(p->child_n+1));p->children[p->child_n++]=c;}

static int is_arrow_function(void) {
    if(g_tok.type==TOK_LPAREN){int sp=g_pos,sl=g_line;Token st=g_tok;advance();int d=1;while(g_tok.type!=TOK_EOF&&d>0){if(g_tok.type==TOK_LPAREN)d++;else if(g_tok.type==TOK_RPAREN)d--;advance();}int r=(g_tok.type==TOK_ARROW);g_pos=sp;g_line=sl;g_tok=st;return r;}
    if(g_tok.type==TOK_IDENT){int sp=g_pos,sl=g_line;Token st=g_tok;advance();int r=(g_tok.type==TOK_ARROW);g_pos=sp;g_line=sl;g_tok=st;return r;}
    return 0;
}

static AST *parse_arrow_function(void) {
    AST *fn=ast_new(AST_ARROW);fn->is_arrow=1;
    if(g_tok.type==TOK_IDENT){fn->params=xmalloc(sizeof(char*));fn->params[0]=xstrdup(g_tok.text);fn->param_n=1;advance();}
    else{expect(TOK_LPAREN,"expected '('");while(g_tok.type!=TOK_RPAREN&&g_tok.type!=TOK_EOF){if(g_tok.type!=TOK_IDENT)error(g_tok.line,"expected parameter");fn->params=xrealloc(fn->params,sizeof(char*)*(fn->param_n+1));fn->params[fn->param_n++]=xstrdup(g_tok.text);advance();if(!tok_accept(TOK_COMMA))break;}expect(TOK_RPAREN,"expected ')'");}
    expect(TOK_ARROW,"expected '=>'");
    if(g_tok.type==TOK_LBRACE)fn->body=parse_block();
    else{AST *ret=ast_new(AST_RETURN);ret->left=parse_expr();fn->body=ret;}
    return fn;
}

static AST *parse_primary(void) {
    if(is_arrow_function())return parse_arrow_function();
    if(g_tok.type==TOK_NUMBER){AST *n=ast_new(AST_LITERAL);double v=g_tok.num;n->lit=(v==(long long)v&&v>=LLONG_MIN&&v<=LLONG_MAX)?val_int((long long)v):val_float(v);advance();return n;}
    if(g_tok.type==TOK_STRING){AST *n=ast_new(AST_LITERAL);n->lit=val_string(g_tok.text);advance();return n;}
    if(g_tok.type==TOK_TEMPLATE){AST *n=ast_new(AST_TEMPLATE);n->tpl=g_tok.tpl;n->tpl_n=g_tok.tpl_n;advance();return n;}
    if(g_tok.type==TOK_TRUE){AST *n=ast_new(AST_LITERAL);n->lit=val_bool(1);advance();return n;}
    if(g_tok.type==TOK_FALSE){AST *n=ast_new(AST_LITERAL);n->lit=val_bool(0);advance();return n;}
    if(g_tok.type==TOK_NULL){AST *n=ast_new(AST_LITERAL);n->lit=val_null();advance();return n;}
    if(g_tok.type==TOK_IDENT){AST *n=ast_new(AST_VAR);n->name=xstrdup(g_tok.text);advance();return n;}
    if(g_tok.type==TOK_LPAREN){advance();AST *e=parse_expr();expect(TOK_RPAREN,"expected ')'");return e;}
    if(g_tok.type==TOK_LBRACKET){advance();AST *arr=ast_new(AST_ARRAY);while(g_tok.type!=TOK_RBRACKET&&g_tok.type!=TOK_EOF){ast_add_child(arr,parse_expr());if(!tok_accept(TOK_COMMA))break;}expect(TOK_RBRACKET,"expected ']'");return arr;}
    if(g_tok.type==TOK_LBRACE){advance();AST *obj=ast_new(AST_OBJECT);while(g_tok.type!=TOK_RBRACE&&g_tok.type!=TOK_EOF){char *key=NULL;if(g_tok.type==TOK_STRING||g_tok.type==TOK_IDENT){key=xstrdup(g_tok.text);advance();}else error(g_tok.line,"expected object key");expect(TOK_COLON,"expected ':'");obj->keys=xrealloc(obj->keys,sizeof(char*)*(obj->child_n+1));obj->keys[obj->child_n]=key;ast_add_child(obj,parse_expr());if(!tok_accept(TOK_COMMA))break;}expect(TOK_RBRACE,"expected '}'");return obj;}
    if(g_tok.type==TOK_FUNC){advance();AST *fn=ast_new(AST_FUNC);fn->name=NULL;expect(TOK_LPAREN,"expected '('");while(g_tok.type!=TOK_RPAREN&&g_tok.type!=TOK_EOF){if(g_tok.type!=TOK_IDENT)error(g_tok.line,"expected parameter");fn->params=xrealloc(fn->params,sizeof(char*)*(fn->param_n+1));fn->params[fn->param_n++]=xstrdup(g_tok.text);advance();if(!tok_accept(TOK_COMMA))break;}expect(TOK_RPAREN,"expected ')'");fn->body=parse_block();return fn;}
    if(g_tok.type==TOK_NEW){advance();AST *n=ast_new(AST_NEW);if(g_tok.type!=TOK_IDENT)error(g_tok.line,"expected class name");n->name=xstrdup(g_tok.text);advance();expect(TOK_LPAREN,"expected '('");while(g_tok.type!=TOK_RPAREN&&g_tok.type!=TOK_EOF){ast_add_child(n,parse_expr());if(!tok_accept(TOK_COMMA))break;}expect(TOK_RPAREN,"expected ')'");return n;}
    if(g_tok.type==TOK_THIS){AST *n=ast_new(AST_THIS);advance();return n;}
    if(g_tok.type==TOK_SUPER){AST *n=ast_new(AST_SUPER);advance();return n;}
    error(g_tok.line,"unexpected token '%s'",g_tok.text);return NULL;
}

static AST *parse_postfix(void) { AST *node=parse_primary(); while(1){if(g_tok.type==TOK_LBRACKET){advance();AST *idx=ast_new(AST_INDEX);idx->left=node;idx->right=parse_expr();expect(TOK_RBRACKET,"expected ']'");node=idx;}else if(g_tok.type==TOK_DOT){advance();if(g_tok.type!=TOK_IDENT)error(g_tok.line,"expected identifier");AST *mem=ast_new(AST_MEMBER);mem->left=node;mem->name=xstrdup(g_tok.text);advance();node=mem;}else if(g_tok.type==TOK_LPAREN){advance();AST *call=ast_new(AST_CALL);call->left=node;while(g_tok.type!=TOK_RPAREN&&g_tok.type!=TOK_EOF){ast_add_child(call,parse_expr());if(!tok_accept(TOK_COMMA))break;}expect(TOK_RPAREN,"expected ')'");node=call;}else if(g_tok.type==TOK_PLUSPLUS||g_tok.type==TOK_MINUSMINUS){AST *post=ast_new(AST_UNARY);strcpy(post->op,g_tok.type==TOK_PLUSPLUS?"++p":"--p");post->left=node;advance();node=post;}else break;} return node; }
static AST *parse_unary(void) { if(g_tok.type==TOK_MINUS||g_tok.type==TOK_NOT||g_tok.type==TOK_PLUS||g_tok.type==TOK_PLUSPLUS||g_tok.type==TOK_MINUSMINUS){AST *n=ast_new(AST_UNARY);switch(g_tok.type){case TOK_MINUS:strcpy(n->op,"-");break;case TOK_NOT:strcpy(n->op,"!");break;case TOK_PLUS:strcpy(n->op,"+");break;case TOK_PLUSPLUS:strcpy(n->op,"++");break;case TOK_MINUSMINUS:strcpy(n->op,"--");break;default:break;}advance();n->right=parse_unary();return n;} return parse_postfix(); }
static AST *parse_power(void){AST *node=parse_unary();if(g_tok.type==TOK_POW){AST *n=ast_new(AST_BINOP);strcpy(n->op,"**");advance();n->left=node;n->right=parse_power();return n;}return node;}
static AST *parse_mul(void){AST *node=parse_power();while(g_tok.type==TOK_MUL||g_tok.type==TOK_DIV||g_tok.type==TOK_MOD){AST *n=ast_new(AST_BINOP);strcpy(n->op,g_tok.type==TOK_MUL?"*":g_tok.type==TOK_DIV?"/":"%");advance();n->left=node;n->right=parse_power();node=n;}return node;}
static AST *parse_add(void){AST *node=parse_mul();while(g_tok.type==TOK_PLUS||g_tok.type==TOK_MINUS){AST *n=ast_new(AST_BINOP);strcpy(n->op,g_tok.type==TOK_PLUS?"+":"-");advance();n->left=node;n->right=parse_mul();node=n;}return node;}
static AST *parse_cmp(void){AST *node=parse_add();while(g_tok.type==TOK_LT||g_tok.type==TOK_GT||g_tok.type==TOK_LE||g_tok.type==TOK_GE){AST *n=ast_new(AST_BINOP);switch(g_tok.type){case TOK_LT:strcpy(n->op,"<");break;case TOK_GT:strcpy(n->op,">");break;case TOK_LE:strcpy(n->op,"<=");break;case TOK_GE:strcpy(n->op,">=");break;default:break;}advance();n->left=node;n->right=parse_add();node=n;}return node;}
static AST *parse_eq(void){AST *node=parse_cmp();while(g_tok.type==TOK_EQ||g_tok.type==TOK_NEQ){AST *n=ast_new(AST_BINOP);strcpy(n->op,g_tok.type==TOK_EQ?"==":"!=");advance();n->left=node;n->right=parse_cmp();node=n;}return node;}
static AST *parse_and(void){AST *node=parse_eq();while(g_tok.type==TOK_AND){AST *n=ast_new(AST_BINOP);strcpy(n->op,"&&");advance();n->left=node;n->right=parse_eq();node=n;}return node;}
static AST *parse_or(void){AST *node=parse_and();while(g_tok.type==TOK_OR){AST *n=ast_new(AST_BINOP);strcpy(n->op,"||");advance();n->left=node;n->right=parse_and();node=n;}return node;}
static AST *parse_ternary(void){AST *node=parse_or();if(g_tok.type==TOK_QUESTION){advance();AST *t=ast_new(AST_TERNARY);t->cond=node;t->left=parse_expr();expect(TOK_COLON,"expected ':'");t->right=parse_expr();return t;}return node;}
static AST *parse_expr(void){return parse_ternary();}
static AST *parse_block(void){expect(TOK_LBRACE,"expected '{'");AST *blk=ast_new(AST_BLOCK);while(g_tok.type!=TOK_RBRACE&&g_tok.type!=TOK_EOF){AST *s=parse_stmt();if(s)ast_add_child(blk,s);}expect(TOK_RBRACE,"expected '}'");return blk;}

static AST *parse_stmt(void) {
    while(tok_accept(TOK_SEMICOLON));
    if(g_tok.type==TOK_IMPORT){advance();AST *imp=ast_new(AST_IMPORT);if(g_tok.type!=TOK_STRING)error(g_tok.line,"expected import path");imp->imp_path=xstrdup(g_tok.text);advance();if(g_tok.type==TOK_AS){advance();if(g_tok.type!=TOK_IDENT)error(g_tok.line,"expected alias");imp->imp_alias=xstrdup(g_tok.text);advance();}else{char *slash=strrchr(imp->imp_path,'/');if(!slash)slash=strrchr(imp->imp_path,'\\');char *name=slash?slash+1:imp->imp_path;char *dot=strrchr(name,'.');int len=dot?(int)(dot-name):(int)strlen(name);imp->imp_alias=xmalloc(len+1);strncpy(imp->imp_alias,name,len);imp->imp_alias[len]='\0';}tok_accept(TOK_SEMICOLON);return imp;}
    if(g_tok.type==TOK_EXPORT){advance();return parse_stmt();}
    if(g_tok.type==TOK_IF){advance();expect(TOK_LPAREN,"expected '('");AST *n=ast_new(AST_IF);n->cond=parse_expr();expect(TOK_RPAREN,"expected ')'");n->body=parse_block();if(g_tok.type==TOK_ELSE){advance();if(g_tok.type==TOK_IF){AST *eb=ast_new(AST_BLOCK);ast_add_child(eb,parse_stmt());n->right=eb;}else n->right=parse_block();}return n;}
    if(g_tok.type==TOK_WHILE){advance();expect(TOK_LPAREN,"expected '('");AST *n=ast_new(AST_WHILE);n->cond=parse_expr();expect(TOK_RPAREN,"expected ')'");n->body=parse_block();return n;}
    if(g_tok.type==TOK_FOR){advance();int has_parens=tok_accept(TOK_LPAREN);if(g_tok.type==TOK_IDENT){char *first=xstrdup(g_tok.text);advance();if(g_tok.type==TOK_IN||g_tok.type==TOK_COMMA){AST *n=ast_new(AST_FOR);n->iter_var=first;if(g_tok.type==TOK_COMMA){advance();if(g_tok.type!=TOK_IDENT)error(g_tok.line,"expected index var");n->idx_var=xstrdup(g_tok.text);advance();}else n->idx_var=xstrdup("_idx");expect(TOK_IN,"expected 'in'");n->iter_expr=parse_expr();if(has_parens)expect(TOK_RPAREN,"expected ')'");n->body=parse_block();return n;}else{AST *n=ast_new(AST_FOR_C);if(g_tok.type==TOK_ASSIGN){advance();AST *asgn=ast_new(AST_ASSIGN);asgn->name=xstrdup(first);asgn->left=parse_expr();n->init=asgn;}else{AST *var=ast_new(AST_VAR);var->name=first;n->init=var;}expect(TOK_SEMICOLON,"expected ';'");n->cond=parse_expr();expect(TOK_SEMICOLON,"expected ';'");n->update=parse_stmt();if(has_parens)expect(TOK_RPAREN,"expected ')'");n->body=parse_block();return n;}}else error(g_tok.line,"expected identifier in for");}
    if(g_tok.type==TOK_CLASS){advance();AST *n=ast_new(AST_CLASS);if(g_tok.type!=TOK_IDENT)error(g_tok.line,"expected class name");n->name=xstrdup(g_tok.text);advance();if(g_tok.type==TOK_EXTENDS){advance();if(g_tok.type!=TOK_IDENT)error(g_tok.line,"expected parent class");n->parent=xstrdup(g_tok.text);advance();}expect(TOK_LBRACE,"expected '{'");while(g_tok.type!=TOK_RBRACE&&g_tok.type!=TOK_EOF){if(g_tok.type==TOK_FUNC||g_tok.type==TOK_IDENT){AST *m=ast_new(AST_FUNC);if(g_tok.type==TOK_FUNC)advance();if(g_tok.type!=TOK_IDENT)error(g_tok.line,"expected method name");m->name=xstrdup(g_tok.text);advance();expect(TOK_LPAREN,"expected '('");while(g_tok.type!=TOK_RPAREN&&g_tok.type!=TOK_EOF){if(g_tok.type!=TOK_IDENT)error(g_tok.line,"expected parameter");m->params=xrealloc(m->params,sizeof(char*)*(m->param_n+1));m->params[m->param_n++]=xstrdup(g_tok.text);advance();if(!tok_accept(TOK_COMMA))break;}expect(TOK_RPAREN,"expected ')'");m->body=parse_block();n->methods=xrealloc(n->methods,sizeof(AST*)*(n->method_n+1));n->methods[n->method_n++]=m;}else error(g_tok.line,"expected method in class");}expect(TOK_RBRACE,"expected '}'");return n;}
    if(g_tok.type==TOK_FUNC){advance();AST *n=ast_new(AST_FUNC);if(g_tok.type!=TOK_IDENT)error(g_tok.line,"expected function name");n->name=xstrdup(g_tok.text);advance();expect(TOK_LPAREN,"expected '('");while(g_tok.type!=TOK_RPAREN&&g_tok.type!=TOK_EOF){if(g_tok.type!=TOK_IDENT)error(g_tok.line,"expected parameter");n->params=xrealloc(n->params,sizeof(char*)*(n->param_n+1));n->params[n->param_n++]=xstrdup(g_tok.text);advance();if(!tok_accept(TOK_COMMA))break;}expect(TOK_RPAREN,"expected ')'");n->body=parse_block();return n;}
    if(g_tok.type==TOK_CONST){advance();if(g_tok.type!=TOK_IDENT)error(g_tok.line,"expected identifier");AST *n=ast_new(AST_ASSIGN);n->name=xstrdup(g_tok.text);n->is_const=1;advance();expect(TOK_ASSIGN,"expected '='");n->left=parse_expr();return n;}
    if(g_tok.type==TOK_RETURN){advance();AST *n=ast_new(AST_RETURN);if(g_tok.type!=TOK_RBRACE&&g_tok.type!=TOK_SEMICOLON&&g_tok.type!=TOK_EOF)n->left=parse_expr();tok_accept(TOK_SEMICOLON);return n;}
    if(g_tok.type==TOK_BREAK){AST *n=ast_new(AST_BREAK);advance();tok_accept(TOK_SEMICOLON);return n;}
    if(g_tok.type==TOK_CONTINUE){AST *n=ast_new(AST_CONTINUE);advance();tok_accept(TOK_SEMICOLON);return n;}
    if(g_tok.type==TOK_CASE){advance();expect(TOK_LPAREN,"expected '('");AST *n=ast_new(AST_CASE);n->cond=parse_expr();expect(TOK_RPAREN,"expected ')'");expect(TOK_LBRACE,"expected '{'");while(g_tok.type!=TOK_RBRACE&&g_tok.type!=TOK_EOF){AST *c=ast_new(AST_BLOCK);if(tok_accept(TOK_DEFAULT)){c->cond=NULL;expect(TOK_COLON,"expected ':'");}else{AST *matches=ast_new(AST_ARRAY);do{AST *val=parse_primary();if(g_tok.type==TOK_RANGE){advance();AST *rng=ast_new(AST_BINOP);strcpy(rng->op,"..");rng->left=val;rng->right=parse_primary();ast_add_child(matches,rng);}else{ast_add_child(matches,val);}}while(tok_accept(TOK_COMMA));c->cond=matches;expect(TOK_COLON,"expected ':'");}while(g_tok.type!=TOK_RBRACE&&g_tok.type!=TOK_EOF&&g_tok.type!=TOK_DEFAULT){int sp=g_pos,sl=g_line;Token st=g_tok;if(g_tok.type==TOK_NUMBER||g_tok.type==TOK_STRING||g_tok.type==TOK_IDENT||g_tok.type==TOK_MINUS){advance();if(g_tok.type==TOK_NUMBER)advance();if(g_tok.type==TOK_COLON||g_tok.type==TOK_RANGE||g_tok.type==TOK_COMMA){g_pos=sp;g_line=sl;g_tok=st;break;}g_pos=sp;g_line=sl;g_tok=st;}AST *s=parse_stmt();if(s)ast_add_child(c,s);}ast_add_child(n,c);}expect(TOK_RBRACE,"expected '}'");return n;}
    AST *expr=parse_expr();
    if(g_tok.type==TOK_ASSIGN){advance();AST *rhs=parse_expr();if(expr->type==AST_VAR){AST *n=ast_new(AST_ASSIGN);n->name=xstrdup(expr->name);n->left=rhs;return n;}else if(expr->type==AST_INDEX||expr->type==AST_MEMBER){AST *n=ast_new(AST_INDEX_ASSIGN);n->left=expr;n->right=rhs;return n;}else error(expr->line,"invalid assignment target");}
    if(g_tok.type==TOK_PLUSEQ||g_tok.type==TOK_MINUSEQ||g_tok.type==TOK_MULEQ||g_tok.type==TOK_DIVEQ||g_tok.type==TOK_MODEQ){AST *n=ast_new(AST_COMPOUND);switch(g_tok.type){case TOK_PLUSEQ:strcpy(n->op,"+");break;case TOK_MINUSEQ:strcpy(n->op,"-");break;case TOK_MULEQ:strcpy(n->op,"*");break;case TOK_DIVEQ:strcpy(n->op,"/");break;case TOK_MODEQ:strcpy(n->op,"%");break;default:break;}advance();n->left=expr;n->right=parse_expr();return n;}
    tok_accept(TOK_SEMICOLON);AST *stmt=ast_new(AST_EXPR_STMT);stmt->left=expr;return stmt;
}
static AST *parse_program(void){AST *prog=ast_new(AST_BLOCK);while(g_tok.type!=TOK_EOF){AST *s=parse_stmt();if(s)ast_add_child(prog,s);}return prog;}

/* Builtins */
static Value *bi_print(AST **args,int argc){for(int i=0;i<argc;i++){Value *v=eval(args[i]);printf("%s",val_tostr(v));if(i<argc-1)printf(" ");val_decref(v);}printf("\n");return val_null();}
static Value *bi_len(Value **a,int n){if(n!=1)return val_int(0);switch(a[0]->type){case V_STRING:return val_int(strlen(a[0]->as.s));case V_ARRAY:return val_int(a[0]->as.arr->len);case V_OBJECT:return val_int(a[0]->as.obj->count);default:return val_int(0);}}
static Value *bi_typeof(Value **a,int n){if(n!=1)return val_string("undefined");const char *t[]={"null","boolean","integer","float","string","array","object","function","class","instance","module","closure"};return val_string(t[a[0]->type]);}
static Value *bi_keys(Value **a,int n){if(n!=1||(a[0]->type!=V_OBJECT&&a[0]->type!=V_MODULE))return val_array(0);Value *arr=val_array(a[0]->as.obj->count);for(int i=0;i<a[0]->as.obj->count;i++){Value *k=val_string(a[0]->as.obj->pairs[i].key);arr_push(arr,k);val_decref(k);}return arr;}
static Value *bi_values(Value **a,int n){if(n!=1||(a[0]->type!=V_OBJECT&&a[0]->type!=V_MODULE))return val_array(0);Value *arr=val_array(a[0]->as.obj->count);for(int i=0;i<a[0]->as.obj->count;i++){Value *v=val_copy(a[0]->as.obj->pairs[i].val);arr_push(arr,v);val_decref(v);}return arr;}
static Value *bi_push(Value **a,int n){if(n!=2||a[0]->type!=V_ARRAY)return val_null();arr_push(a[0],a[1]);return val_int(a[0]->as.arr->len);}
static Value *bi_pop(Value **a,int n){if(n!=1||a[0]->type!=V_ARRAY)return val_null();return arr_pop(a[0]);}
static Value *bi_input(Value **a,int n){if(n>0&&a[0]->type==V_STRING){printf("%s",a[0]->as.s);fflush(stdout);}char buf[4096];if(!fgets(buf,sizeof(buf),stdin))return val_string("");buf[strcspn(buf,"\n")]='\0';return val_string(buf);}
static Value *bi_str(Value **a,int n){return n==1?val_string(val_tostr(a[0])):val_string("");}
static Value *bi_int_fn(Value **a,int n){if(n!=1)return val_int(0);switch(a[0]->type){case V_INT:return val_int(a[0]->as.i);case V_FLOAT:return val_int((long long)a[0]->as.f);case V_STRING:return val_int(atoll(a[0]->as.s));case V_BOOL:return val_int(a[0]->as.b);default:return val_int(0);}}
static Value *bi_float_fn(Value **a,int n){return n==1?val_float(val_tonum(a[0])):val_float(0);}
static Value *bi_range(Value **a,int n){long long start=0,stop=0,step=1;if(n==1)stop=(long long)val_tonum(a[0]);else if(n==2){start=(long long)val_tonum(a[0]);stop=(long long)val_tonum(a[1]);}else if(n>=3){start=(long long)val_tonum(a[0]);stop=(long long)val_tonum(a[1]);step=(long long)val_tonum(a[2]);}if(step==0)step=1;Value *arr=val_array(16);if(step>0)for(long long i=start;i<stop;i+=step){Value *v=val_int(i);arr_push(arr,v);val_decref(v);}else for(long long i=start;i>stop;i+=step){Value *v=val_int(i);arr_push(arr,v);val_decref(v);}return arr;}
static Value *bi_join(Value **a,int n){if(n<1||a[0]->type!=V_ARRAY)return val_string("");const char *sep=(n>=2&&a[1]->type==V_STRING)?a[1]->as.s:"";char buf[MAX_STR];int pos=0;for(int i=0;i<a[0]->as.arr->len&&pos<MAX_STR-100;i++){if(i>0){int sl=strlen(sep);memcpy(buf+pos,sep,sl);pos+=sl;}const char *s=val_tostr(a[0]->as.arr->items[i]);int sl=strlen(s);if(pos+sl<MAX_STR-1){memcpy(buf+pos,s,sl);pos+=sl;}}buf[pos]='\0';return val_string(buf);}
static Value *bi_split(Value **a,int n){if(n<1||a[0]->type!=V_STRING)return val_array(0);const char *sep=(n>=2&&a[1]->type==V_STRING)?a[1]->as.s:" ";Value *arr=val_array(8);char *str=xstrdup(a[0]->as.s);if(strlen(sep)==0){for(size_t i=0;i<strlen(a[0]->as.s);i++){char c[2]={a[0]->as.s[i],'\0'};Value *v=val_string(c);arr_push(arr,v);val_decref(v);}}else{char *tok=strtok(str,sep);while(tok){Value *v=val_string(tok);arr_push(arr,v);val_decref(v);tok=strtok(NULL,sep);}}free(str);return arr;}
static Value *bi_trim(Value **a,int n){if(n!=1||a[0]->type!=V_STRING)return val_string("");char *s=a[0]->as.s;while(*s&&isspace((unsigned char)*s))s++;if(!*s)return val_string("");char *e=s+strlen(s)-1;while(e>s&&isspace((unsigned char)*e))e--;int len=e-s+1;char *r=xmalloc(len+1);memcpy(r,s,len);r[len]='\0';Value *v=val_string(r);free(r);return v;}
static Value *bi_upper(Value **a,int n){if(n!=1||a[0]->type!=V_STRING)return val_string("");char *r=xstrdup(a[0]->as.s);for(char *p=r;*p;p++)*p=toupper((unsigned char)*p);Value *v=val_string(r);free(r);return v;}
static Value *bi_lower(Value **a,int n){if(n!=1||a[0]->type!=V_STRING)return val_string("");char *r=xstrdup(a[0]->as.s);for(char *p=r;*p;p++)*p=tolower((unsigned char)*p);Value *v=val_string(r);free(r);return v;}
static Value *bi_replace(Value **a,int n){if(n<3||a[0]->type!=V_STRING||a[1]->type!=V_STRING||a[2]->type!=V_STRING)return val_string("");char *str=a[0]->as.s,*old=a[1]->as.s,*new_s=a[2]->as.s;if(!strlen(old))return val_string(str);int count=0;char *p=str;while((p=strstr(p,old))){count++;p+=strlen(old);}if(!count)return val_string(str);size_t nl=strlen(str)+count*(strlen(new_s)-strlen(old));char *result=xmalloc(nl+1),*out=result;p=str;char *pos;while((pos=strstr(p,old))){memcpy(out,p,pos-p);out+=pos-p;memcpy(out,new_s,strlen(new_s));out+=strlen(new_s);p=pos+strlen(old);}strcpy(out,p);Value *v=val_string(result);free(result);return v;}
static Value *bi_indexOf(Value **a,int n){if(n<2)return val_int(-1);if(a[0]->type==V_STRING&&a[1]->type==V_STRING){char *found=strstr(a[0]->as.s,a[1]->as.s);return val_int(found?(found-a[0]->as.s):-1);}if(a[0]->type==V_ARRAY){for(int i=0;i<a[0]->as.arr->len;i++){Value *item=a[0]->as.arr->items[i];if(item->type==a[1]->type){if(item->type==V_INT&&item->as.i==a[1]->as.i)return val_int(i);if(item->type==V_STRING&&strcmp(item->as.s,a[1]->as.s)==0)return val_int(i);}}return val_int(-1);}return val_int(-1);}
static Value *bi_slice(Value **a,int n){if(n<1)return val_null();int start=(n>=2)?(int)val_tonum(a[1]):0;int end=(n>=3)?(int)val_tonum(a[2]):INT_MAX;if(a[0]->type==V_STRING){int len=strlen(a[0]->as.s);if(start<0)start=len+start;if(end<0)end=len+end;if(start<0)start=0;if(end>len)end=len;if(start>=end)return val_string("");int sl=end-start;char *r=xmalloc(sl+1);memcpy(r,a[0]->as.s+start,sl);r[sl]='\0';Value *v=val_string(r);free(r);return v;}if(a[0]->type==V_ARRAY){int len=a[0]->as.arr->len;if(start<0)start=len+start;if(end<0)end=len+end;if(start<0)start=0;if(end>len)end=len;if(start>=end)return val_array(0);Value *arr=val_array(end-start);for(int i=start;i<end;i++){Value *item=val_copy(a[0]->as.arr->items[i]);arr_push(arr,item);val_decref(item);}return arr;}return val_null();}
static Value *bi_reverse(Value **a,int n){if(n!=1)return val_null();if(a[0]->type==V_STRING){int len=strlen(a[0]->as.s);char *r=xmalloc(len+1);for(int i=0;i<len;i++)r[i]=a[0]->as.s[len-1-i];r[len]='\0';Value *v=val_string(r);free(r);return v;}if(a[0]->type==V_ARRAY){Array *ar=a[0]->as.arr;for(int i=0;i<ar->len/2;i++){Value *tmp=ar->items[i];ar->items[i]=ar->items[ar->len-1-i];ar->items[ar->len-1-i]=tmp;}val_incref(a[0]);return a[0];}return val_null();}
static int cmp_vals(const void *x,const void *y){Value *va=*(Value**)x,*vb=*(Value**)y;if(va->type==V_INT&&vb->type==V_INT)return(va->as.i>vb->as.i)-(va->as.i<vb->as.i);if(va->type==V_FLOAT||vb->type==V_FLOAT){double fa=val_tonum(va),fb=val_tonum(vb);return(fa>fb)-(fa<fb);}if(va->type==V_STRING&&vb->type==V_STRING)return strcmp(va->as.s,vb->as.s);return 0;}
static Value *bi_sort(Value **a,int n){if(n!=1||a[0]->type!=V_ARRAY)return val_null();qsort(a[0]->as.arr->items,a[0]->as.arr->len,sizeof(Value*),cmp_vals);val_incref(a[0]);return a[0];}
static Value *bi_abs(Value **a,int n){if(n!=1)return val_int(0);if(a[0]->type==V_INT)return val_int(llabs(a[0]->as.i));return val_float(fabs(val_tonum(a[0])));}
static Value *bi_min(Value **a,int n){if(n==0)return val_null();if(n==1&&a[0]->type==V_ARRAY){if(a[0]->as.arr->len==0)return val_null();double m=val_tonum(a[0]->as.arr->items[0]);for(int i=1;i<a[0]->as.arr->len;i++){double v=val_tonum(a[0]->as.arr->items[i]);if(v<m)m=v;}return val_float(m);}double m=val_tonum(a[0]);for(int i=1;i<n;i++){double v=val_tonum(a[i]);if(v<m)m=v;}return val_float(m);}
static Value *bi_max(Value **a,int n){if(n==0)return val_null();if(n==1&&a[0]->type==V_ARRAY){if(a[0]->as.arr->len==0)return val_null();double m=val_tonum(a[0]->as.arr->items[0]);for(int i=1;i<a[0]->as.arr->len;i++){double v=val_tonum(a[0]->as.arr->items[i]);if(v>m)m=v;}return val_float(m);}double m=val_tonum(a[0]);for(int i=1;i<n;i++){double v=val_tonum(a[i]);if(v>m)m=v;}return val_float(m);}
static Value *bi_floor(Value **a,int n){return n==1?val_int((long long)floor(val_tonum(a[0]))):val_int(0);}
static Value *bi_ceil(Value **a,int n){return n==1?val_int((long long)ceil(val_tonum(a[0]))):val_int(0);}
static Value *bi_round(Value **a,int n){return n==1?val_int((long long)round(val_tonum(a[0]))):val_int(0);}
static Value *bi_sqrt(Value **a,int n){return n==1?val_float(sqrt(val_tonum(a[0]))):val_float(0);}
static Value *bi_pow_fn(Value **a,int n){return n==2?val_float(pow(val_tonum(a[0]),val_tonum(a[1]))):val_float(0);}
static Value *bi_random(Value **a,int n){(void)a;(void)n;if(!g_rand_init){srand((unsigned)time(NULL));g_rand_init=1;}return val_float((double)rand()/RAND_MAX);}
static Value *bi_randomInt(Value **a,int n){if(!g_rand_init){srand((unsigned)time(NULL));g_rand_init=1;}if(n==1)return val_int(rand()%(int)val_tonum(a[0]));if(n>=2){int mn=(int)val_tonum(a[0]),mx=(int)val_tonum(a[1]);return val_int(mn+rand()%(mx-mn));}return val_int(rand());}
static Value *bi_hasKey(Value **a,int n){if(n!=2||(a[0]->type!=V_OBJECT&&a[0]->type!=V_MODULE)||a[1]->type!=V_STRING)return val_bool(0);return val_bool(obj_has(a[0],a[1]->as.s));}
static Value *bi_delete(Value **a,int n){if(n!=2||(a[0]->type!=V_OBJECT&&a[0]->type!=V_MODULE)||a[1]->type!=V_STRING)return val_null();obj_del(a[0],a[1]->as.s);return val_null();}
static Value *bi_clone(Value **a,int n){return n==1?val_clone(a[0]):val_null();}
static Value *bi_isArray(Value **a,int n){return n==1?val_bool(a[0]->type==V_ARRAY):val_bool(0);}
static Value *bi_isObject(Value **a,int n){return n==1?val_bool(a[0]->type==V_OBJECT||a[0]->type==V_MODULE):val_bool(0);}
static Value *bi_isString(Value **a,int n){return n==1?val_bool(a[0]->type==V_STRING):val_bool(0);}
static Value *bi_isNumber(Value **a,int n){return n==1?val_bool(a[0]->type==V_INT||a[0]->type==V_FLOAT):val_bool(0);}
static Value *bi_isFunc(Value **a,int n){return n==1?val_bool(a[0]->type==V_FUNC||a[0]->type==V_CLOSURE):val_bool(0);}
static Value *bi_assert(Value **a,int n){if(n<1)return val_null();if(!val_truthy(a[0])){const char *msg=(n>=2&&a[1]->type==V_STRING)?a[1]->as.s:"Assertion failed";fprintf(stderr,"Assertion failed: %s\n",msg);exit(1);}return val_bool(1);}
static Value *bi_sleep(Value **a,int n){if(n!=1)return val_null();long long ms=(long long)val_tonum(a[0]);if(ms>0)sleep_ms((unsigned)ms);return val_null();}

/* Timer functions */
static long long current_time_ms(void) {
#ifdef _WIN32
    FILETIME ft;GetSystemTimeAsFileTime(&ft);ULARGE_INTEGER uli;uli.LowPart=ft.dwLowDateTime;uli.HighPart=ft.dwHighDateTime;return (long long)(uli.QuadPart/10000ULL-11644473600000ULL);
#else
    struct timespec ts;clock_gettime(CLOCK_REALTIME,&ts);return (long long)ts.tv_sec*1000LL+(long long)ts.tv_nsec/1000000LL;
#endif
}
static int add_timer(Value *callback, int delay_ms, int interval) {
    if(g_timer_n>=g_timer_cap){g_timer_cap=g_timer_cap?g_timer_cap*2:16;g_timers=xrealloc(g_timers,sizeof(Timer)*g_timer_cap);}
    int id=g_timer_next_id++;
    g_timers[g_timer_n].id=id;
    g_timers[g_timer_n].fire_at=current_time_ms()+delay_ms;
    g_timers[g_timer_n].interval=interval;
    g_timers[g_timer_n].active=1;
    val_incref(callback);
    g_timers[g_timer_n].callback=callback;
    g_timer_n++;
    return id;
}
static Value *bi_setTimeout(Value **a,int n){if(n<2||(a[0]->type!=V_CLOSURE&&a[0]->type!=V_FUNC))return val_int(0);int ms=(int)val_tonum(a[1]);return val_int(add_timer(a[0],ms,0));}
static Value *bi_setInterval(Value **a,int n){if(n<2||(a[0]->type!=V_CLOSURE&&a[0]->type!=V_FUNC))return val_int(0);int ms=(int)val_tonum(a[1]);return val_int(add_timer(a[0],ms,ms));}
static Value *bi_clearTimeout(Value **a,int n){if(n<1)return val_bool(0);int id=(int)val_tonum(a[0]);for(int i=0;i<g_timer_n;i++){if(g_timers[i].id==id){g_timers[i].active=0;return val_bool(1);}}return val_bool(0);}
static Value *bi_clearInterval(Value **a,int n){return bi_clearTimeout(a,n);}
static void process_timers(void) {
    long long now=current_time_ms();
    for(int i=0;i<g_timer_n;i++) {
        if(g_timers[i].active && now>=g_timers[i].fire_at) {
            Value *cb=g_timers[i].callback;
            if(g_timers[i].interval>0) g_timers[i].fire_at=now+g_timers[i].interval;
            else g_timers[i].active=0;
            call_closure(cb,NULL,0);
        }
    }
}
static int has_active_timers(void) { for(int i=0;i<g_timer_n;i++)if(g_timers[i].active)return 1; return 0; }
static Value *bi_runTimers(Value **a,int n){(void)a;(void)n;while(has_active_timers()){process_timers();sleep_ms(10);}return val_null();}
static Value *bi_time(Value **a,int n){(void)a;(void)n;return val_int((long long)time(NULL));}
static Value *bi_now(Value **a,int n){(void)a;(void)n;
#ifdef _WIN32
FILETIME ft;GetSystemTimeAsFileTime(&ft);ULARGE_INTEGER uli;uli.LowPart=ft.dwLowDateTime;uli.HighPart=ft.dwHighDateTime;return val_float((double)(uli.QuadPart/10000ULL-11644473600000ULL));
#else
struct timespec ts;clock_gettime(CLOCK_REALTIME,&ts);return val_float((double)ts.tv_sec*1000.0+(double)ts.tv_nsec/1000000.0);
#endif
}
static Value *bi_readFile(Value **a,int n){if(n!=1||a[0]->type!=V_STRING)return val_null();FILE *f=fopen(a[0]->as.s,"rb");if(!f)return val_null();fseek(f,0,SEEK_END);long sz=ftell(f);fseek(f,0,SEEK_SET);char *buf=xmalloc(sz+1);size_t rd=fread(buf,1,sz,f);buf[rd]='\0';fclose(f);Value *v=val_string(buf);free(buf);return v;}
static Value *bi_writeFile(Value **a,int n){if(n!=2||a[0]->type!=V_STRING)return val_bool(0);FILE *f=fopen(a[0]->as.s,"wb");if(!f)return val_bool(0);const char *c=val_tostr(a[1]);size_t len=strlen(c),written=fwrite(c,1,len,f);fclose(f);return val_bool(written==len);}
static Value *bi_appendFile(Value **a,int n){if(n!=2||a[0]->type!=V_STRING)return val_bool(0);FILE *f=fopen(a[0]->as.s,"ab");if(!f)return val_bool(0);const char *c=val_tostr(a[1]);size_t len=strlen(c),written=fwrite(c,1,len,f);fclose(f);return val_bool(written==len);}
static Value *bi_fileExists(Value **a,int n){if(n!=1||a[0]->type!=V_STRING)return val_bool(0);FILE *f=fopen(a[0]->as.s,"r");if(f){fclose(f);return val_bool(1);}return val_bool(0);}
static Value *bi_remove(Value **a,int n){if(n!=1||a[0]->type!=V_STRING)return val_bool(0);return val_bool(remove(a[0]->as.s)==0);}
static Value *bi_mkdir(Value **a,int n){if(n!=1||a[0]->type!=V_STRING)return val_bool(0);
#ifdef _WIN32
return val_bool(_mkdir(a[0]->as.s)==0);
#else
return val_bool(mkdir(a[0]->as.s,0755)==0);
#endif
}
static Value *bi_rename(Value **a,int n){if(n!=2||a[0]->type!=V_STRING||a[1]->type!=V_STRING)return val_bool(0);return val_bool(rename(a[0]->as.s,a[1]->as.s)==0);}

/* JSON */
static Value *parse_json(const char **p);
static void json_skip_ws(const char **p){while(**p&&isspace((unsigned char)**p))(*p)++;}
static Value *parse_json_str(const char **p){(*p)++;char buf[MAX_TOK];int i=0;while(**p&&**p!='"'&&i<MAX_TOK-1){if(**p=='\\'&&*(*p+1)){(*p)++;switch(**p){case 'n':buf[i++]='\n';break;case 't':buf[i++]='\t';break;case 'r':buf[i++]='\r';break;case '"':case '\\':buf[i++]=**p;break;default:buf[i++]=**p;}}else buf[i++]=**p;(*p)++;}buf[i]='\0';if(**p=='"')(*p)++;return val_string(buf);}
static Value *parse_json_num(const char **p){char buf[64];int i=0,has_dot=0;if(**p=='-')buf[i++]=*(*p)++;while(**p&&(isdigit((unsigned char)**p)||**p=='.'||**p=='e'||**p=='E'||**p=='+'||**p=='-')){if(**p=='.')has_dot=1;if(i<63)buf[i++]=**p;(*p)++;}buf[i]='\0';return(has_dot||strchr(buf,'e')||strchr(buf,'E'))?val_float(atof(buf)):val_int(atoll(buf));}
static Value *parse_json_arr(const char **p){(*p)++;Value *arr=val_array(8);json_skip_ws(p);while(**p&&**p!=']'){Value *item=parse_json(p);if(item){arr_push(arr,item);val_decref(item);}json_skip_ws(p);if(**p==',')(*p)++;json_skip_ws(p);}if(**p==']')(*p)++;return arr;}
static Value *parse_json_obj(const char **p){(*p)++;Value *obj=val_object();json_skip_ws(p);while(**p&&**p!='}'){json_skip_ws(p);if(**p!='"')break;Value *key=parse_json_str(p);json_skip_ws(p);if(**p==':')(*p)++;json_skip_ws(p);Value *val=parse_json(p);if(key&&val&&key->type==V_STRING)obj_set(obj,key->as.s,val);if(key)val_decref(key);if(val)val_decref(val);json_skip_ws(p);if(**p==',')(*p)++;}if(**p=='}')(*p)++;return obj;}
static Value *parse_json(const char **p){json_skip_ws(p);if(**p=='"')return parse_json_str(p);if(**p=='[')return parse_json_arr(p);if(**p=='{')return parse_json_obj(p);if(**p=='-'||isdigit((unsigned char)**p))return parse_json_num(p);if(strncmp(*p,"true",4)==0){*p+=4;return val_bool(1);}if(strncmp(*p,"false",5)==0){*p+=5;return val_bool(0);}if(strncmp(*p,"null",4)==0){*p+=4;return val_null();}return val_null();}
static Value *bi_jsonParse(Value **a,int n){if(n!=1||a[0]->type!=V_STRING)return val_null();const char *p=a[0]->as.s;return parse_json(&p);}
static void json_stringify(Value *v,char *buf,int *pos,int size,int indent,int cur);
static void json_indent(char *buf,int *pos,int size,int n){for(int i=0;i<n&&*pos<size-1;i++)buf[(*pos)++]=' ';}
static void json_stringify(Value *v,char *buf,int *pos,int size,int indent,int cur){if(*pos>=size-100)return;switch(v->type){case V_NULL:*pos+=snprintf(buf+*pos,size-*pos,"null");break;case V_BOOL:*pos+=snprintf(buf+*pos,size-*pos,"%s",v->as.b?"true":"false");break;case V_INT:*pos+=snprintf(buf+*pos,size-*pos,"%lld",v->as.i);break;case V_FLOAT:*pos+=snprintf(buf+*pos,size-*pos,"%g",v->as.f);break;case V_STRING:buf[(*pos)++]='"';for(char *s=v->as.s;*s&&*pos<size-10;s++){if(*s=='"'||*s=='\\')buf[(*pos)++]='\\';else if(*s=='\n'){buf[(*pos)++]='\\';buf[(*pos)++]='n';continue;}else if(*s=='\t'){buf[(*pos)++]='\\';buf[(*pos)++]='t';continue;}buf[(*pos)++]=*s;}buf[(*pos)++]='"';break;case V_ARRAY:buf[(*pos)++]='[';for(int i=0;i<v->as.arr->len&&*pos<size-100;i++){if(i>0)buf[(*pos)++]=',';if(indent>0){buf[(*pos)++]='\n';json_indent(buf,pos,size,cur+indent);}json_stringify(v->as.arr->items[i],buf,pos,size,indent,cur+indent);}if(indent>0&&v->as.arr->len>0){buf[(*pos)++]='\n';json_indent(buf,pos,size,cur);}buf[(*pos)++]=']';break;case V_OBJECT:case V_MODULE:buf[(*pos)++]='{';for(int i=0;i<v->as.obj->count&&*pos<size-100;i++){if(i>0)buf[(*pos)++]=',';if(indent>0){buf[(*pos)++]='\n';json_indent(buf,pos,size,cur+indent);}buf[(*pos)++]='"';for(char *s=v->as.obj->pairs[i].key;*s&&*pos<size-10;s++)buf[(*pos)++]=*s;buf[(*pos)++]='"';buf[(*pos)++]=':';if(indent>0)buf[(*pos)++]=' ';json_stringify(v->as.obj->pairs[i].val,buf,pos,size,indent,cur+indent);}if(indent>0&&v->as.obj->count>0){buf[(*pos)++]='\n';json_indent(buf,pos,size,cur);}buf[(*pos)++]='}';break;default:*pos+=snprintf(buf+*pos,size-*pos,"null");}}
static Value *bi_jsonStringify(Value **a,int n){if(n<1)return val_string("");int indent=(n>=2)?(int)val_tonum(a[1]):0;char buf[MAX_STR];int pos=0;json_stringify(a[0],buf,&pos,sizeof(buf),indent,0);buf[pos]='\0';return val_string(buf);}

/* HTTP Client */
static void init_sockets(void){
#ifdef _WIN32
if(!g_ws_init){WSADATA wsa;WSAStartup(MAKEWORD(2,2),&wsa);g_ws_init=1;}
#endif
}
static Value *http_request(const char *method, char *url, const char *body, const char *contentType){
    init_sockets();char host[256]={0},path[1024]="/";int port=80;
    if(strncmp(url,"http://",7)==0)url+=7;else if(strncmp(url,"https://",8)==0){url+=8;port=443;}
    char *slash=strchr(url,'/');if(slash){strncpy(host,url,slash-url);strcpy(path,slash);}else strcpy(host,url);
    char *colon=strchr(host,':');if(colon){*colon='\0';port=atoi(colon+1);}
    struct hostent *he=gethostbyname(host);if(!he)return val_null();
    SOCKET sock=socket(AF_INET,SOCK_STREAM,0);if(sock==INVALID_SOCKET)return val_null();
    struct sockaddr_in addr;addr.sin_family=AF_INET;addr.sin_port=htons(port);memcpy(&addr.sin_addr,he->h_addr,he->h_length);
    if(connect(sock,(struct sockaddr*)&addr,sizeof(addr))<0){close(sock);return val_null();}
    char req[HTTP_BUF];
    if(body&&strlen(body)>0){
        snprintf(req,sizeof(req),"%s %s HTTP/1.1\r\nHost: %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s",method,path,host,contentType,(int)strlen(body),body);
    }else{
        snprintf(req,sizeof(req),"%s %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nUser-Agent: Jeem/3.0\r\n\r\n",method,path,host);
    }
    send(sock,req,strlen(req),0);
    char *resp=xmalloc(HTTP_BUF);int total=0,bytes;
    while((bytes=recv(sock,resp+total,HTTP_BUF-total-1,0))>0){total+=bytes;if(total>=HTTP_BUF-1)break;}
    resp[total]='\0';close(sock);
    Value *result=val_object();
    char *respBody=strstr(resp,"\r\n\r\n");if(respBody){respBody+=4;Value *bv=val_string(respBody);obj_set(result,"body",bv);val_decref(bv);}
    int status=0;sscanf(resp,"HTTP/%*s %d",&status);Value *sv=val_int(status);obj_set(result,"status",sv);val_decref(sv);
    free(resp);return result;
}
static Value *bi_httpGet(Value **a,int n){if(n<1||a[0]->type!=V_STRING)return val_null();return http_request("GET",a[0]->as.s,NULL,NULL);}
static Value *bi_httpPost(Value **a,int n){if(n<1||a[0]->type!=V_STRING)return val_null();const char *body=(n>=2)?val_tostr(a[1]):"";const char *ct=(n>=3&&a[2]->type==V_STRING)?a[2]->as.s:"application/json";return http_request("POST",a[0]->as.s,body,ct);}
static Value *bi_httpPut(Value **a,int n){if(n<1||a[0]->type!=V_STRING)return val_null();const char *body=(n>=2)?val_tostr(a[1]):"";const char *ct=(n>=3&&a[2]->type==V_STRING)?a[2]->as.s:"application/json";return http_request("PUT",a[0]->as.s,body,ct);}
static Value *bi_httpDelete(Value **a,int n){if(n<1||a[0]->type!=V_STRING)return val_null();return http_request("DELETE",a[0]->as.s,NULL,NULL);}
static Value *bi_httpPatch(Value **a,int n){if(n<1||a[0]->type!=V_STRING)return val_null();const char *body=(n>=2)?val_tostr(a[1]):"";const char *ct=(n>=3&&a[2]->type==V_STRING)?a[2]->as.s:"application/json";return http_request("PATCH",a[0]->as.s,body,ct);}
static Value *bi_http(Value **a,int n){if(n<2||a[0]->type!=V_STRING||a[1]->type!=V_STRING)return val_null();const char *method=a[0]->as.s;const char *body=(n>=3)?val_tostr(a[2]):"";const char *ct=(n>=4&&a[3]->type==V_STRING)?a[3]->as.s:"application/json";return http_request(method,a[1]->as.s,body,ct);}


/* HTTP Server */
static Value *bi_createServer(Value **a,int n){int port=(n>=1)?(int)val_tonum(a[0]):8080;init_sockets();HttpServer *srv=xmalloc(sizeof(HttpServer));memset(srv,0,sizeof(HttpServer));srv->port=port;srv->route_cap=32;srv->routes=xmalloc(sizeof(HttpRoute)*srv->route_cap);srv->sock=socket(AF_INET,SOCK_STREAM,0);if(srv->sock==INVALID_SOCKET){free(srv->routes);free(srv);return val_null();}int opt=1;setsockopt(srv->sock,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));struct sockaddr_in addr;addr.sin_family=AF_INET;addr.sin_addr.s_addr=INADDR_ANY;addr.sin_port=htons(port);if(bind(srv->sock,(struct sockaddr*)&addr,sizeof(addr))<0){close(srv->sock);free(srv->routes);free(srv);return val_null();}listen(srv->sock,10);g_server=srv;Value *obj=val_object();Value *pv=val_int(port);obj_set(obj,"port",pv);val_decref(pv);return obj;}

static Value *bi_serverRoute(Value **a,int n){if(n<3||!g_server)return val_bool(0);if(a[0]->type!=V_STRING||a[1]->type!=V_STRING)return val_bool(0);if(a[2]->type!=V_CLOSURE&&a[2]->type!=V_FUNC)return val_bool(0);if(g_server->route_n>=g_server->route_cap){g_server->route_cap*=2;g_server->routes=xrealloc(g_server->routes,sizeof(HttpRoute)*g_server->route_cap);}g_server->routes[g_server->route_n].method=xstrdup(a[0]->as.s);g_server->routes[g_server->route_n].path=xstrdup(a[1]->as.s);val_incref(a[2]);g_server->routes[g_server->route_n].handler=a[2];g_server->route_n++;return val_bool(1);}

static void http_send(SOCKET client,int status,const char *type,const char *body){const char *st=status==200?"OK":status==404?"Not Found":"Error";char hdr[1024];snprintf(hdr,sizeof(hdr),"HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",status,st,type,(int)strlen(body));send(client,hdr,strlen(hdr),0);send(client,body,strlen(body),0);}

static int server_handle_one(int timeout_ms){if(!g_server||!g_server->running)return 0;struct sockaddr_in ca;socklen_t al=sizeof(ca);struct timeval tv;tv.tv_sec=timeout_ms/1000;tv.tv_usec=(timeout_ms%1000)*1000;fd_set fds;FD_ZERO(&fds);FD_SET(g_server->sock,&fds);if(select(g_server->sock+1,&fds,NULL,NULL,&tv)<=0)return 0;SOCKET client=accept(g_server->sock,(struct sockaddr*)&ca,&al);if(client==INVALID_SOCKET)return 0;char buf[HTTP_BUF];int bytes=recv(client,buf,sizeof(buf)-1,0);if(bytes<=0){close(client);return 0;}buf[bytes]='\0';char method[16],path[1024],ver[16];sscanf(buf,"%15s %1023s %15s",method,path,ver);char *body=strstr(buf,"\r\n\r\n");if(body)body+=4;HttpRoute *route=NULL;for(int i=0;i<g_server->route_n;i++){if(strcmp(g_server->routes[i].method,method)==0&&strcmp(g_server->routes[i].path,path)==0){route=&g_server->routes[i];break;}}if(route&&route->handler){Value *req=val_object();Value *mv=val_string(method);Value *pv=val_string(path);Value *bv=val_string(body?body:"");obj_set(req,"method",mv);obj_set(req,"path",pv);obj_set(req,"body",bv);val_decref(mv);val_decref(pv);val_decref(bv);Value *args[1]={req};Value *result=call_closure(route->handler,args,1);if(result&&result->type==V_OBJECT){Value *sv=obj_get(result,"status");Value *bodv=obj_get(result,"body");Value *tv=obj_get(result,"contentType");int st=sv?(int)val_tonum(sv):200;const char *rb=bodv?val_tostr(bodv):"";const char *rt=(tv&&tv->type==V_STRING)?tv->as.s:"text/html";http_send(client,st,rt,rb);}else{const char *rb=result?val_tostr(result):"";http_send(client,200,"text/html",rb);}val_decref(req);if(result)val_decref(result);}else{http_send(client,404,"text/html","Not Found");}close(client);return 1;}
static Value *bi_serverHandle(Value **a,int n){int timeout=(n>=1)?(int)val_tonum(a[0]):100;return val_bool(server_handle_one(timeout));}
static Value *bi_serverListen(Value **a,int n){(void)a;(void)n;if(!g_server)return val_null();printf("Server listening on port %d (Ctrl+C to stop)\n",g_server->port);g_server->running=1;while(g_server->running){server_handle_one(100);process_timers();}return val_null();}

static Value *bi_serverStop(Value **a,int n){(void)a;(void)n;if(g_server)g_server->running=0;return val_bool(1);}

/* Closure calling */
static Value *call_closure(Value *closure,Value **args,int argc){if(!closure)return val_null();if(closure->type==V_FUNC){FuncDef *fn=func_find(closure->as.s);if(!fn)return val_null();Scope *fn_scope=scope_new(fn->closure?fn->closure:g_global);Scope *prev=g_scope;g_scope=fn_scope;for(int i=0;i<fn->param_n;i++)scope_def(fn_scope,fn->params[i],(i<argc)?args[i]:val_null(),0);g_return=0;g_retval=NULL;eval(fn->body);Value *result=g_retval?g_retval:val_null();g_return=0;g_retval=NULL;g_scope=prev;scope_free(fn_scope);return result;}if(closure->type==V_CLOSURE){Closure *cl=closure->as.cl;Scope *fn_scope=scope_new(cl->env?cl->env:g_global);Scope *prev=g_scope;g_scope=fn_scope;for(int i=0;i<cl->param_n;i++)scope_def(fn_scope,cl->params[i],(i<argc)?args[i]:val_null(),0);g_return=0;g_retval=NULL;eval(cl->body);Value *result=g_retval?g_retval:val_null();g_return=0;g_retval=NULL;g_scope=prev;scope_free(fn_scope);return result;}return val_null();}

/* Array methods */
static Value *array_map(Value *arr,Value **args,int argc){if(argc<1||(args[0]->type!=V_CLOSURE&&args[0]->type!=V_FUNC))return val_array(0);Value *result=val_array(arr->as.arr->len);for(int i=0;i<arr->as.arr->len;i++){Value *item=arr->as.arr->items[i];Value *idx=val_int(i);Value *cbArgs[3]={item,idx,arr};Value *mapped=call_closure(args[0],cbArgs,3);arr_push(result,mapped);val_decref(mapped);val_decref(idx);}return result;}
static Value *array_filter(Value *arr,Value **args,int argc){if(argc<1||(args[0]->type!=V_CLOSURE&&args[0]->type!=V_FUNC))return val_array(0);Value *result=val_array(8);for(int i=0;i<arr->as.arr->len;i++){Value *item=arr->as.arr->items[i];Value *idx=val_int(i);Value *cbArgs[3]={item,idx,arr};Value *keep=call_closure(args[0],cbArgs,3);if(val_truthy(keep)){Value *copy=val_copy(item);arr_push(result,copy);val_decref(copy);}val_decref(keep);val_decref(idx);}return result;}
static Value *array_reduce(Value *arr,Value **args,int argc){if(argc<1||(args[0]->type!=V_CLOSURE&&args[0]->type!=V_FUNC))return val_null();Value *acc=(argc>=2)?val_copy(args[1]):(arr->as.arr->len>0?val_copy(arr->as.arr->items[0]):val_null());int start=(argc>=2)?0:1;for(int i=start;i<arr->as.arr->len;i++){Value *item=arr->as.arr->items[i];Value *idx=val_int(i);Value *cbArgs[4]={acc,item,idx,arr};Value *newAcc=call_closure(args[0],cbArgs,4);val_decref(acc);acc=newAcc;val_decref(idx);}return acc;}
static Value *array_forEach(Value *arr,Value **args,int argc){if(argc<1||(args[0]->type!=V_CLOSURE&&args[0]->type!=V_FUNC))return val_null();for(int i=0;i<arr->as.arr->len;i++){Value *item=arr->as.arr->items[i];Value *idx=val_int(i);Value *cbArgs[3]={item,idx,arr};Value *result=call_closure(args[0],cbArgs,3);val_decref(result);val_decref(idx);}return val_null();}
static Value *array_find(Value *arr,Value **args,int argc){if(argc<1||(args[0]->type!=V_CLOSURE&&args[0]->type!=V_FUNC))return val_null();for(int i=0;i<arr->as.arr->len;i++){Value *item=arr->as.arr->items[i];Value *idx=val_int(i);Value *cbArgs[3]={item,idx,arr};Value *found=call_closure(args[0],cbArgs,3);int match=val_truthy(found);val_decref(found);val_decref(idx);if(match)return val_copy(item);}return val_null();}
static Value *array_findIndex(Value *arr,Value **args,int argc){if(argc<1||(args[0]->type!=V_CLOSURE&&args[0]->type!=V_FUNC))return val_int(-1);for(int i=0;i<arr->as.arr->len;i++){Value *item=arr->as.arr->items[i];Value *idx=val_int(i);Value *cbArgs[3]={item,idx,arr};Value *found=call_closure(args[0],cbArgs,3);int match=val_truthy(found);val_decref(found);val_decref(idx);if(match)return val_int(i);}return val_int(-1);}
static Value *array_every(Value *arr,Value **args,int argc){if(argc<1||(args[0]->type!=V_CLOSURE&&args[0]->type!=V_FUNC))return val_bool(0);for(int i=0;i<arr->as.arr->len;i++){Value *item=arr->as.arr->items[i];Value *idx=val_int(i);Value *cbArgs[3]={item,idx,arr};Value *result=call_closure(args[0],cbArgs,3);int pass=val_truthy(result);val_decref(result);val_decref(idx);if(!pass)return val_bool(0);}return val_bool(1);}
static Value *array_some(Value *arr,Value **args,int argc){if(argc<1||(args[0]->type!=V_CLOSURE&&args[0]->type!=V_FUNC))return val_bool(0);for(int i=0;i<arr->as.arr->len;i++){Value *item=arr->as.arr->items[i];Value *idx=val_int(i);Value *cbArgs[3]={item,idx,arr};Value *result=call_closure(args[0],cbArgs,3);int pass=val_truthy(result);val_decref(result);val_decref(idx);if(pass)return val_bool(1);}return val_bool(0);}
static Value *array_includes(Value *arr,Value **args,int argc){if(argc<1)return val_bool(0);for(int i=0;i<arr->as.arr->len;i++){Value *item=arr->as.arr->items[i];if(item->type==args[0]->type){if(item->type==V_INT&&item->as.i==args[0]->as.i)return val_bool(1);if(item->type==V_STRING&&strcmp(item->as.s,args[0]->as.s)==0)return val_bool(1);if(item->type==V_FLOAT&&item->as.f==args[0]->as.f)return val_bool(1);if(item->type==V_BOOL&&item->as.b==args[0]->as.b)return val_bool(1);}}return val_bool(0);}

/* Method dispatcher */
static Value *call_method(Value *obj,const char *method,Value **args,int argc){
    if(obj->type==V_ARRAY){
        if(strcmp(method,"map")==0)return array_map(obj,args,argc);
        if(strcmp(method,"filter")==0)return array_filter(obj,args,argc);
        if(strcmp(method,"reduce")==0)return array_reduce(obj,args,argc);
        if(strcmp(method,"forEach")==0)return array_forEach(obj,args,argc);
        if(strcmp(method,"find")==0)return array_find(obj,args,argc);
        if(strcmp(method,"findIndex")==0)return array_findIndex(obj,args,argc);
        if(strcmp(method,"every")==0)return array_every(obj,args,argc);
        if(strcmp(method,"some")==0)return array_some(obj,args,argc);
        if(strcmp(method,"includes")==0)return array_includes(obj,args,argc);
        if(strcmp(method,"push")==0&&argc>=1){arr_push(obj,args[0]);return val_int(obj->as.arr->len);}
        if(strcmp(method,"pop")==0)return arr_pop(obj);
        if(strcmp(method,"len")==0)return val_int(obj->as.arr->len);
        if(strcmp(method,"join")==0){Value *a[2]={obj,(argc>=1)?args[0]:val_string(",")};return bi_join(a,2);}
        if(strcmp(method,"reverse")==0){Value *a[1]={obj};return bi_reverse(a,1);}
        if(strcmp(method,"sort")==0){Value *a[1]={obj};return bi_sort(a,1);}
        if(strcmp(method,"slice")==0){Value *a[3]={obj,(argc>=1)?args[0]:val_int(0),(argc>=2)?args[1]:val_int(INT_MAX)};return bi_slice(a,argc+1);}
        if(strcmp(method,"indexOf")==0&&argc>=1){Value *a[2]={obj,args[0]};return bi_indexOf(a,2);}
    }
    if(obj->type==V_STRING){
        if(strcmp(method,"upper")==0){Value *a[1]={obj};return bi_upper(a,1);}
        if(strcmp(method,"lower")==0){Value *a[1]={obj};return bi_lower(a,1);}
        if(strcmp(method,"trim")==0){Value *a[1]={obj};return bi_trim(a,1);}
        if(strcmp(method,"split")==0){Value *a[2]={obj,(argc>=1)?args[0]:val_string(" ")};return bi_split(a,2);}
        if(strcmp(method,"replace")==0&&argc>=2){Value *a[3]={obj,args[0],args[1]};return bi_replace(a,3);}
        if(strcmp(method,"substring")==0||strcmp(method,"slice")==0){Value *a[3]={obj,(argc>=1)?args[0]:val_int(0),(argc>=2)?args[1]:val_int(INT_MAX)};return bi_slice(a,argc+1);}
        if(strcmp(method,"indexOf")==0&&argc>=1){Value *a[2]={obj,args[0]};return bi_indexOf(a,2);}
        if(strcmp(method,"includes")==0&&argc>=1){char *found=strstr(obj->as.s,val_tostr(args[0]));return val_bool(found!=NULL);}
        if(strcmp(method,"startsWith")==0&&argc>=1){return val_bool(strncmp(obj->as.s,val_tostr(args[0]),strlen(val_tostr(args[0])))==0);}
        if(strcmp(method,"endsWith")==0&&argc>=1){int sl=strlen(obj->as.s),suffl=strlen(val_tostr(args[0]));if(suffl>sl)return val_bool(0);return val_bool(strcmp(obj->as.s+sl-suffl,val_tostr(args[0]))==0);}
        if(strcmp(method,"charAt")==0&&argc>=1){int idx=(int)val_tonum(args[0]);if(idx<0||(size_t)idx>=strlen(obj->as.s))return val_string("");char c[2]={obj->as.s[idx],'\0'};return val_string(c);}
        if(strcmp(method,"len")==0)return val_int(strlen(obj->as.s));
    }
    if(obj->type==V_OBJECT||obj->type==V_MODULE){
        if(strcmp(method,"keys")==0){Value *a[1]={obj};return bi_keys(a,1);}
        if(strcmp(method,"values")==0){Value *a[1]={obj};return bi_values(a,1);}
        if(strcmp(method,"entries")==0){Value *arr=val_array(obj->as.obj->count);for(int i=0;i<obj->as.obj->count;i++){Value *pair=val_array(2);Value *k=val_string(obj->as.obj->pairs[i].key);Value *v=val_copy(obj->as.obj->pairs[i].val);arr_push(pair,k);arr_push(pair,v);val_decref(k);val_decref(v);arr_push(arr,pair);val_decref(pair);}return arr;}
        if(strcmp(method,"hasOwnProperty")==0&&argc>=1){return val_bool(obj_has(obj,val_tostr(args[0])));}
        if(strcmp(method,"len")==0)return val_int(obj->as.obj->count);
    }
    return val_null();
}

typedef struct{const char *name;Value *(*func)(Value**,int);}Builtin;
static Builtin builtins[]={{"typeof",bi_typeof},{"keys",bi_keys},{"values",bi_values},{"push",bi_push},{"pop",bi_pop},{"input",bi_input},{"str",bi_str},{"int",bi_int_fn},{"float",bi_float_fn},{"range",bi_range},{"join",bi_join},{"split",bi_split},{"replace",bi_replace},{"indexOf",bi_indexOf},{"slice",bi_slice},{"reverse",bi_reverse},{"sort",bi_sort},{"abs",bi_abs},{"min",bi_min},{"max",bi_max},{"floor",bi_floor},{"ceil",bi_ceil},{"round",bi_round},{"sqrt",bi_sqrt},{"pow",bi_pow_fn},{"random",bi_random},{"randomInt",bi_randomInt},{"hasKey",bi_hasKey},{"delete",bi_delete},{"clone",bi_clone},{"isArray",bi_isArray},{"isObject",bi_isObject},{"isString",bi_isString},{"isNumber",bi_isNumber},{"isFunc",bi_isFunc},{"assert",bi_assert},{"sleep",bi_sleep},{"setTimeout",bi_setTimeout},{"setInterval",bi_setInterval},{"clearTimeout",bi_clearTimeout},{"clearInterval",bi_clearInterval},{"time",bi_time},{"now",bi_now},{"readFile",bi_readFile},{"writeFile",bi_writeFile},{"appendFile",bi_appendFile},{"fileExists",bi_fileExists},{"remove",bi_remove},{"mkdir",bi_mkdir},{"rename",bi_rename},{"jsonParse",bi_jsonParse},{"jsonStringify",bi_jsonStringify},{"httpGet",bi_httpGet},{"httpPost",bi_httpPost},{"httpPut",bi_httpPut},{"httpDelete",bi_httpDelete},{"httpPatch",bi_httpPatch},{"http",bi_http},{"createServer",bi_createServer},{"serverRoute",bi_serverRoute},{"serverListen",bi_serverListen},{"serverHandle",bi_serverHandle},{"serverStop",bi_serverStop},{NULL,NULL}};

static Value *call_func(const char *name,AST **args,int argc){
    if(strcmp(name,"print")==0)return bi_print(args,argc);
    Value **vals=xmalloc(sizeof(Value*)*(argc+1));for(int i=0;i<argc;i++)vals[i]=eval(args[i]);Value *result=NULL;
    for(int i=0;builtins[i].name;i++){if(strcmp(name,builtins[i].name)==0){result=builtins[i].func(vals,argc);goto cleanup;}}
    FuncDef *fn=func_find(name);if(!fn)runtime_error(g_line,"undefined function '%s'",name);
    Scope *fn_scope=scope_new(fn->closure?fn->closure:g_global);Scope *prev=g_scope;g_scope=fn_scope;
    for(int i=0;i<fn->param_n;i++)scope_def(fn_scope,fn->params[i],(i<argc)?vals[i]:val_null(),0);
    g_return=0;g_retval=NULL;eval(fn->body);result=g_retval?g_retval:val_null();g_return=0;g_retval=NULL;g_scope=prev;scope_free(fn_scope);
cleanup:for(int i=0;i<argc;i++)val_decref(vals[i]);free(vals);return result?result:val_null();
}

/* Import */
static char *read_file(const char *path){FILE *f=fopen(path,"rb");if(!f)return NULL;fseek(f,0,SEEK_END);long sz=ftell(f);fseek(f,0,SEEK_SET);char *buf=xmalloc(sz+1);size_t rd=fread(buf,1,sz,f);buf[rd]='\0';fclose(f);return buf;}
static char *get_dir(const char *path){char *dir=xstrdup(path);char *sep=strrchr(dir,PATH_SEP);if(!sep)sep=strrchr(dir,'/');if(sep)*sep='\0';else{dir[0]='.';dir[1]='\0';}return dir;}
static char *resolve_path(const char *imp,const char *cur){if(imp[0]=='/'||(strlen(imp)>1&&imp[1]==':'))return xstrdup(imp);char resolved[4096];if(cur&&strlen(cur)>0)snprintf(resolved,sizeof(resolved),"%s%c%s",cur,PATH_SEP,imp);else snprintf(resolved,sizeof(resolved),"%s",imp);return xstrdup(resolved);}

static Value *do_import(const char *path,const char *alias){
    for(int i=0;i<g_import_n;i++){if(strcmp(g_imports[i].path,path)==0){Value *mod=val_copy(g_imports[i].module);scope_def(g_scope,alias,mod,0);return mod;}}
    char *source=read_file(path);if(!source)runtime_error(g_line,"cannot import '%s'",path);
    char *saved_src=g_src;int saved_pos=g_pos,saved_len=g_len,saved_line=g_line;const char *saved_file=g_file;Token saved_tok=g_tok;Scope *saved_scope=g_scope;int saved_funcs=g_func_n;
    Value *module=val_module();Scope *mod_scope=scope_new(g_global);g_scope=mod_scope;g_src=source;g_len=strlen(source);g_pos=0;g_line=1;g_file=path;advance();AST *prog=parse_program();Value *r=eval(prog);val_decref(r);
    for(int i=0;i<mod_scope->count;i++)obj_set(module,mod_scope->syms[i].name,mod_scope->syms[i].val);
    for(int i=saved_funcs;i<g_func_n;i++){Value *fn=val_func(g_funcs[i].name);obj_set(module,g_funcs[i].name,fn);val_decref(fn);}
    g_src=saved_src;g_pos=saved_pos;g_len=saved_len;g_line=saved_line;g_file=saved_file;g_tok=saved_tok;g_scope=saved_scope;free(source);
    if(g_import_n<MAX_IMPORTS){g_imports[g_import_n].path=xstrdup(path);g_imports[g_import_n].alias=xstrdup(alias);val_incref(module);g_imports[g_import_n++].module=module;}
    scope_def(g_scope,alias,module,0);return module;
}

/* Evaluator */
static Value *eval_template(AST *node){char buf[MAX_STR];int pos=0;for(int i=0;i<node->tpl_n&&pos<MAX_STR-100;i++){TemplatePart *part=&node->tpl[i];if(part->is_expr){char *saved_src=g_src;int saved_pos=g_pos,saved_len=g_len,saved_line=g_line;Token saved_tok=g_tok;g_src=part->text;g_len=strlen(part->text);g_pos=0;advance();AST *expr=parse_expr();Value *val=eval(expr);const char *s=val_tostr(val);int len=strlen(s);if(pos+len<MAX_STR-1){memcpy(buf+pos,s,len);pos+=len;}val_decref(val);g_src=saved_src;g_pos=saved_pos;g_len=saved_len;g_line=saved_line;g_tok=saved_tok;}else{int len=strlen(part->text);if(pos+len<MAX_STR-1){memcpy(buf+pos,part->text,len);pos+=len;}}}buf[pos]='\0';return val_string(buf);}

static Value *eval(AST *node){
    if(!node)return val_null();if(g_return||g_break||g_continue)return val_null();g_line=node->line;
    switch(node->type){
        case AST_NOP:return val_null();
        case AST_LITERAL:return val_copy(node->lit);
        case AST_TEMPLATE:return eval_template(node);
        case AST_ARRAY:{Value *arr=val_array(node->child_n);for(int i=0;i<node->child_n;i++){Value *v=eval(node->children[i]);arr_push(arr,v);val_decref(v);}return arr;}
        case AST_OBJECT:{Value *obj=val_object();for(int i=0;i<node->child_n;i++){Value *v=eval(node->children[i]);obj_set(obj,node->keys[i],v);val_decref(v);}return obj;}
        case AST_VAR:{Symbol *sym=scope_find(g_scope,node->name);if(!sym){FuncDef *fn=func_find(node->name);if(fn)return val_func(node->name);runtime_error(node->line,"undefined variable '%s'",node->name);}if(sym->val->type==V_ARRAY||sym->val->type==V_OBJECT||sym->val->type==V_MODULE||sym->val->type==V_INSTANCE||sym->val->type==V_CLOSURE){val_incref(sym->val);return sym->val;}return val_copy(sym->val);}
        case AST_INDEX:{Value *container=eval(node->left);Value *index=eval(node->right);Value *result=NULL;if(container->type==V_ARRAY){Value *item=arr_get(container,(int)val_tonum(index));result=item?val_copy(item):val_null();}else if(container->type==V_OBJECT||container->type==V_MODULE){Value *item=obj_get(container,val_tostr(index));result=item?val_copy(item):val_null();}else if(container->type==V_STRING){int idx=(int)val_tonum(index);int len=strlen(container->as.s);if(idx<0)idx=len+idx;if(idx>=0&&idx<len){char c[2]={container->as.s[idx],'\0'};result=val_string(c);}else result=val_string("");}val_decref(container);val_decref(index);return result?result:val_null();}
        case AST_MEMBER:{Value *obj=eval(node->left);Value *result=NULL;if(obj->type==V_OBJECT||obj->type==V_MODULE){Value *item=obj_get(obj,node->name);result=item?val_copy(item):val_null();}else if(obj->type==V_INSTANCE){Value *prop=inst_get(obj,node->name);result=prop?val_copy(prop):val_null();}val_decref(obj);return result?result:val_null();}
        case AST_BINOP:{if(strcmp(node->op,"&&")==0){Value *left=eval(node->left);if(!val_truthy(left))return left;val_decref(left);return eval(node->right);}if(strcmp(node->op,"||")==0){Value *left=eval(node->left);if(val_truthy(left))return left;val_decref(left);return eval(node->right);}Value *left=eval(node->left);Value *right=eval(node->right);Value *result=NULL;if(strcmp(node->op,"+")==0&&(left->type==V_STRING||right->type==V_STRING)){char buf[MAX_STR];char *ls=xstrdup(val_tostr(left));char *rs=val_tostr(right);snprintf(buf,sizeof(buf),"%s%s",ls,rs);free(ls);result=val_string(buf);}else if(strcmp(node->op,"+")==0){if(left->type==V_FLOAT||right->type==V_FLOAT)result=val_float(val_tonum(left)+val_tonum(right));else result=val_int(left->as.i+right->as.i);}else if(strcmp(node->op,"-")==0){if(left->type==V_FLOAT||right->type==V_FLOAT)result=val_float(val_tonum(left)-val_tonum(right));else result=val_int(left->as.i-right->as.i);}else if(strcmp(node->op,"*")==0){if(left->type==V_FLOAT||right->type==V_FLOAT)result=val_float(val_tonum(left)*val_tonum(right));else result=val_int(left->as.i*right->as.i);}else if(strcmp(node->op,"/")==0){double rv=val_tonum(right);if(rv==0)runtime_error(node->line,"division by zero");result=val_float(val_tonum(left)/rv);}else if(strcmp(node->op,"%")==0){double rv=val_tonum(right);if(rv==0)runtime_error(node->line,"modulo by zero");if(left->type==V_FLOAT||right->type==V_FLOAT)result=val_float(fmod(val_tonum(left),rv));else result=val_int((long long)val_tonum(left)%(long long)rv);}else if(strcmp(node->op,"**")==0)result=val_float(pow(val_tonum(left),val_tonum(right)));else if(strcmp(node->op,"<")==0)result=val_bool(val_tonum(left)<val_tonum(right));else if(strcmp(node->op,">")==0)result=val_bool(val_tonum(left)>val_tonum(right));else if(strcmp(node->op,"<=")==0)result=val_bool(val_tonum(left)<=val_tonum(right));else if(strcmp(node->op,">=")==0)result=val_bool(val_tonum(left)>=val_tonum(right));else if(strcmp(node->op,"==")==0){if((left->type==V_INT||left->type==V_FLOAT)&&(right->type==V_INT||right->type==V_FLOAT))result=val_bool(val_tonum(left)==val_tonum(right));else if(left->type!=right->type)result=val_bool(0);else{switch(left->type){case V_NULL:result=val_bool(1);break;case V_BOOL:result=val_bool(left->as.b==right->as.b);break;case V_INT:result=val_bool(left->as.i==right->as.i);break;case V_FLOAT:result=val_bool(left->as.f==right->as.f);break;case V_STRING:result=val_bool(strcmp(left->as.s,right->as.s)==0);break;default:result=val_bool(left==right);}}}else if(strcmp(node->op,"!=")==0){if((left->type==V_INT||left->type==V_FLOAT)&&(right->type==V_INT||right->type==V_FLOAT))result=val_bool(val_tonum(left)!=val_tonum(right));else if(left->type!=right->type)result=val_bool(1);else{switch(left->type){case V_NULL:result=val_bool(0);break;case V_BOOL:result=val_bool(left->as.b!=right->as.b);break;case V_INT:result=val_bool(left->as.i!=right->as.i);break;case V_FLOAT:result=val_bool(left->as.f!=right->as.f);break;case V_STRING:result=val_bool(strcmp(left->as.s,right->as.s)!=0);break;default:result=val_bool(left!=right);}}}val_decref(left);val_decref(right);return result?result:val_null();}
        case AST_UNARY:{if(strcmp(node->op,"-")==0){Value *v=eval(node->right);Value *r=(v->type==V_FLOAT)?val_float(-v->as.f):val_int(-(long long)val_tonum(v));val_decref(v);return r;}if(strcmp(node->op,"+")==0)return eval(node->right);if(strcmp(node->op,"!")==0){Value *v=eval(node->right);Value *r=val_bool(!val_truthy(v));val_decref(v);return r;}if(strcmp(node->op,"++")==0||strcmp(node->op,"--")==0){if(node->right->type==AST_VAR){Symbol *sym=scope_find(g_scope,node->right->name);if(!sym)runtime_error(node->line,"undefined '%s'",node->right->name);if(sym->is_const)runtime_error(node->line,"cannot modify constant");long long v=(long long)val_tonum(sym->val);v+=(node->op[0]=='+')?1:-1;Value *nv=val_int(v);val_decref(sym->val);sym->val=nv;val_incref(nv);return nv;}}if(strcmp(node->op,"++p")==0||strcmp(node->op,"--p")==0){if(node->left->type==AST_VAR){Symbol *sym=scope_find(g_scope,node->left->name);if(!sym)runtime_error(node->line,"undefined '%s'",node->left->name);if(sym->is_const)runtime_error(node->line,"cannot modify constant");Value *old=val_copy(sym->val);long long v=(long long)val_tonum(sym->val);v+=(node->op[0]=='+')?1:-1;Value *nv=val_int(v);val_decref(sym->val);sym->val=nv;return old;}}return val_null();}
        case AST_TERNARY:{Value *cond=eval(node->cond);int t=val_truthy(cond);val_decref(cond);return eval(t?node->left:node->right);}
        case AST_ASSIGN:{Value *val=eval(node->left);if(node->is_const){scope_def(g_scope,node->name,val,1);}else{Symbol *local=scope_find_local(g_scope,node->name);if(local){if(local->is_const)runtime_error(node->line,"cannot reassign constant '%s'",node->name);val_decref(local->val);val_incref(val);local->val=val;}else{Symbol *outer=scope_find(g_scope->parent,node->name);if(outer&&!outer->is_const){val_decref(outer->val);val_incref(val);outer->val=val;}else{scope_def(g_scope,node->name,val,0);}}}return val;}
        case AST_INDEX_ASSIGN:{Value *val=eval(node->right);AST *target=node->left;if(target->type==AST_INDEX){Value *container=eval(target->left);Value *index=eval(target->right);if(container->type==V_ARRAY)arr_set(container,(int)val_tonum(index),val);else if(container->type==V_OBJECT||container->type==V_MODULE)obj_set(container,val_tostr(index),val);val_decref(container);val_decref(index);}else if(target->type==AST_MEMBER){Value *obj=eval(target->left);if(obj->type==V_OBJECT||obj->type==V_MODULE)obj_set(obj,target->name,val);else if(obj->type==V_INSTANCE)inst_set(obj,target->name,val);val_decref(obj);}return val;}
        case AST_COMPOUND:{Value *current=NULL;const char *name=NULL;if(node->left->type==AST_VAR){name=node->left->name;Symbol *sym=scope_find(g_scope,name);if(!sym)runtime_error(node->line,"undefined '%s'",name);if(sym->is_const)runtime_error(node->line,"cannot modify constant");current=val_copy(sym->val);}else runtime_error(node->line,"invalid compound assignment target");Value *rhs=eval(node->right);Value *result=NULL;if(strcmp(node->op,"+")==0){if(current->type==V_STRING||rhs->type==V_STRING){char buf[MAX_STR];char *cs=xstrdup(val_tostr(current));char *rs=val_tostr(rhs);snprintf(buf,sizeof(buf),"%s%s",cs,rs);free(cs);result=val_string(buf);}else if(current->type==V_FLOAT||rhs->type==V_FLOAT)result=val_float(val_tonum(current)+val_tonum(rhs));else result=val_int((long long)val_tonum(current)+(long long)val_tonum(rhs));}else if(strcmp(node->op,"-")==0){if(current->type==V_FLOAT||rhs->type==V_FLOAT)result=val_float(val_tonum(current)-val_tonum(rhs));else result=val_int((long long)val_tonum(current)-(long long)val_tonum(rhs));}else if(strcmp(node->op,"*")==0){if(current->type==V_FLOAT||rhs->type==V_FLOAT)result=val_float(val_tonum(current)*val_tonum(rhs));else result=val_int((long long)val_tonum(current)*(long long)val_tonum(rhs));}else if(strcmp(node->op,"/")==0){double rv=val_tonum(rhs);if(rv==0)runtime_error(node->line,"division by zero");result=val_float(val_tonum(current)/rv);}else if(strcmp(node->op,"%")==0){double rv=val_tonum(rhs);if(rv==0)runtime_error(node->line,"modulo by zero");if(current->type==V_FLOAT||rhs->type==V_FLOAT)result=val_float(fmod(val_tonum(current),rv));else result=val_int((long long)val_tonum(current)%(long long)rv);}scope_set(g_scope,name,result);val_decref(current);val_decref(rhs);return result;}
        case AST_CALL:{
            /* Check if it's a variable holding a closure first */
            if(node->left->type==AST_VAR){
                Symbol *sym=scope_find(g_scope,node->left->name);
                if(sym&&(sym->val->type==V_CLOSURE||sym->val->type==V_FUNC)){
                    Value **args=xmalloc(sizeof(Value*)*(node->child_n+1));
                    for(int i=0;i<node->child_n;i++)args[i]=eval(node->children[i]);
                    Value *result=call_closure(sym->val,args,node->child_n);
                    for(int i=0;i<node->child_n;i++)val_decref(args[i]);
                    free(args);
                    return result;
                }
                return call_func(node->left->name,node->children,node->child_n);
            }
            if(node->left->type==AST_MEMBER){Value *obj=eval(node->left->left);const char *method=node->left->name;
                /* Check for method call on built-in types */
                Value **args=xmalloc(sizeof(Value*)*(node->child_n+1));for(int i=0;i<node->child_n;i++)args[i]=eval(node->children[i]);
                Value *result=call_method(obj,method,args,node->child_n);
                if(result->type!=V_NULL||(obj->type!=V_OBJECT&&obj->type!=V_MODULE&&obj->type!=V_INSTANCE)){for(int i=0;i<node->child_n;i++)val_decref(args[i]);free(args);val_decref(obj);return result;}
                val_decref(result);
                /* Check for function property */
                if(obj->type==V_MODULE||obj->type==V_OBJECT){Value *fn=obj_get(obj,method);if(fn&&fn->type==V_FUNC){result=call_func(fn->as.s,node->children,node->child_n);for(int i=0;i<node->child_n;i++)val_decref(args[i]);free(args);val_decref(obj);return result;}if(fn&&fn->type==V_CLOSURE){result=call_closure(fn,args,node->child_n);for(int i=0;i<node->child_n;i++)val_decref(args[i]);free(args);val_decref(obj);return result;}}
                if(obj->type==V_INSTANCE){Method *m=method_find(obj->as.inst->cd,method);if(m){Value *saved_this=g_this;g_this=obj;Scope *meth_scope=scope_new(g_global);Scope *prev=g_scope;g_scope=meth_scope;for(int i=0;i<m->param_n;i++)scope_def(meth_scope,m->params[i],(i<node->child_n)?args[i]:val_null(),0);g_return=0;g_retval=NULL;eval(m->body);result=g_retval?g_retval:val_null();g_return=0;g_retval=NULL;g_scope=prev;scope_free(meth_scope);g_this=saved_this;for(int i=0;i<node->child_n;i++)val_decref(args[i]);free(args);val_decref(obj);return result;}}
                for(int i=0;i<node->child_n;i++)val_decref(args[i]);free(args);val_decref(obj);
            }
            /* Anonymous function/closure call */
            if(node->left->type==AST_ARROW||node->left->type==AST_FUNC){Value *closure=eval(node->left);Value **args=xmalloc(sizeof(Value*)*(node->child_n+1));for(int i=0;i<node->child_n;i++)args[i]=eval(node->children[i]);Value *result=call_closure(closure,args,node->child_n);for(int i=0;i<node->child_n;i++)val_decref(args[i]);free(args);val_decref(closure);return result;}
            /* Variable holding closure */
            Value *fn=eval(node->left);if(fn->type==V_CLOSURE||fn->type==V_FUNC){Value **args=xmalloc(sizeof(Value*)*(node->child_n+1));for(int i=0;i<node->child_n;i++)args[i]=eval(node->children[i]);Value *result=call_closure(fn,args,node->child_n);for(int i=0;i<node->child_n;i++)val_decref(args[i]);free(args);val_decref(fn);return result;}
            val_decref(fn);runtime_error(node->line,"not a function");return val_null();
        }
        case AST_BLOCK:{Scope *blk_scope=scope_new(g_scope);Scope *prev=g_scope;g_scope=blk_scope;Value *result=val_null();for(int i=0;i<node->child_n;i++){val_decref(result);result=eval(node->children[i]);if(g_return||g_break||g_continue)break;}g_scope=prev;scope_free(blk_scope);return result;}
        case AST_IF:{Value *cond=eval(node->cond);int t=val_truthy(cond);val_decref(cond);if(t)return eval(node->body);if(node->right)return eval(node->right);return val_null();}
        case AST_WHILE:{while(1){Value *cond=eval(node->cond);int t=val_truthy(cond);val_decref(cond);if(!t)break;Value *r=eval(node->body);val_decref(r);if(g_return)break;if(g_break){g_break=0;break;}if(g_continue){g_continue=0;continue;}}return val_null();}
        case AST_FOR:{Value *iter=eval(node->iter_expr);int len=0;if(iter->type==V_ARRAY)len=iter->as.arr->len;else if(iter->type==V_OBJECT||iter->type==V_MODULE)len=iter->as.obj->count;else if(iter->type==V_STRING)len=strlen(iter->as.s);Scope *for_scope=scope_new(g_scope);Scope *prev=g_scope;g_scope=for_scope;for(int i=0;i<len;i++){Value *idx=val_int(i);scope_def(for_scope,node->idx_var,idx,0);val_decref(idx);Value *item=NULL;if(iter->type==V_ARRAY)item=val_copy(iter->as.arr->items[i]);else if(iter->type==V_OBJECT||iter->type==V_MODULE)item=val_string(iter->as.obj->pairs[i].key);else if(iter->type==V_STRING){char c[2]={iter->as.s[i],'\0'};item=val_string(c);}scope_def(for_scope,node->iter_var,item,0);val_decref(item);Value *r=eval(node->body);val_decref(r);if(g_return)break;if(g_break){g_break=0;break;}if(g_continue){g_continue=0;continue;}}g_scope=prev;scope_free(for_scope);val_decref(iter);return val_null();}
        case AST_FOR_C:{Scope *for_scope=scope_new(g_scope);Scope *prev=g_scope;g_scope=for_scope;if(node->init){Value *r=eval(node->init);val_decref(r);}while(1){if(node->cond){Value *cond=eval(node->cond);int t=val_truthy(cond);val_decref(cond);if(!t)break;}Value *r=eval(node->body);val_decref(r);if(g_return)break;if(g_break){g_break=0;break;}if(g_continue)g_continue=0;if(node->update){Value *u=eval(node->update);val_decref(u);}}g_scope=prev;scope_free(for_scope);return val_null();}
        case AST_RETURN:g_retval=node->left?eval(node->left):val_null();g_return=1;return val_null();
        case AST_BREAK:g_break=1;return val_null();
        case AST_CONTINUE:g_continue=1;return val_null();
        case AST_CASE:{Value *sw=eval(node->cond);int matched=0;for(int i=0;i<node->child_n;i++){AST *c=node->children[i];if(!matched){if(c->cond&&c->cond->type==AST_ARRAY){for(int m=0;m<c->cond->child_n&&!matched;m++){AST *cond=c->cond->children[m];if(cond->type==AST_BINOP&&strcmp(cond->op,"..")==0){Value *lo=eval(cond->left);Value *hi=eval(cond->right);double swn=val_tonum(sw),lon=val_tonum(lo),hin=val_tonum(hi);if(swn>=lon&&swn<=hin)matched=1;val_decref(lo);val_decref(hi);}else{Value *cv=eval(cond);int eq=0;if(sw->type==cv->type){if(sw->type==V_INT)eq=(sw->as.i==cv->as.i);else if(sw->type==V_FLOAT)eq=(sw->as.f==cv->as.f);else if(sw->type==V_STRING)eq=(strcmp(sw->as.s,cv->as.s)==0);else if(sw->type==V_BOOL)eq=(sw->as.b==cv->as.b);}val_decref(cv);if(eq)matched=1;}}}else if(!c->cond)matched=1;}if(matched){for(int j=0;j<c->child_n;j++){Value *r=eval(c->children[j]);val_decref(r);if(g_break){g_break=0;val_decref(sw);return val_null();}if(g_return)break;}if(g_return)break;}}val_decref(sw);return val_null();}
        case AST_FUNC:func_register(node);if(node->name){Value *fn=val_func(node->name);scope_def(g_scope,node->name,fn,0);return fn;}return val_null();
        case AST_ARROW:{Closure *cl=xmalloc(sizeof(Closure));memset(cl,0,sizeof(Closure));cl->name=NULL;cl->params=xmalloc(sizeof(char*)*node->param_n);cl->param_n=node->param_n;for(int i=0;i<node->param_n;i++)cl->params[i]=xstrdup(node->params[i]);cl->body=node->body;cl->env=scope_clone(g_scope);cl->is_arrow=1;return val_closure_new(cl);}
        case AST_CLASS:{ClassDef cd={0};cd.name=xstrdup(node->name);cd.parent=node->parent?xstrdup(node->parent):NULL;cd.methods=xmalloc(sizeof(Method)*node->method_n);cd.method_n=node->method_n;cd.method_cap=node->method_n;for(int i=0;i<node->method_n;i++){AST *m=node->methods[i];cd.methods[i].name=xstrdup(m->name);cd.methods[i].params=xmalloc(sizeof(char*)*m->param_n);cd.methods[i].param_n=m->param_n;for(int j=0;j<m->param_n;j++)cd.methods[i].params[j]=xstrdup(m->params[j]);cd.methods[i].body=m->body;}class_register(&cd);Value *cls=val_new(V_CLASS);cls->as.s=xstrdup(node->name);scope_def(g_scope,node->name,cls,0);return cls;}
        case AST_NEW:{ClassDef *cd=class_find(node->name);if(!cd)runtime_error(node->line,"undefined class '%s'",node->name);Value *inst=val_instance(cd);Method *init=method_find(cd,"init");if(init){Value *saved_this=g_this;g_this=inst;Scope *meth_scope=scope_new(g_global);Scope *prev=g_scope;g_scope=meth_scope;for(int i=0;i<init->param_n;i++){Value *arg=(i<node->child_n)?eval(node->children[i]):val_null();scope_def(meth_scope,init->params[i],arg,0);val_decref(arg);}g_return=0;g_retval=NULL;eval(init->body);g_return=0;if(g_retval)val_decref(g_retval);g_retval=NULL;g_scope=prev;scope_free(meth_scope);g_this=saved_this;}return inst;}
        case AST_THIS:if(!g_this)runtime_error(node->line,"'this' outside of method");val_incref(g_this);return g_this;
        case AST_SUPER:if(!g_this||g_this->type!=V_INSTANCE)runtime_error(node->line,"'super' outside of method");val_incref(g_this);return g_this;
        case AST_IMPORT:{char *path=resolve_path(node->imp_path,g_dir);Value *mod=do_import(path,node->imp_alias);free(path);return mod;}
        case AST_EXPR_STMT:{Value *r=eval(node->left);val_decref(r);return val_null();}
        default:return val_null();
    }
}

/* Main */
static void init_globals(void){g_global=scope_new(NULL);g_scope=g_global;Value *pi=val_float(3.14159265358979323846);scope_def(g_global,"PI",pi,1);val_decref(pi);Value *e=val_float(2.71828182845904523536);scope_def(g_global,"E",e,1);val_decref(e);Value *ver=val_string("3.0");scope_def(g_global,"VERSION",ver,1);val_decref(ver);}

int main(int argc,char **argv){
    char *source=NULL;
    if(argc>=2&&(strcmp(argv[1],"-v")==0||strcmp(argv[1],"--version")==0)){printf("Jeem v3.0\n");return 0;}
    if(argc>=2&&(strcmp(argv[1],"-h")==0||strcmp(argv[1],"--help")==0)){printf("Jeem v3.0 - A lightweight scripting language\n\nUsage: jeem <file.jm>       Run a script file\n       jeem -e \"code\"       Execute code directly\n       jeem init [name]     Initialize a new project\n       jeem start           Run scripts.start from jeem.json\n       jeem test            Run scripts.test from jeem.json\n       jeem run <script>    Run custom script from jeem.json\n       jeem -v, --version   Show version\n       jeem -h, --help      Show this help\n");return 0;}
    if(argc>=2&&strcmp(argv[1],"init")==0){
        const char *name=(argc>=3)?argv[2]:"my-project";
        FILE *f=fopen("jeem.json","w");
        if(!f){fprintf(stderr,"Error: cannot create jeem.json\n");return 1;}
        fprintf(f,"{\n  \"name\": \"%s\",\n  \"version\": \"1.0.0\",\n  \"main\": \"main.jm\",\n  \"scripts\": {\n    \"start\": \"main.jm\",\n    \"test\": \"test.jm\"\n  }\n}\n",name);
        fclose(f);
        f=fopen("main.jm","w");
        if(f){fprintf(f,"// %s - Jeem Project\n\nprint(\"Hello, World!\")\n",name);fclose(f);}
        f=fopen("test.jm","w");
        if(f){fprintf(f,"// %s - Tests\n\nprint(\"Running tests...\")\nprint(\"All tests passed!\")\n",name);fclose(f);}
        printf("Initialized Jeem project '%s'\n  Created: jeem.json\n  Created: main.jm\n  Created: test.jm\n\nRun with: jeem start\n",name);
        return 0;
    }
    /* Script runner - reads jeem.json and runs scripts */
    if(argc>=2&&(strcmp(argv[1],"start")==0||strcmp(argv[1],"test")==0||(strcmp(argv[1],"run")==0&&argc>=3))){
        const char *script_name=(strcmp(argv[1],"run")==0)?argv[2]:argv[1];
        FILE *jf=fopen("jeem.json","r");
        if(!jf){fprintf(stderr,"Error: jeem.json not found. Run 'jeem init' first.\n");return 1;}
        fseek(jf,0,SEEK_END);long jsz=ftell(jf);fseek(jf,0,SEEK_SET);
        char *jbuf=xmalloc(jsz+1);fread(jbuf,1,jsz,jf);jbuf[jsz]='\0';fclose(jf);
        /* Parse scripts from JSON */
        char *scripts=strstr(jbuf,"\"scripts\"");
        if(!scripts){fprintf(stderr,"Error: no 'scripts' section in jeem.json\n");free(jbuf);return 1;}
        char search[256];snprintf(search,sizeof(search),"\"%s\"",script_name);
        char *found=strstr(scripts,search);
        if(!found){fprintf(stderr,"Error: script '%s' not found in jeem.json\n",script_name);free(jbuf);return 1;}
        found+=strlen(search);
        while(*found&&(*found==':'||*found==' '||*found=='\t'||*found=='\n'))found++;
        if(*found!='"'){fprintf(stderr,"Error: invalid script value for '%s'\n",script_name);free(jbuf);return 1;}
        found++;char script_file[512];int si=0;
        while(*found&&*found!='"'&&si<511)script_file[si++]=*found++;
        script_file[si]='\0';free(jbuf);
        printf("> %s\n\n",script_file);
        /* Now run the script file */
        source=read_file(script_file);
        if(!source){fprintf(stderr,"Error: cannot read '%s'\n",script_file);return 1;}
        g_file=script_file;g_dir=get_dir(script_file);
        goto run_source;
    }
    if(argc<2){printf("Jeem v3.0 - Usage: jeem <file.jm> or jeem -e \"code\"\n");return 1;}
    init_globals();
    if(strcmp(argv[1],"-e")==0){if(argc<3){fprintf(stderr,"Error: -e requires code\n");return 1;}source=xstrdup(argv[2]);g_file="<eval>";g_dir=xstrdup(".");}
    else{source=read_file(argv[1]);if(!source){fprintf(stderr,"Error: cannot read '%s'\n",argv[1]);return 1;}g_file=argv[1];g_dir=get_dir(argv[1]);}
run_source:
    if(!g_global)init_globals();
    Value *args_arr=val_array(argc-1);for(int i=1;i<argc;i++){Value *arg=val_string(argv[i]);arr_push(args_arr,arg);val_decref(arg);}scope_def(g_global,"ARGS",args_arr,1);val_decref(args_arr);
    g_src=source;g_len=strlen(source);g_pos=0;g_line=1;advance();AST *prog=parse_program();
    g_scope=g_global;Value *result=val_null();for(int i=0;i<prog->child_n;i++){val_decref(result);result=eval(prog->children[i]);if(g_return||g_break||g_continue)break;}val_decref(result);
    free(source);free(g_dir);
    /* Auto-run pending timers (like JavaScript event loop) */
    while(has_active_timers()){process_timers();sleep_ms(1);}
    return 0;
}
